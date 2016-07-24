//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDKernelDebuggingManager.cpp
///
//==================================================================================

//------------------------------ csAMDKernelDebuggingManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSAPIWrappers/Include/oaRawFileSeralizer.h>
#include <AMDTAPIClasses/Include/Events/apAfterKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suInterceptionFunctions.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Local:
#include <Include/csPublicStringConstants.h>
#include <src/csAMDKernelDebuggingFunctionPointers.h>
#include <src/csAMDKernelDebuggingManager.h>
#include <src/csContextMonitor.h>
#include <src/csDWARFParser.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLMonitor.h>
#include <src/csStringConstants.h>

#define CS_AMD_DEBUG_CONTEXT_NULL (amdclDebugContext)NULL
#define CS_AMD_DEBUG_PC_NULL (amdclDebugPC)NULL

// Static members initializations:
csAMDKernelDebuggingManager* csAMDKernelDebuggingManager::_pMySingleInstance = NULL;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define CS_AMD_KERNEL_DEBUGGING_MODULE_NAME L"AMDOpenCLDebug"   GDT_PLATFORM_SUFFIX_W   GDT_DEBUG_SUFFIX_W  L".dll"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        #if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            #define CS_AMD_KERNEL_DEBUGGING_MODULE_NAME L"libAMDOpenCLDebugAPI32-d.so"
        #elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
            #define CS_AMD_KERNEL_DEBUGGING_MODULE_NAME L"libAMDOpenCLDebugAPI32.so"
        #else // AMDT_BUILD_CONFIGURATION
            #error unknown configuration!
        #endif
    #elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        #if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            #define CS_AMD_KERNEL_DEBUGGING_MODULE_NAME L"libAMDOpenCLDebugAPI64-d.so"
        #elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
            #define CS_AMD_KERNEL_DEBUGGING_MODULE_NAME L"libAMDOpenCLDebugAPI64.so"
        #else // AMDT_BUILD_CONFIGURATION
            #error unknown configuration!
        #endif
    #else // AMDT_ADDRESS_SPACE_TYPE
        #error unknown address space type!
    #endif
#else // AMDT_BUILD_TARGET
    #error unknown build target!
#endif

#define CS_KERNEL_DEBUGGING_BREAKPOINT_PC_LOOKAHEAD (cl_uint)10
#define CS_KERNEL_DEBUGGING_BREAKPOINT_LINE_NUMBER_LOOKAHEAD 10
#define CS_KERNEL_DEBUGGING_DEFAULT_SIZE_OF_POINTER 4

// See //devtools/main/OpenCLDebugAPI/Src/ConvertRegister.h:
// Bits 0:7     Extended register type 30 = ITEMP
// Bits 8:23    Extended register number
// Bits 24:55   Extended register index
// Bits 56:62   Reserved
// Bit 63       Extended register flag
#define CS_KERNEL_DEBUGGING_INDEXED_TEMP_REGISTER_LOCATOR(temp, index) (0x800000000000001eULL | ((temp & 0xffffULL) << 8) | ((index & 0xffffffffULL) << 24))

// ----------------------------------------------------------------------------------
// Class Name:          csAMDKernelDebuggingManagerCallbackSynchronizer
// General Description: The AMDOpenCLDebugAPI is calling us inside a try-catch block.
//                      this class makes sure that cleanup operations are performed
//                      even if an exception is thrown.
// Author:              Uri Shomroni
// Creation Date:       18/09/2012
// ----------------------------------------------------------------------------------
class csAMDKernelDebuggingManagerCallbackSynchronizer
{
public:
    csAMDKernelDebuggingManagerCallbackSynchronizer(osCriticalSection& kernelDebuggingCS, csAMDKernelDebuggingSession*& prKernelDebuggingSessionPointer)
        : m_kernelDebuggingCS(kernelDebuggingCS), m_prKernelDebuggingSessionPointer(prKernelDebuggingSessionPointer)
    {
        // We synchronize the entire block to avoid having the "kernel debugging callback waiting flag from being accessed
        // by multiple threads as well (case 6600, case 8318):
        su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();

        m_kernelDebuggingCS.enter();
    }

    ~csAMDKernelDebuggingManagerCallbackSynchronizer()
    {
        m_prKernelDebuggingSessionPointer = NULL;

        m_kernelDebuggingCS.leave();

        su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
    }

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    csAMDKernelDebuggingManagerCallbackSynchronizer() = delete;
    csAMDKernelDebuggingManagerCallbackSynchronizer(const csAMDKernelDebuggingManagerCallbackSynchronizer&) = delete;
    csAMDKernelDebuggingManagerCallbackSynchronizer& operator=(const csAMDKernelDebuggingManagerCallbackSynchronizer&) = delete;

private:
    osCriticalSection& m_kernelDebuggingCS;
    csAMDKernelDebuggingSession*& m_prKernelDebuggingSessionPointer;
};

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingSession::csAMDKernelDebuggingSession
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/4/2011
// ---------------------------------------------------------------------------
csAMDKernelDebuggingSession::csAMDKernelDebuggingSession(oaCLKernelHandle kernel, apMonitoredFunctionId kernelDebuggingFunction, suIKernelDebuggingManager::KernelDebuggingSessionReason reason, unsigned int workDimension, const gtSize_t* globalWorkOffset, const gtSize_t* globalWorkSize, const gtSize_t* localWorkSize)
    : _pDWARFParser(NULL), _canDebugSessionBeDeleted(false), _debuggedKernelHandle(OA_CL_NULL_HANDLE), _debuggedKernelContainingProgramHandle(OA_CL_NULL_HANDLE), _debuggingKernelFunction(kernelDebuggingFunction), _kernelDebuggingReason(reason),
      _debuggedKernelTotalGlobalWorkSize(-1), _debuggedKernelBinary(NULL), _debuggedKernelBinarySize(0), _kernelDebugContext(CS_AMD_DEBUG_CONTEXT_NULL), _lastDebuggedKernelLineNumber(-1), _debuggedKernelValidWorkItems(NULL),
      _debuggedKernelProgramCounter(CS_AMD_DEBUG_PC_NULL), _debuggedKernelLastProgramCounter(CS_AMD_DEBUG_PC_NULL), _debuggedKernelLastStoppedProgramCounter(CS_AMD_DEBUG_PC_NULL),
      _debuggedKernelFirstCallback(true), _debuggedKernelFirstBreakpoint(true), _continuedStepBreakReason(AP_STEP_OVER_BREAKPOINT_HIT), _areStepBreakpointsSet(false), _stepBreakpointsSetSuccessfully(false),
      _nextDebuggingCommand(AMDCLDEBUG_COMMAND_CONTINUE), _kernelDebuggingCallbackWaiting(false)
{
    // Create the DWARF parser for this session:
    _pDWARFParser = new csDWARFParser;

    //////////////////////////////////////////////////////////////////////////
    // Set the details that stem from the kernel handle:
    _debuggedKernelHandle = kernel;

    if (_debuggedKernelHandle != OA_CL_NULL_HANDLE)
    {
        // Get the kernel's context Id:
        apContextID containingContextId;
        const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingKernel(_debuggedKernelHandle);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            containingContextId = pContextMonitor->contextID();
        }

        // Set the context id into the variable:
        _debuggedKernelComputeContextId = containingContextId;

        // Get the containing program:
        oaCLProgramHandle containingProgramHandle = cs_stat_openCLMonitorInstance.programContainingKernel(_debuggedKernelHandle);

        if (containingProgramHandle != OA_CL_NULL_HANDLE)
        {
            _debuggedKernelContainingProgramHandle = containingProgramHandle;

            // Get the context containing this program:
            const csContextMonitor* pContextMonitorInner = cs_stat_openCLMonitorInstance.contextContainingProgram(containingProgramHandle);
            GT_IF_WITH_ASSERT(pContextMonitorInner != NULL)
            {
                // Get the program details:
                const apCLProgram* pProgramDetails = pContextMonitorInner->programsAndKernelsMonitor().programMonitor(containingProgramHandle);
                GT_IF_WITH_ASSERT(pProgramDetails != NULL)
                {
                    // Get the source code path of the currently debugged kernel:
                    _debuggedKernelContainingProgramSourceFilePath = pProgramDetails->sourceCodeFilePath().asString();

                    // Set this into the DWARF parser:
                    _pDWARFParser->setFirstSourceFileRealPath(_debuggedKernelContainingProgramSourceFilePath);
                }
            }
        }
    }
    else // _debuggedKernelHandle == OA_CL_NULL_HANDLE
    {
        // When clearing the kernel handle, clear the context Id as well:
        apContextID nullContextId;
        _debuggedKernelComputeContextId = nullContextId;
    }

    //////////////////////////////////////////////////////////////////////////
    // Set the details that stem from the work dimensions:
    bool isXValid = (workDimension > 0) && (NULL != globalWorkSize);
    bool isYValid = (workDimension > 1) && (NULL != globalWorkSize);
    bool isZValid = (workDimension > 2) && (NULL != globalWorkSize);

    _debuggedKernelGlobalWorkSize[0] = isXValid ? (int)globalWorkSize[0] : -1;
    _debuggedKernelGlobalWorkSize[1] = isYValid ? (int)globalWorkSize[1] : -1;
    _debuggedKernelGlobalWorkSize[2] = isZValid ? (int)globalWorkSize[2] : -1;

    if (globalWorkOffset != NULL)
    {
        _debuggedKernelGlobalWorkOffset[0] = isXValid ? (int)globalWorkOffset[0] : -1;
        _debuggedKernelGlobalWorkOffset[1] = isYValid ? (int)globalWorkOffset[1] : -1;
        _debuggedKernelGlobalWorkOffset[2] = isZValid ? (int)globalWorkOffset[2] : -1;
    }
    else // globalWorkOffset == NULL
    {
        _debuggedKernelGlobalWorkOffset[0] = isXValid ? 0 : -1;
        _debuggedKernelGlobalWorkOffset[1] = isYValid ? 0 : -1;
        _debuggedKernelGlobalWorkOffset[2] = isZValid ? 0 : -1;
    }

    // If we don't have the local work size, use the global work size in its stead:
    if (localWorkSize != NULL)
    {
        _debuggedKernelLocalWorkSize[0] = isXValid ? (int)localWorkSize[0] : -1;
        _debuggedKernelLocalWorkSize[1] = isYValid ? (int)localWorkSize[1] : -1;
        _debuggedKernelLocalWorkSize[2] = isZValid ? (int)localWorkSize[2] : -1;
    }
    else // localWorkSize == NULL
    {
        _debuggedKernelLocalWorkSize[0] = _debuggedKernelGlobalWorkSize[0];
        _debuggedKernelLocalWorkSize[1] = _debuggedKernelGlobalWorkSize[1];
        _debuggedKernelLocalWorkSize[2] = _debuggedKernelGlobalWorkSize[2];
    }

    _debuggedKernelTotalGlobalWorkSize = _debuggedKernelGlobalWorkSize[0];

    if (_debuggedKernelGlobalWorkSize[1] > 0)
    {
        _debuggedKernelTotalGlobalWorkSize *= _debuggedKernelGlobalWorkSize[1];
    }

    if (_debuggedKernelGlobalWorkSize[2] > 0)
    {
        _debuggedKernelTotalGlobalWorkSize *= _debuggedKernelGlobalWorkSize[2];
    }

    // Reset the current work item as it may now be invalid:
    _steppingWorkItem[0] = -1;
    _steppingWorkItem[1] = -1;
    _steppingWorkItem[2] = -1;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingSession::~csAMDKernelDebuggingSession
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/4/2011
// ---------------------------------------------------------------------------
csAMDKernelDebuggingSession::~csAMDKernelDebuggingSession()
{
    // Delete the DWARF parser:
    delete _pDWARFParser;
    _pDWARFParser = NULL;

    // Note that we successfully stopped debugging:
    suIKernelDebuggingManager::setDisptachInFlight(false);
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::csAMDKernelDebuggingManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
csAMDKernelDebuggingManager::csAMDKernelDebuggingManager() : _hCLKernelDebuggingModule(NULL), _pCurrentKernelDebuggingSession(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::~csAMDKernelDebuggingManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
csAMDKernelDebuggingManager::~csAMDKernelDebuggingManager()
{
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
csAMDKernelDebuggingManager& csAMDKernelDebuggingManager::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new csAMDKernelDebuggingManager;
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::initialize
// Description: Initializes the AMD Kernel debugging API by loading the kernel
//              module debugging API module and getting the function pointers.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/10/2010
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::initialize()
{
    static bool stat_retVal = false;

    // Only attempt this once:
    static bool stat_isFirstTime = true;

    if (stat_isFirstTime)
    {
        stat_isFirstTime = false;

        // Clear the function pointers struct:
        ::memset(&cs_stat_amdKernelDebuggingFunctionPointers, 0, sizeof(csAMDKernelDebuggingFunctionPointers));

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        // Verify the struct is the correct size:
        GT_ASSERT(sizeof(csAMDKernelDebuggingFunctionPointers) == (sizeof(osProcedureAddress) * CS_NUMBER_OF_OPENCL_DEBUGGING_API_FUNCTIONS));

        // Set the SW DBE log level:
        gtString logLevel;
        osDebugLogSeverity currentSeverity = osDebugLog::instance().loggedSeverity();

        switch (currentSeverity)
        {
            case OS_DEBUG_LOG_ERROR:
                logLevel = '3';
                break;

            case OS_DEBUG_LOG_INFO:
            default:
                logLevel = '5';
                break;

            case OS_DEBUG_LOG_DEBUG:
                logLevel = '6';
                break;

            case OS_DEBUG_LOG_EXTENSIVE:
                logLevel = '8';
                break;
        }

        osEnvironmentVariable swdbeLogLevel(L"AMD_CL_DEBUG_LOG_LEVEL", logLevel);
        osSetCurrentProcessEnvVariable(swdbeLogLevel);
#endif

        // Get the path from the spies path and add the file name:
        gtString openCLDebuggingModulePathAsString = suDebuggerInstallDir().asString();
        openCLDebuggingModulePathAsString.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator).append(CS_AMD_KERNEL_DEBUGGING_MODULE_NAME);
        osFilePath openCLDebuggingModulePath(openCLDebuggingModulePathAsString);

        // Load the module:
        bool rcModule = osLoadModule(openCLDebuggingModulePath , _hCLKernelDebuggingModule);
        GT_IF_WITH_ASSERT(rcModule)
        {
            // The names of the functions. Note that these must match the order as declared in csAMDKernelDebuggingFunctionPointers in the csAMDKernelDebuggingManager.h file.
            static const char* openCLDebuggingAPIFunctionNames[CS_NUMBER_OF_OPENCL_DEBUGGING_API_FUNCTIONS] =
            {
                "amdclInterceptCreateKernel",
                "amdclInterceptCreateKernelsInProgram",
                "amdclInterceptRetainKernel",
                "amdclInterceptReleaseKernel",
                "amdclInterceptSetKernelArg",
                "amdclInterceptCreateContext",
                "amdclInterceptCreateContextFromType",
                "amdclInterceptCreateCommandQueue",
                "amdclInterceptCreateCommandQueueWithProperties",
                "amdclDebugEnqueueNDRangeKernel",
                "amdclDebugEnqueueTask",
                "amdclDebugGetProgramCounter",
                "amdclDebugSetBreakpoint",
                "amdclDebugClearBreakpoint",
                "amdclDebugClearAllBreakpoints",
                "amdclDebugGetNumBreakpoints",
                "amdclDebugGetAllBreakpoints",
                "amdclDebugGetKernelBinarySize",
                "amdclDebugGetKernelBinary",
                "amdclDebugGetExecutionMask",
                "amdclDebugSetTrackingRegisters",
                "amdclDebugGetNumberOfActiveRegisters",
                "amdclDebugGetActiveRegisters",
                "amdclDebugGetRegisterValues",
                "amdclDebugGetPrivateMemoryValues",
                "amdclDebugGetGlobalMemoryValues",
                "amdclDebugUtilGetLastError",
                //              "amdclDebugGetCallStack",
                //              "amdclDebugGetCallStackExecutionMask",
                "amdclDebugGetDispatchTable",
                "amdclDebugSetDispatchTable",
                //              "amdclDebugUtilStringToDebugRegisterLocator",
                //              "amdclDebugUtilDebugRegisterLocatorToString",
            };

            for (int i = 0; i < CS_NUMBER_OF_OPENCL_DEBUGGING_API_FUNCTIONS; i++)
            {
                osProcedureAddress pCurrentFunction = NULL;
                bool rcProc = osGetProcedureAddress(_hCLKernelDebuggingModule, openCLDebuggingAPIFunctionNames[i], pCurrentFunction);

                if (rcProc)
                {
                    ((osProcedureAddress*)(&cs_stat_amdKernelDebuggingFunctionPointers))[i] = pCurrentFunction;
                }
            }
        }

        // We only consider success if we got all the function pointers:
        if ((cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateKernel != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateKernelsInProgram != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptRetainKernel != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptReleaseKernel != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptSetKernelArg != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateContext != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateContextFromType != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateCommandQueue != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateCommandQueueWithProperties != NULL) &&
            // Uri, 17/4/11 - This function is shown in the header file, but is not yet exported by the debug DLL.
            // don't fail on it, since we don't use it yet anyway.
            // (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetVersion != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugEnqueueNDRangeKernel != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugEnqueueTask != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetProgramCounter != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetBreakpoint != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugClearBreakpoint != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugClearAllBreakpoints != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetNumBreakpoints != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetAllBreakpoints != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetKernelBinarySize != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetKernelBinary != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetExecutionMask != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetTrackingRegisters != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetNumberOfActiveRegisters != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetActiveRegisters != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetRegisterValues != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetPrivateMemoryValues != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetGlobalMemoryValues != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugUtilGetLastError != NULL) &&
            /*          (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetCallStack != NULL) &&
                        (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetCallStackExecutionMask != NULL) &&*/
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetDispatchTable != NULL) &&
            (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetDispatchTable != NULL)
            /*          (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugUtilStringToDebugRegisterLocator != NULL) &&
                        (cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugUtilDebugRegisterLocatorToString != NULL) */
           )
        {
            // Mark we succeeded:
            stat_retVal = true;

            // Mark that the debugging API is initialized properly:
            m_isInitialized = true;

            // Build a dispatch struct with the intercepted functions, and pass it to the debug API:
            cl_icd_dispatch_table realFunctionPointersForAMDKernelDebuggingAPI;
            memset((void*)&realFunctionPointersForAMDKernelDebuggingAPI, 0, sizeof(cl_icd_dispatch_table));
            csFillICDDispatchTableWithFunctionPointers(realFunctionPointersForAMDKernelDebuggingAPI, cs_stat_realFunctionPointers);

            cl_int rcDisp = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetDispatchTable(&realFunctionPointersForAMDKernelDebuggingAPI);
            GT_ASSERT(rcDisp == CL_SUCCESS);
        }
    }

    return stat_retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::terminate
// Description: Terminates the AMD kernel debugging API.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/10/2010
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::terminate()
{
    // If this is initialized, terminate it:
    if (m_isInitialized)
    {
        // Unload the debugging API module:
        bool unloadedSuccessfully = osReleaseModule(_hCLKernelDebuggingModule);

        if (unloadedSuccessfully)
        {
            // Mark we are de-initialized:
            _hCLKernelDebuggingModule = NULL;
            m_isInitialized = false;
        }
    }

    return !m_isInitialized;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::programBuildFlagsSupported
// Description: Returns true if the build flags are supported by the current
//              kernel debugger. The SW DBE does not support OpenCL 2.0.
// Author:      Uri Shomroni
// Date:        25/11/2014
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::programBuildFlagsSupported(const char* buildFlags, gtString& failureReason)
{
    // Start with the basic implementation:
    bool retVal = suIKernelDebuggingManager::programBuildFlagsSupported(buildFlags, failureReason);

    if (retVal)
    {
        gtASCIIString flags;

        if (NULL != buildFlags)
        {
            flags = buildFlags;
        }

        static const gtASCIIString opencl20Flag = "-cl-std=CL2.0";

        if (-1 != flags.find(opencl20Flag))
        {
            retVal = false;
            failureReason = L"OpenCL 2.0 kernel debugging is not currently supported.";
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::kernelDebuggerType
// Description: Returns this manager's type
// Author:      Uri Shomroni
// Date:        11/11/2012
// ---------------------------------------------------------------------------
suIKernelDebuggingManager::KernelDebuggingManagerType csAMDKernelDebuggingManager::kernelDebuggerType()
{
    return CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::currentlyDebuggedKernel
// Description: Returns the handle of the currently debugged kernel
// Author:      Uri Shomroni
// Date:        26/4/2011
// ---------------------------------------------------------------------------
oaCLKernelHandle csAMDKernelDebuggingManager::currentlyDebuggedKernel()
{
    oaCLKernelHandle retVal = OA_CL_NULL_HANDLE;

    if (_pCurrentKernelDebuggingSession != NULL)
    {
        retVal = _pCurrentKernelDebuggingSession->_debuggedKernelHandle;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getGlobalWorkGeometry
// Description: Outputs the global work geometry for the currently debugged kernel dispatch
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getGlobalWorkGeometry(int& o_workDimensions, int o_globalWorkSize[3], int o_localWorkSize[3], int o_globalWorkOffset[3])
{
    bool retVal = false;

    // If we are in kernel debugging:
    if (NULL != _pCurrentKernelDebuggingSession)
    {
        // Output the values:
        o_workDimensions = ((0 < _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0]) ? 1 : 0) +
                           ((0 < _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1]) ? 1 : 0) +
                           ((0 < _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2]) ? 1 : 0);
        o_globalWorkSize[0] = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0];
        o_globalWorkSize[1] = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1];
        o_globalWorkSize[2] = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2];
        o_localWorkSize[0] = _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize[0];
        o_localWorkSize[1] = _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize[1];
        o_localWorkSize[2] = _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize[2];
        o_globalWorkOffset[0] = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset[0];
        o_globalWorkOffset[1] = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset[1];
        o_globalWorkOffset[2] = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset[2];

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::setNextKernelDebuggingCommand
// Description: Sets the value of the debugging command we will return via the
//              callback function for the debugger.
// Author:      Uri Shomroni
// Date:        24/10/2010
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::setNextKernelDebuggingCommand(apKernelDebuggingCommand command)
{
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // Translate the value to the API constant:
        amdclDebugCommand amdCommand = AMDCLDEBUG_COMMAND_CONTINUE;

        switch (command)
        {
            case AP_KERNEL_CONTINUE:
            {
                amdCommand = AMDCLDEBUG_COMMAND_CONTINUE;
            }
            break;

            case AP_KERNEL_STEP_OVER:
            {
                amdCommand = AMDCLDEBUG_COMMAND_STEP;
                _pCurrentKernelDebuggingSession->_continuedStepBreakReason = AP_STEP_OVER_BREAKPOINT_HIT;
            }
            break;

            case AP_KERNEL_STEP_IN:
            {
                amdCommand = AMDCLDEBUG_COMMAND_STEP;
                _pCurrentKernelDebuggingSession->_continuedStepBreakReason = AP_STEP_IN_BREAKPOINT_HIT;
            }
            break;

            case AP_KERNEL_STEP_OUT:
            {
                if (_pCurrentKernelDebuggingSession->_pDWARFParser->addressStackDepth(_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter) > 1)
                {
                    // Step out of the inner function/scope we are in:
                    amdCommand = AMDCLDEBUG_COMMAND_STEP;
                }
                else // csDWARFParser::instance().addressStackDepth(_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter) <= 1
                {
                    // Stepping out of the main kernel is simply exiting the kernel:
                    amdCommand = AMDCLDEBUG_COMMAND_CONTINUE;
                }

                _pCurrentKernelDebuggingSession->_continuedStepBreakReason = AP_STEP_OUT_BREAKPOINT_HIT;
            }
            break;

            default:
            {
                // Unexpected value:
                GT_ASSERT(false);
            }
            break;
        }

        // Set the value:
        _pCurrentKernelDebuggingSession->_nextDebuggingCommand = amdCommand;
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::setSteppingWorkItem
// Description: Sets the work item used for stepping. As long as this work item
//              is not valid, we will continue stepping.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/2/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::setSteppingWorkItem(const int coordinate[3])
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // Make sure the values aren't our of range (but allow the coordinate 0 for invalid indices):
        int maxX = std::max(_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0], 1);
        int maxY = std::max(_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1], 1);
        int maxZ = std::max(_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2], 1);

        if ((coordinate[0] < maxX) && (coordinate[1] < maxY) && (coordinate[2] < maxZ))
        {
            // Allow negative values, as they represent stepping regardless of validity:
            _pCurrentKernelDebuggingSession->_steppingWorkItem[0] = (_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0] > 0) ? coordinate[0] : -1;
            _pCurrentKernelDebuggingSession->_steppingWorkItem[1] = (_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1] > 0) ? coordinate[1] : -1;
            _pCurrentKernelDebuggingSession->_steppingWorkItem[2] = (_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2] > 0) ? coordinate[2] : -1;

            // This is successful:
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::isWorkItemValid
// Description: Returns true iff we have an execution mask and the indicated item
//              is valid in it.
// Author:      Uri Shomroni
// Date:        15/2/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::isWorkItemValid(const int coordinate[3])
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // If we have an execution mask:
        if (_pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems != NULL)
        {
            // Calculate the item's index:
            int totalIndex = -1;
            int* globalWorkSize = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize;
            bool xUsed = (globalWorkSize[0] > 0);
            int xCoord = coordinate[0];
            bool yUsed = (globalWorkSize[1] > 0);
            int yCoord = coordinate[1];
            bool zUsed = (globalWorkSize[2] > 0);
            int zCoord = coordinate[2];

            // Make sure the index for each used coordinate is valid:
            if (((!xUsed) || ((-1 < xCoord) && (globalWorkSize[0] > xCoord))) &&
                ((!yUsed) || ((-1 < yCoord) && (globalWorkSize[1] > yCoord))) &&
                ((!zUsed) || ((-1 < zCoord) && (globalWorkSize[2] > zCoord))))
            {
                if (xUsed)
                {
                    // Start with the column:
                    totalIndex = xCoord;

                    if (yUsed)
                    {
                        // Add the line if needed:
                        totalIndex += (yCoord * globalWorkSize[0]);

                        if (zUsed)
                        {
                            // Add the slice if needed:
                            totalIndex += (zCoord * globalWorkSize[0] * globalWorkSize[1]);
                        }
                    }
                }
            }

            // Verify the index is valid:
            if ((totalIndex > -1) && (totalIndex < _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize))
            {
                retVal = _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems[totalIndex];
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getFirstValidWorkItem
// Description: Sets the coordinate parameter to be the first work item that is
//              valid (Lowest Z, Lowest Y, Lowest X). If no work items are valid, returns -1.
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getFirstValidWorkItem(int /*ignored*/ wavefrontIndex, int coordinate[3])
{
    (void)(wavefrontIndex); // unused
    (void)(coordinate); //unused
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        const bool* pExecutionMask = _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems;
        GT_IF_WITH_ASSERT(pExecutionMask != NULL)
        {
            // Clear the value:
            coordinate[0] = -1;
            coordinate[1] = -1;
            coordinate[2] = -1;

            // Get the range:
            bool xValid = true;
            int maxX = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0];

            if (maxX <= 0)
            {
                xValid = false;
                maxX = 1;
            }

            bool yValid = true;
            int maxY = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1];

            if (maxY <= 0)
            {
                yValid = false;
                maxY = 1;
            }

            bool zValid = true;
            int maxZ = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2];

            if (maxZ <= 0)
            {
                zValid = false;
                maxZ = 1;
            }

            // Go through the items:
            int currentI = 0;
            bool goOn = true;
            int totalWorkSize = _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize;

            for (int currentZ = 0; (currentZ < maxZ) && goOn; currentZ++)
            {
                for (int currentY = 0; (currentY < maxY) && goOn; currentY++)
                {
                    for (int currentX = 0; (currentX < maxX) && goOn; currentX++)
                    {
                        // If this item is valid:
                        if (pExecutionMask[currentI])
                        {
                            // We searched in order, we can stop looking:
                            coordinate[0] = xValid ? currentX : -1;
                            coordinate[1] = yValid ? currentY : -1;
                            coordinate[2] = zValid ? currentZ : -1;
                            retVal = true;
                            goOn = false;
                        }

                        // Next index:
                        currentI++;

                        // Sanity check:
                        if (currentI >= totalWorkSize)
                        {
                            // This should never happen!
                            GT_ASSERT(false);
                            goOn = false;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::releaseCallbackToNextCommand
// Description: Lets the callback function know it can release the kernel debugging
//              API to execute the next command.
// Author:      Uri Shomroni
// Date:        24/10/2010
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::releaseCallbackToNextCommand()
{
    if (_pCurrentKernelDebuggingSession != NULL && _pMySingleInstance != NULL)
    {
        // Make sure we are not "releasing" something that should already be released.
        // This most often indicates a timing issue, such as case 6476).
        GT_ASSERT(_pCurrentKernelDebuggingSession->_kernelDebuggingCallbackWaiting);
        _pCurrentKernelDebuggingSession->_kernelDebuggingCallbackWaiting = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::lineNumberFromKernelProgramCounter
// Description: Returns the line number that corresponds to programCounter,
//              using the kernel debug information, or -1 if it is out of range
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
int csAMDKernelDebuggingManager::lineNumberFromKernelProgramCounter(amdclDebugContext debugContext, amdclDebugPC programCounter, gtString& sourceFilePath)
{
    (void)(debugContext); // unused
    int retVal = -1;

    amdclDebugPC currentProgramCounter = programCounter;

    while (currentProgramCounter > 0)
    {
        // Use the DWARF data to get the line number:
        csDWARFCodeLocation codeLoc;
        bool rcLN = _pCurrentKernelDebuggingSession->_pDWARFParser->lineNumberFromAddress((csDwarfAddressType)currentProgramCounter, codeLoc);

        if (rcLN)
        {
            retVal = codeLoc._lineNumber;
            sourceFilePath = codeLoc._sourceFileFullPath;
            break;
        }

        // Try one line backwards, as the program counter might not be the first in the source line:
        currentProgramCounter--;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::kernelProgramCountersFromLineNumber
// Description: Returns the address of the first legal instruction after the
//              start of line #lineNumber in the currently debugged kernel's code
//              for each instance of that source code (e.g. inlined functions)
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::kernelProgramCountersFromLineNumber(amdclDebugContext debugContext, const osFilePath& filePath, int lineNumber, gtVector<amdclDebugPC>& programCounters)
{
    (void)(debugContext); // unused
    programCounters.clear();

    if (lineNumber > 0)
    {
        csDWARFCodeLocation codeLoc(filePath.asString(), lineNumber);

        for (int i = 0; i < CS_KERNEL_DEBUGGING_BREAKPOINT_LINE_NUMBER_LOOKAHEAD; i++)
        {
            // Get the vector of addresses:
            gtVector<csDwarfAddressType> addresses;
            codeLoc._lineNumber = lineNumber + i;
            bool rcAD = _pCurrentKernelDebuggingSession->_pDWARFParser->addressesFromLineNumber(codeLoc, addresses);

            if (rcAD)
            {
                // Copy the vector:
                int numberOfAddrs = (int)addresses.size();

                for (int j = 0; j < numberOfAddrs; j++)
                {
                    // cast each address to the type we expect:
                    programCounters.push_back((amdclDebugPC)addresses[j]);
                }

                // Stop if you found any addresses:
                if (numberOfAddrs > 0)
                {
                    break;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getCurrentKernelCallStack
// Description: Gets the calls stack of the kernel currently being debugged
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getCurrentKernelCallStack(const int /*ignored*/ coordinate[3], osCallStack& kernelStack)
{
    (void)(coordinate); // unused

    bool retVal = false;
    kernelStack.clearStack();

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        const gtString& containingProgramSourceFilePath = _pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramSourceFilePath;

        if (!containingProgramSourceFilePath.isEmpty())
        {
            retVal = true;

            // Create a stack frame we will use to generate the calls stack:
            osFilePath programFilePath(containingProgramSourceFilePath);
            osCallStackFrame kernelStackFrame;
            kernelStackFrame.setSourceCodeFilePath(programFilePath);
            kernelStackFrame.setKernelSourceCode(true);
            csDWARFParser& theDWARFParser = *_pCurrentKernelDebuggingSession->_pDWARFParser;

            // Get the stack frames:
            gtVector<amdclDebugPC> callStackProgramCounters;
            // TO_DO: get a full stack instead of just the top item, check the ret val, etc.:
            amdclDebugPC topFramePC = _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter;
            callStackProgramCounters.push_back(topFramePC);

            // Add the frames to the calls stack:
            int numberOfFrames = (int)callStackProgramCounters.size();

            for (int i = 0; i < numberOfFrames; i++)
            {
                // Add the program counter:
                const amdclDebugPC& currentPC = callStackProgramCounters[i];
                kernelStackFrame.setInstructionCounterAddress((osInstructionPointer)(gtSize_t)currentPC);

                // Add the line number:
                gtString frameFilePathAsString;
                int lineNumber = lineNumberFromKernelProgramCounter(_pCurrentKernelDebuggingSession->_kernelDebugContext, currentPC, frameFilePathAsString);
                kernelStackFrame.setSourceCodeFileLineNumber(lineNumber);

                if (frameFilePathAsString.isEmpty())
                {
                    kernelStackFrame.setSourceCodeFilePath(programFilePath);
                }
                else
                {
                    kernelStackFrame.setSourceCodeFilePath(frameFilePathAsString);
                }

                // Get any inlined functions surrounding this program counter:
                const csDWARFProgram* pCurrentProgram = theDWARFParser.findAddressScope(currentPC);

                while (pCurrentProgram != NULL)
                {
                    // If this program has a name, use it as the frame's function name:
                    if (kernelStackFrame.functionName().isEmpty() && (!pCurrentProgram->_programName.isEmpty()))
                    {
                        kernelStackFrame.setFunctionName(pCurrentProgram->_programName);
                    }

                    // If this program is an inlined function, add another frame for it:
                    if (pCurrentProgram->_programScopeType == csDWARFProgram::CS_INLINE_FUNCTION_SCOPE)
                    {
                        // Set the inlined function name as the name:
                        if (!pCurrentProgram->_inlinedFunctionName.isEmpty())
                        {
                            kernelStackFrame.setFunctionName(pCurrentProgram->_inlinedFunctionName);
                        }

                        // Add the virtual frame:
                        kernelStack.addStackFrame(kernelStackFrame);

                        // Set the next frame's line number to be the line where the function was inlined:
                        if (pCurrentProgram->_inlinedFunctionCodeLocation._lineNumber > -1)
                        {
                            kernelStackFrame.setSourceCodeFileLineNumber(pCurrentProgram->_inlinedFunctionCodeLocation._lineNumber);
                        }

                        const gtString& currentProgramSourceFile = pCurrentProgram->_inlinedFunctionCodeLocation._sourceFileFullPath;

                        if (!currentProgramSourceFile.isEmpty())
                        {
                            kernelStackFrame.setSourceCodeFilePath(currentProgramSourceFile);
                        }

                        // Clear the next frame's function name:
                        kernelStackFrame.setFunctionName(L"");
                    }

                    // Go up one level:
                    pCurrentProgram = pCurrentProgram->_pParentProgram;
                }

                // Add the frame to the calls stack:
                kernelStack.addStackFrame(kernelStackFrame);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getCurrentKernelDebugLineNumber
// Description: Returns the line number for the current program counter:
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
int csAMDKernelDebuggingManager::getCurrentKernelDebugLineNumber(const int /*ignored*/ coordinate[3])
{
    (void)(coordinate); // unused
    int retVal = -2;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        gtString ignored;
        retVal = lineNumberFromKernelProgramCounter(_pCurrentKernelDebuggingSession->_kernelDebugContext, _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, ignored);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::doesVariableExistInCurrentScope
// Description: Returns true iff the named variable exists in the current scope
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::doesVariableExistInCurrentScope(const gtString& variableName, const int /*ignored*/ coordinate[3])
{
    (void)(coordinate); // unused
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // If the variable is going to be dereferenced, get the base name:
        gtString variableBaseName = variableName;
        gtString variableNameSuffix;
        bool shouldDereference = false;
        int variableIndex = 0;
        extractDerefernceDataFromExpresssion(variableName, variableBaseName, variableNameSuffix, shouldDereference, variableIndex);
        variableBaseName.append(variableNameSuffix);

        // See if the variable is supported in this scope or any of its parents:
        csDWARFParser& theDWARFParser = *_pCurrentKernelDebuggingSession->_pDWARFParser;
        bool rcDW = (theDWARFParser.findClosestScopeContainingVariable((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, variableBaseName, NULL) != NULL);

        if (rcDW)
        {
            // The variable exists here:
            retVal = true;
        }
        else
        {
            // If this member has an alias:
            gtString variableNameWithAlias;

            if (matchOpenCLMemberAliases(variableBaseName, variableNameWithAlias))
            {
                // Try with the alias:
                rcDW = (theDWARFParser.findClosestScopeContainingVariable((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, variableNameWithAlias, NULL) != NULL);

                if (rcDW)
                {
                    // The variable exists here:
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getVariableValuesInCurrentLocation
// Description: Gets the variable values based on the current scope / program counter
//              location.
//
//              IMPORTANT NOTE:
//              All successful calls to this function MUST be matched with a call
//              releaseVariableValues, passing the variableValues and wasAllocated
//              returned values
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getVariableValuesInCurrentLocation(const gtString& variableName, int variableIndex, const void*& variableValues, unsigned int& valueStride, csDWARFVariable& variableData, bool& wasAllocated)
{
    bool retVal = false;

    wasAllocated = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // See if the variable is supported in this scope or any of its parents:
        const csDWARFVariable* pVarDetails = getVariableDetailsInCurrentLocation(variableName);

        // If we found the variable or its aliases:
        if (pVarDetails != NULL)
        {
            gtSize_t additionalOffsetFromArray = 0;

            if (pVarDetails->_valueIsArray && (variableIndex > 0))
            {
                additionalOffsetFromArray = pVarDetails->_valueSize * (gtSize_t)variableIndex;
            }

            // Make sure we got a valid handle:
            gtUInt64 variableLocation = pVarDetails->_variableLocation._variableLocation;

            if (variableLocation != GT_UINT64_MAX)
            {
                csDWARFVariable::ValueLocationType variableLocationType = pVarDetails->_variableLocation._variableLocationType;

                switch (variableLocationType)
                {
                    case csDWARFVariable::CS_REGISTER:
                    {
                        // Get the values:
                        const void* outputBuffer = NULL;
                        gtSize_t outputStride = 0;
                        gtSize_t numberOfElements = 0;
                        amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetRegisterValues(_pCurrentKernelDebuggingSession->_kernelDebugContext, variableLocation, &outputBuffer, &outputStride, &numberOfElements);

                        if (rc == AMDCLDEBUG_SUCCESS)
                        {
                            // Output the data:
                            variableData = *pVarDetails;
                            gtInt32 variableLocationAccumulatedOffset = variableData._variableLocation._variableLocationAccumulatedOffset;
                            variableValues = (const void*)((gtSize_t)outputBuffer + (gtSize_t)variableLocationAccumulatedOffset + additionalOffsetFromArray);
                            valueStride = (unsigned int)outputStride;
                            GT_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize == (int)numberOfElements);
                            GT_ASSERT(variableLocationAccumulatedOffset < (gtInt32)valueStride);
                            retVal = true;
                            wasAllocated = false;
                        }
                    }
                    break;

                    case csDWARFVariable::CS_INDIRECT_REGISTER:
                    case csDWARFVariable::CS_STACK_OFFSET:
                    {
                        // Will get the location:
                        amdclDebugMemoryAddress valueAddress = (amdclDebugMemoryAddress)pVarDetails->_variableLocation._variableLocationAccumulatedOffset;
                        bool operationFailed = false;

                        // For indirect registers and stack values, we need to add the value of a register to the pointer (the requested register or the frame pointer)
                        if ((variableLocationType == csDWARFVariable::CS_INDIRECT_REGISTER) || (variableLocationType == csDWARFVariable::CS_STACK_OFFSET))
                        {
                            // Calculate the register index:
                            gtUInt64 registerToAdd = (variableLocationType == csDWARFVariable::CS_INDIRECT_REGISTER) ? variableLocation :
                                                     _pCurrentKernelDebuggingSession->_pDWARFParser->getFramePointerRegister((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, variableName);

                            if (registerToAdd != GT_UINT64_MAX)
                            {
                                // Get the register values:
                                const void* outputBuffer = NULL;
                                gtSize_t outputStride = 0;
                                gtSize_t numberOfElements = 0;
                                amdclDebugError rcReg = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetRegisterValues(_pCurrentKernelDebuggingSession->_kernelDebugContext, (amdclDebugRegisterLocator)registerToAdd, &outputBuffer, &outputStride, &numberOfElements);

                                // Uri, 29/7/14 - Register #1019 is used as an encoding for the indexed temp x0[0] (the private stack pointer).
                                // Newer compiler versions (14.30 and onwards) use the real value in IL, instead of the encoded value which is left
                                // in the DWARF. If we tried to get r1019 and failed, try getting x0[0] instead:
                                if ((1019 == registerToAdd) && (AMDCLDEBUG_SUCCESS != rcReg))
                                {
                                    GT_ASSERT(NULL == outputBuffer);
                                    rcReg = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetRegisterValues(_pCurrentKernelDebuggingSession->_kernelDebugContext, (amdclDebugRegisterLocator)CS_KERNEL_DEBUGGING_INDEXED_TEMP_REGISTER_LOCATOR(0, 0), &outputBuffer, &outputStride, &numberOfElements);
                                }

                                operationFailed = (AMDCLDEBUG_SUCCESS != rcReg) || (NULL == outputBuffer);
                                GT_IF_WITH_ASSERT(!operationFailed)
                                {
                                    // Add the value to the address:
                                    // TO_DO: how should we handle this when the values are different?
                                    valueAddress += (amdclDebugMemoryAddress)(*(gtInt32*)outputBuffer);
                                }
                            }
                        }

                        // If the operation failed, don't attempt to access private memory at an illegal address:
                        GT_IF_WITH_ASSERT(!operationFailed)
                        {
                            // For stack offsets and memory pointers, the variable location is an offset:
                            if (variableLocationType == csDWARFVariable::CS_STACK_OFFSET)
                            {
                                valueAddress += (amdclDebugMemoryAddress)variableLocation;
                            }

                            // Add the array offset, if needed:
                            valueAddress += (amdclDebugMemoryAddress)additionalOffsetFromArray;
                            gtSize_t valueSize = (gtSize_t)pVarDetails->_valueSize;
                            gtSize_t sizeToGet = valueSize;

                            if (pVarDetails->_valueIsPointer)
                            {
                                sizeToGet = CS_KERNEL_DEBUGGING_DEFAULT_SIZE_OF_POINTER;
                                int pointerSize = _pCurrentKernelDebuggingSession->_pDWARFParser->dwarfPointerSize();

                                if (0 < pointerSize)
                                {
                                    sizeToGet = (gtSize_t)pointerSize;
                                }
                            }

                            gtSize_t expectedDataSize = sizeToGet * _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize;

                            // Allocate the data buffer.
                            gtUByte* pDataBuffer = new gtUByte[expectedDataSize];
                            ::memset((void*)pDataBuffer, 0, expectedDataSize);
                            gtSize_t outputSize = 0;
                            amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetPrivateMemoryValues(_pCurrentKernelDebuggingSession->_kernelDebugContext, valueAddress, sizeToGet, (void*)pDataBuffer, &outputSize);

                            if (rc == AMDCLDEBUG_SUCCESS)
                            {
                                // Output the data:
                                variableData = *pVarDetails;
                                variableValues = (const void*)pDataBuffer;
                                valueStride = (unsigned int)sizeToGet;
                                GT_ASSERT(outputSize == expectedDataSize);
                                retVal = true;
                                wasAllocated = true;
                            }
                            else // rc != AMDCLDEBUG_SUCCESS
                            {
                                // Release the buffer:
                                delete[] pDataBuffer;
                            }
                        }
                    }
                    break;

                    default:
                    {
                        // Unknown memory space:
                        GT_ASSERT(false);
                    }
                    break;
                }
            }
            else // pVarDetails->_variableLocation == GT_UINT64_MAX
            {
                // A variable must have either a location or a constant value:
                GT_IF_WITH_ASSERT(pVarDetails->_variableConstantValueExists)
                {
                    // Return the data:
                    variableData = *pVarDetails;
                    variableValues = NULL;
                    valueStride = 0;
                    retVal = true;
                    wasAllocated = false;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::releaseVariableValues
// Description: Release the data buffer allocated by getVariableValuesInCurrentLocation
// Author:      Uri Shomroni
// Date:        27/7/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::releaseVariableValues(const void* variableValues, bool wasAllocated)
{
    // If this is a buffer we had to allocate:
    if (wasAllocated)
    {
        // We create buffers as gtUByte buffers:
        gtUByte* pDataBuffer = (gtUByte*)variableValues;

        // Release the buffer:
        delete[] pDataBuffer;
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getVariableValueString
// Description: Gets and formats the variable value for a given work item
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        20/1/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getVariableValueString(const gtString& variableName, const int coordinate[3], gtString& valueString, gtString& valueStringHex, gtString& variableType)
{
    bool retVal = false;
    valueString.makeEmpty();
    valueStringHex.makeEmpty();

    osDebugLogSeverity loggedSeverity = osDebugLog::instance().loggedSeverity();

    if (OS_DEBUG_LOG_DEBUG <= loggedSeverity)
    {
        gtString logMsg = variableName;
        logMsg.prepend(L"Starting to evaluate variable ");
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // Get the variable values for all work items:
        const void* variableValuesBuffer = NULL;
        unsigned int valueStride = 0;
        csDWARFVariable variableData;
        bool wasVariableValuesBufferAllocated = false;
        bool wiValid = isWorkItemValid(coordinate);

        // If the variable is going to be dereferenced, get the base name:
        gtString variableBaseName = variableName;
        gtString dereferencedVariableMember;
        int variableIndex = 0;
        bool shouldDereference = false;
        extractDerefernceDataFromExpresssion(variableName, variableBaseName, dereferencedVariableMember, shouldDereference, variableIndex);

        bool rcVal = getVariableValuesInCurrentLocation(variableBaseName, shouldDereference ? variableIndex : -1, variableValuesBuffer, valueStride, variableData, wasVariableValuesBufferAllocated);

        if (rcVal)
        {
            // Calculate the item overall index from its coordinates:
            const int* globalWorkOffset = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset;
            int totalIndex = std::max(coordinate[0] - globalWorkOffset[0], 0);
            int workSizeX = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0];
            int workSizeY = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1];

            if (workSizeX > 0)
            {
                totalIndex += workSizeX * std::max(coordinate[1] - globalWorkOffset[1], 0);

                if (workSizeY > 0)
                {
                    totalIndex += workSizeX * workSizeY * std::max(coordinate[2] - globalWorkOffset[2], 0);
                }
            }

            const void* valueData = NULL;

            int dwarfPointerSize = _pCurrentKernelDebuggingSession->_pDWARFParser->dwarfPointerSize();

            if (variableData._variableLocation._variableLocation != GT_UINT64_MAX)
            {
                GT_IF_WITH_ASSERT((variableValuesBuffer != NULL) && (valueStride > 0))
                {
                    // Get the data start pointer for this item from the index:
                    valueData = (const void*)((gtSize_t)variableValuesBuffer + ((gtSize_t)totalIndex * (gtSize_t)valueStride));

                    // Parse the value:
                    if (wiValid)
                    {
                        variableData.parseValueFromPointer(valueData, valueString, false, dwarfPointerSize);
                        variableData.parseValueFromPointer(valueData, valueStringHex, true, dwarfPointerSize);
                    }
                    else
                    {
                        valueString = CS_STR_VariableWorkItemNotValid;
                        valueStringHex = CS_STR_VariableWorkItemNotValid;
                    }

                    // Copy the variable type:
                    variableType = variableData._variableType;

                    // If the variable has an address space qualifier, add it to the type:
                    if ((variableData._valueIsPointer || variableData._valueIsArray || (-1 != variableType.find('&'))) && (!variableType.isEmpty()))
                    {
                        switch (variableData._valuePointerAddressSpace)
                        {
                            case csDWARFVariable::CS_GLOBAL_POINTER:
                                variableType.prepend(L"__global ");
                                break;

                            case csDWARFVariable::CS_REGION_POINTER:
                                variableType.prepend(L"__region ");
                                break;

                            case csDWARFVariable::CS_LOCAL_POINTER:
                                variableType.prepend(L"__local ");
                                break;

                            case csDWARFVariable::CS_PRIVATE_POINTER:
                                variableType.prepend(L"__private ");
                                break;

                            case csDWARFVariable::CS_CONSTANT_POINTER:
                                variableType.prepend(L"__constant ");
                                break;

                            case csDWARFVariable::CS_NOT_A_POINTER:
                            case csDWARFVariable::CS_UNKNOWN_POINTER:
                            default:
                                // Add nothing
                                break;
                        }
                    }

                    retVal = true;
                }
            }
            else // variableData._variableLocation == GT_UINT64_MAX
            {
                // Variables with no location must have a constant value:
                if (variableData._variableConstantValueExists)
                {
                    // Parse the value:
                    if (wiValid)
                    {
                        variableData.parseValueFromPointer(NULL, valueString, false, dwarfPointerSize);
                        variableData.parseValueFromPointer(NULL, valueStringHex, true, dwarfPointerSize);
                    }
                    else
                    {
                        valueString = CS_STR_VariableWorkItemNotValid;
                        valueStringHex = CS_STR_VariableWorkItemNotValid;
                    }

                    // Copy the variable type:
                    variableType = variableData._variableType;

                    retVal = true;
                }
            }

            // Get the details for the specific member we wish to get:
            csDWARFVariable variableFinalData = variableData;

            if (!dereferencedVariableMember.isEmpty())
            {
                GT_ASSERT(shouldDereference);

                // Get the member's details:
                gtString memberFullname = variableBaseName;
                memberFullname.append(dereferencedVariableMember);
                const csDWARFVariable* pMemberDetails = getVariableDetailsInCurrentLocation(memberFullname);

                if (pMemberDetails != NULL)
                {
                    variableFinalData = *pMemberDetails;
                }
                else // pMemberDetails == NULL
                {
                    // we could not find the member, fail the function:
                    retVal = false;
                }
            }

            // If this is an array value, we need to show it somehow:
            if (retVal && (!shouldDereference) && variableData._valueIsArray)
            {
                if (!valueString.isEmpty())
                {
                    valueString.prepend('{').append(L", ... }");
                }

                if (!valueStringHex.isEmpty())
                {
                    valueStringHex.prepend('{').append(L", ... }");
                }
            }

            // If we need to, dereference the variable:
            if (retVal && shouldDereference)
            {
                retVal = false;

                // We currently only allow dereferencing pointers and arrays:
                if ((variableType.count('*') == 1))
                {
                    // We cannot get the base pointer by using "*(amdclDebugMemoryAddress*)valueData", since the hardware pointer size might be smaller than 64 bits.
                    amdclDebugMemoryAddress valuePointer = 0;
                    gtSize_t pointerSize = std::min(sizeof(amdclDebugMemoryAddress), (gtSize_t)CS_KERNEL_DEBUGGING_DEFAULT_SIZE_OF_POINTER);
                    int dwarfPtrSize = _pCurrentKernelDebuggingSession->_pDWARFParser->dwarfPointerSize();

                    if (0 < dwarfPtrSize)
                    {
                        pointerSize = std::min(sizeof(amdclDebugMemoryAddress), (gtSize_t)dwarfPtrSize);
                    }

                    ::memcpy(&valuePointer, valueData, pointerSize);

                    // Add an offset for members after dereferencing (in expressions like varName[123]._memberName):
                    unsigned int valueFinalOffset = (unsigned int)variableFinalData._variableLocation._variableLocationAccumulatedOffset;
                    unsigned int valueOffset = (unsigned int)variableData._variableLocation._variableLocationAccumulatedOffset;
                    unsigned int memberAdditionalOffset = (valueFinalOffset > valueOffset) ? (valueFinalOffset - valueOffset) : 0;
                    valuePointer += memberAdditionalOffset;

                    // Calculate the full offset:
                    unsigned int valueSize = (unsigned int)variableData._valueSize;
                    valuePointer += (variableIndex * valueSize);

                    // Get the value:
                    unsigned int valueFinalSize = (unsigned int)variableFinalData._valueSize;
                    gtUByte* dataBuffer = new gtUByte[valueSize];
                    ::memset(dataBuffer, 0, valueSize);
                    gtSize_t outputSize = 0;
                    amdclDebugError rcMem = AMDCLDEBUG_UNINITIALIZED;

                    if (csDWARFVariable::CS_GLOBAL_POINTER == variableData._valuePointerAddressSpace)
                    {
                        amdclDebugMemoryResource memoryResource = (amdclDebugMemoryResource)variableData._variableLocation._variableLocationResource;
                        rcMem = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetGlobalMemoryValues(_pCurrentKernelDebuggingSession->_kernelDebugContext, memoryResource, valuePointer, (gtSize_t)valueFinalSize, (void*)dataBuffer, &outputSize);
                    }
                    else if (csDWARFVariable::CS_PRIVATE_POINTER == variableData._valuePointerAddressSpace)
                    {
                        gtUByte* allWorkItemsDataBuffer = new gtUByte[valueSize * _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize];
                        rcMem = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetPrivateMemoryValues(_pCurrentKernelDebuggingSession->_kernelDebugContext, valuePointer, (gtSize_t)valueFinalSize, (void*)allWorkItemsDataBuffer, &outputSize);

                        if (AMDCLDEBUG_SUCCESS == rcMem)
                        {
                            gtUByte* startOfSpecificWorkItemData = &(allWorkItemsDataBuffer[valueSize * totalIndex]);
                            ::memcpy(dataBuffer, startOfSpecificWorkItemData, valueSize);
                            GT_ASSERT(valueSize * _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize == outputSize);

                            // The expect output size is a single work item's worth:
                            outputSize = (gtSize_t)valueFinalSize;
                        }
                    }
                    else
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Attempted to derefernce variable of unsupported memory address space", OS_DEBUG_LOG_DEBUG);
                    }

                    if ((rcMem == AMDCLDEBUG_SUCCESS) && (valueFinalSize == (unsigned int)outputSize))
                    {
                        retVal = true;

                        // Parse the value:
                        if (wiValid)
                        {
                            valueString.makeEmpty();
                            variableFinalData.parseValueFromPointer(dataBuffer, valueString, false, dwarfPointerSize, true);
                            valueStringHex.makeEmpty();
                            variableFinalData.parseValueFromPointer(dataBuffer, valueStringHex, true, dwarfPointerSize, true);
                        }
                        else
                        {
                            valueString = CS_STR_VariableWorkItemNotValid;
                            valueStringHex = CS_STR_VariableWorkItemNotValid;
                        }

                        // "dereference" the variable type:
                        variableType = variableFinalData._variableType;
                        variableType.removeTrailing('*');
                    }

                    // Clear the buffer:
                    delete[] dataBuffer;
                }
                else if (variableData._valueIsArray)
                {
                    // Array types are handled in the getVariableValuesInCurrentLocation function
                    retVal = true;

                    if (!dereferencedVariableMember.isEmpty())
                    {
                        gtSize_t memberOffset = (variableData._variableLocation._variableLocationAccumulatedOffset < variableFinalData._variableLocation._variableLocationAccumulatedOffset) ?
                                                (variableFinalData._variableLocation._variableLocationAccumulatedOffset - variableData._variableLocation._variableLocationAccumulatedOffset) : 0;
                        const void* finalDataBuffer = (const void*)((gtSize_t)variableValuesBuffer + memberOffset);

                        if (wiValid)
                        {
                            valueString.makeEmpty();
                            variableFinalData.parseValueFromPointer(finalDataBuffer, valueString, false, dwarfPointerSize, true);
                            valueStringHex.makeEmpty();
                            variableFinalData.parseValueFromPointer(finalDataBuffer, valueStringHex, true, dwarfPointerSize, true);
                        }
                        else
                        {
                            valueString = CS_STR_VariableWorkItemNotValid;
                            valueStringHex = CS_STR_VariableWorkItemNotValid;
                        }
                    }

                    // "dereference" the variable type:
                    variableType = variableFinalData._variableType;
                    variableType.replace(L"[]", L"");
                }
            }

            // Release the variable data:
            releaseVariableValues(variableValuesBuffer, wasVariableValuesBufferAllocated);
        }
    }

    if (OS_DEBUG_LOG_DEBUG <= loggedSeverity)
    {
        gtString logMsg = variableName;
        logMsg.prepend(L"Ended evaluation of variable ");
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getVariableMembers
// Description: Gets the direct children (members) of the selected variable
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/5/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getVariableMembers(const gtString& variableName, const int /*ignored*/ coordinate[3], gtVector<gtString>& memberNames)
{
    (void)(coordinate); // unused
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        gtString variableBaseName = variableName;
        gtString variableNameSuffix;
        bool shouldDereference = false;
        int variableIndex = 0;
        extractDerefernceDataFromExpresssion(variableName, variableBaseName, variableNameSuffix, shouldDereference, variableIndex);
        variableBaseName.append(variableNameSuffix);

        // See if the variable is supported in this scope or any of its parents:
        const csDWARFVariable* pVarDetails = getVariableDetailsInCurrentLocation(variableBaseName);

        // If we found the variable or its aliases:
        if (pVarDetails != NULL)
        {
            retVal = true;

            // Don't give members for unsupported memory spaces:
            if ((csDWARFVariable::CS_LOCAL_POINTER != pVarDetails->_valuePointerAddressSpace) && (csDWARFVariable::CS_CONSTANT_POINTER != pVarDetails->_valuePointerAddressSpace))
            {
                // If it's not a pointer or it's a dereferenced pointer:
                if ((!pVarDetails->_valueIsPointer) || shouldDereference)
                {
                    // Get its members:
                    const gtVector<csDWARFVariable>& variableMembers = pVarDetails->_variableMembers;
                    int numberOfMembers = (int)variableMembers.size();

                    for (int i = 0; i < numberOfMembers; i++)
                    {
                        // Get the member name:
                        const gtString& currentMemberName = variableMembers[i]._variableName;

                        if (!currentMemberName.isEmpty())
                        {
                            // Add it to the vector:
                            memberNames.push_back(currentMemberName);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getAvailableVariables
// Description: Gets a list of variables available in the current PC
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getAvailableVariables(const int /*ignored*/ coordinate[3], gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth)
{
    (void)(coordinate); // unused
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        retVal = _pCurrentKernelDebuggingSession->_pDWARFParser->listVariablesFromAddress((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, variableNames, getLeaves, stackFrameDepth);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getExecutionMask
// Description: Gets the execution mask if it is available
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        15/2/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getExecutionMask(bool*& executionMask, int& maskSize)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // If we have a mask:
        if (_pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems != NULL)
        {
            // Return it:
            executionMask = _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems;
            maskSize = _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getWavefrontIndex
// Description: Gets the number of wavefronts (1) if kernel debugging is active
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/4/2013
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getAmountOfActiveWavefronts(int& amountOfWavefronts)
{
    amountOfWavefronts = 0;
    bool retVal = false;

    if (NULL != _pCurrentKernelDebuggingSession)
    {
        amountOfWavefronts = 1;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getActiveWavefrontID
// Description: Gets the kernel wavefront id (only one wavefront, #1), if kernel
//              debugging is active
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        30/10/2013
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getActiveWavefrontID(int wavefrontIndex, int& wavefrontId)
{
    bool retVal = false;
    wavefrontId = -1;

    if ((NULL != _pCurrentKernelDebuggingSession) && (0 == wavefrontIndex))
    {
        wavefrontId = 1;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getWavefrontIndex
// Description: Gets the wavefront index if it is available
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/3/2013
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getWavefrontIndex(const int coordinate[3], int& o_wavefrontIndex)
{
    o_wavefrontIndex = -1;
    bool retVal = false;

    if (NULL != _pCurrentKernelDebuggingSession)
    {
        if (isWorkItemValid(coordinate))
        {
            o_wavefrontIndex = 0;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::exportVariableValuesToFile
// Description: Exports the variable values to a file.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        3/3/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::exportVariableValuesToFile(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableDataFilePath)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // Get the dimensions:
        int dataXSize = std::max(_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0], 1);
        int dataYSize = std::max(_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1], 1);
        int dataZSize = std::max(_pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2], 1);
        int totalWorkSize = _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize;

        static const gtString executionMaskPseudoVariableName = CS_STR_KernelDebuggingExecutionMaskPseudoVariableName;
        static const gtString wavefrontMaskPseudoVariableName = CS_STR_KernelDebuggingWavefrontMaskPseudoVariableName;
        static const gtString availabilityMaskPseudoVariableNamePrefix = CS_STR_KernelDebuggingAvailabilityMaskPseudoVariableNamePrefix;
        bool startsWithAvailPrefix = variableName.startsWith(availabilityMaskPseudoVariableNamePrefix);

        if ((variableName != executionMaskPseudoVariableName) && (variableName != wavefrontMaskPseudoVariableName) && (!startsWithAvailPrefix))
        {
            // Get the values buffer from the debugging API:
            const void* variableValues = NULL;
            unsigned int variableValuesStrideSize = 0;
            csDWARFVariable variableDetails;
            bool wasVariableValuesBufferAllocated = false;
            bool rcVal = getVariableValuesInCurrentLocation(variableName, -1, variableValues, variableValuesStrideSize, variableDetails, wasVariableValuesBufferAllocated);

            if (rcVal)
            {
                // Verify the values we got make sense:
                gtUInt32& variableValueSize = variableDetails._valueSize;

                if (variableDetails._valueIsPointer)
                {
                    variableValueSize = (gtUInt32)CS_KERNEL_DEBUGGING_DEFAULT_SIZE_OF_POINTER;
                    int pointerSize = _pCurrentKernelDebuggingSession->_pDWARFParser->dwarfPointerSize();

                    if (0 < pointerSize)
                    {
                        variableValueSize = (gtUInt32)pointerSize;
                    }
                }

                GT_IF_WITH_ASSERT((variableValueSize != 0))
                {
                    // Get the format needed for this variable:
                    oaTexelDataFormat dataFormat;
                    oaDataType dataType;
                    variableDetails.getDataFormat(dataFormat, dataType);

                    if ((dataFormat != OA_TEXEL_FORMAT_UNKNOWN) && (dataType != OA_UNKNOWN_DATA_TYPE))
                    {
                        // Create a buffer for the data:
                        int totalBufferSize = totalWorkSize * (int)variableValueSize;
                        gtByte* pDataBuffer = new gtByte[totalBufferSize];
                        ::memset(pDataBuffer, 0, totalBufferSize);

                        // Copy the value from each variable into the buffer:
                        bool copiedValues = false;

                        if (variableDetails._variableLocation._variableLocation != GT_UINT64_MAX)
                        {
                            copiedValues = true;

                            for (int i = 0; i < totalWorkSize; i++)
                            {
                                gtSize_t sourceLocation = (gtSize_t)variableValues + (gtSize_t)(i * variableValuesStrideSize);
                                gtSize_t destinationLocation = (gtSize_t)pDataBuffer + (gtSize_t)(i * variableValueSize);
                                ::memcpy((void*)destinationLocation, (const void*)sourceLocation, variableValueSize);
                            }
                        }
                        else // variableDetails._variableLocation == GT_UINT64_MAX
                        {
                            // A variable must have a constant value or a location:
                            GT_IF_WITH_ASSERT(variableDetails._variableConstantValueExists)
                            {
                                copiedValues = true;
                                // Copy the value to all locations:
                                gtSize_t copySize = std::min((gtSize_t)variableValueSize, sizeof(gtUInt64));

                                for (int i = 0; i < totalWorkSize; i++)
                                {
                                    gtSize_t destinationLocation = (gtSize_t)pDataBuffer + (gtSize_t)(i * variableValueSize);
                                    ::memcpy((void*)destinationLocation, (const void*)&variableDetails._variableConstantValue, copySize);
                                }
                            }
                        }

                        GT_IF_WITH_ASSERT(copiedValues)
                        {
                            // Create a raw file serializer:
                            oaRawFileSeralizer rawFileSerializer;
                            rawFileSerializer.setRawData(pDataBuffer);
                            rawFileSerializer.setRawDataDimensions(dataXSize, dataYSize);
                            rawFileSerializer.setRawDataFormat(dataFormat, dataType);
                            rawFileSerializer.setAmountOfPages(dataZSize);

                            // Create a file path:
                            osFilePath outputFilePath;
                            filePathFromVariableName(variableName, outputFilePath);

                            // Output the data to the file:
                            retVal = rawFileSerializer.saveToFile(outputFilePath);

                            if (retVal)
                            {
                                // Return the file path:
                                variableTypeSupported = true;
                                variableDataFilePath = outputFilePath;
                            }

                            // Release the data pointer from the serializer, and delete the buffer:
                            rawFileSerializer.getRawDataPointer(true);
                            delete[] pDataBuffer;
                        }
                    }
                    else // (dataFormat == OA_TEXEL_FORMAT_UNKNOWN) || (dataType == OA_UNKNOWN_DATA_TYPE)
                    {
                        // We found the variable, but could not get its values to a file since it is not a supported type (e.g. it's a compound type):
                        retVal = true;
                        variableTypeSupported = false;
                    }
                }

                // Release the variable data:
                releaseVariableValues(variableValues, wasVariableValuesBufferAllocated);
            }
        }
        else if (startsWithAvailPrefix)
        {
            // If the availability mask was requested - since a variable is either available to all work items, or to none - return a mask full of 0s or 1s, accordingly.
            static const int availMaskPrefixLength = availabilityMaskPseudoVariableNamePrefix.length();
            gtString varName = variableName;
            varName.truncate(availMaskPrefixLength, variableName.length() - 1);

            // Check if it exists:
            int dummyCoord[3] = {0};
            bool varExists = doesVariableExistInCurrentScope(varName, dummyCoord);

            gtUByte* pDataBuffer = new gtUByte[totalWorkSize];
            ::memset(pDataBuffer, 0, totalWorkSize);

            // Create the availability mask:
            for (int i = 0; i < totalWorkSize; i++)
            {
                pDataBuffer[i] = varExists ? 1 : 0;
            }

            // Create a raw file serializer:
            oaRawFileSeralizer rawFileSerializer;
            rawFileSerializer.setRawData((gtByte*)pDataBuffer);
            rawFileSerializer.setRawDataDimensions(dataXSize, dataYSize);
            rawFileSerializer.setRawDataFormat(OA_TEXEL_FORMAT_VARIABLE_VALUE, OA_UNSIGNED_BYTE);
            rawFileSerializer.setAmountOfPages(dataZSize);

            // Create a file path:
            osFilePath outputFilePath;
            filePathFromVariableName(variableName, outputFilePath);

            // Output the data to the file:
            retVal = rawFileSerializer.saveToFile(outputFilePath);

            if (retVal)
            {
                // Return the file path:
                variableDataFilePath = outputFilePath;
            }
        }
        else // variableName == executionMaskPseudoVariableName || variableName != wavefrontMaskPseudoVariableName
        {
            // Note that since the software debugger only has one virtual wavefront, the execution mask is equal to the wavefront mask.
            // If the execution mask is updated:
            GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems != NULL)
            {
                // We requested the execution mask, create a buffer for its data:
                gtUByte* pDataBuffer = new gtUByte[totalWorkSize];
                ::memset(pDataBuffer, 0, totalWorkSize);

                // Copy the execution mask:
                for (int i = 0; i < totalWorkSize; i++)
                {
                    pDataBuffer[i] = _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems[i] ? 1 : 0;
                }

                // Create a raw file serializer:
                oaRawFileSeralizer rawFileSerializer;
                rawFileSerializer.setRawData((gtByte*)pDataBuffer);
                rawFileSerializer.setRawDataDimensions(dataXSize, dataYSize);
                rawFileSerializer.setRawDataFormat(OA_TEXEL_FORMAT_VARIABLE_VALUE, OA_UNSIGNED_BYTE);
                rawFileSerializer.setAmountOfPages(dataZSize);

                // Create a file path:
                osFilePath outputFilePath;
                filePathFromVariableName(variableName, outputFilePath);

                // Output the data to the file:
                retVal = rawFileSerializer.saveToFile(outputFilePath);

                if (retVal)
                {
                    // Return the file path:
                    variableDataFilePath = outputFilePath;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getKernelSourceBreakpointResolution
// Description: Checks if the breakpoint requested is currently resolved. If it
//              is, returns the line where it was resolved
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/5/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::getKernelSourceBreakpointResolution(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber)
{
    bool retVal = false;

    // Sanity check:
    if (_pCurrentKernelDebuggingSession != NULL)
    {
        // Verify the program handle is correct:
        if (programHandle == _pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramHandle)
        {
            // Uri, 30/11/11 - we currently only support breakpoints in the main kernel file.isEmpty
            osFilePath programSourcePath(_pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramSourceFilePath);

            // Get the program counters where the breakpoint would be bound:
            gtVector<amdclDebugPC> requestedPCs;
            kernelProgramCountersFromLineNumber(_pCurrentKernelDebuggingSession->_kernelDebugContext, programSourcePath, requestedLineNumber, requestedPCs);

            // Try to find one that was bound successfully:
            int numberOfRequestedPCs = (int)requestedPCs.size();
            const gtMap<amdclDebugPC, amdclDebugPC>& requestedToBoundBreakpoints = _pCurrentKernelDebuggingSession->_debuggedKernelRequestedToBoundBreakpoints;
            gtMap<amdclDebugPC, amdclDebugPC>::const_iterator endIter = requestedToBoundBreakpoints.end();

            for (int i = 0; i < numberOfRequestedPCs; i++)
            {
                // Try to find this PC in the bindings map:
                gtMap<amdclDebugPC, amdclDebugPC>::const_iterator findIter = requestedToBoundBreakpoints.find(requestedPCs[i]);

                if (findIter != endIter)
                {
                    // Found a bound breakpoint, return its line number:
                    gtString ignored;
                    int resolvedLine = lineNumberFromKernelProgramCounter(_pCurrentKernelDebuggingSession->_kernelDebugContext, requestedPCs[i], ignored);

                    if (resolvedLine > -1)
                    {
                        // Found a valid line number, stop looking:
                        resolvedLineNumber = resolvedLine;
                        retVal = true;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::synchronizeWithCSKernelDebuggingCallback
// Description: Syncs the calling thread with the CS (software-based) kernel debugging
//              manager ONLY
// Author:      Uri Shomroni
// Date:        28/4/2014
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::synchronizeWithCSKernelDebuggingCallback()
{
    OS_OUTPUT_DEBUG_LOG(L"Sync with *CS* kernel debugging callback started", OS_DEBUG_LOG_EXTENSIVE);
    osCriticalSectionLocker csSynchro(cs_stat_amdKernelDebuggingManager.m_sCriticalSection);
    OS_OUTPUT_DEBUG_LOG(L"Sync with *CS* kernel debugging callback ended", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::csAMDclDebugCallback
// Description: Function called by the kernel debugging API to handle kernel debugging events.
//              Gets the "break reason" and returns the next action to be performed.
// Author:      Uri Shomroni
// Date:        24/10/2010
// ---------------------------------------------------------------------------
amdclDebugCommand csAMDKernelDebuggingManager::csAMDclDebugCallback(amdclDebugContext debugContext, amdclDebugEvent debugEvent, void* user_data)
{
    // Log entering the callback before syncing with the CS:
    osDebugLogSeverity loggedSeverity = osDebugLog::instance().loggedSeverity();

    if (OS_DEBUG_LOG_EXTENSIVE <= loggedSeverity)
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"csAMDclDebugCallback(%p, %#x, %p) - before sync", (void*)debugContext, (unsigned int)debugEvent, user_data);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    // Lock the critical section:
    csAMDKernelDebuggingManagerCallbackSynchronizer csSynchro(cs_stat_amdKernelDebuggingManager.m_sCriticalSection, cs_stat_amdKernelDebuggingManager._pCurrentKernelDebuggingSession);
    GT_ASSERT(cs_stat_amdKernelDebuggingManager._pCurrentKernelDebuggingSession == NULL);
    csAMDKernelDebuggingSession* pCurrentKernelDebuggingSession = (csAMDKernelDebuggingSession*)user_data;
    cs_stat_amdKernelDebuggingManager._pCurrentKernelDebuggingSession = pCurrentKernelDebuggingSession;
    GT_ASSERT(pCurrentKernelDebuggingSession != NULL);

    OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - after sync", OS_DEBUG_LOG_EXTENSIVE);

    // Handle the event:
    bool shouldBreak = false;
    apBreakReason brkReason = AP_STEP_OVER_BREAKPOINT_HIT;
    suIKernelDebuggingManager::KernelDebuggingSessionReason debuggingReason = pCurrentKernelDebuggingSession->_kernelDebuggingReason;

    // Act according to the debug event type:
    switch (debugEvent)
    {
        case AMDCLDEBUG_EVENT_STARTED:
        {
            OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - AMDCLDEBUG_EVENT_STARTED", OS_DEBUG_LOG_EXTENSIVE);

            // The kernel just started running.
            // If the "break in kernel by function name" field was filled:
            if (debuggingReason == suIKernelDebuggingManager::KERNEL_FUNCTION_NAME_BREAKPOINT)
            {
                // We got here because of a kernel breakpoint:
                shouldBreak = true;
                brkReason = AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT;
            }
            // If we stepped in:
            else if (debuggingReason == suIKernelDebuggingManager::STEP_IN_COMMAND)
            {
                // We got here by a "step in" command, so we need to break:
                shouldBreak = true;
                brkReason = AP_STEP_IN_BREAKPOINT_HIT;
            }

            // Initialize the debug session:
            cs_stat_amdKernelDebuggingManager.onKernelDebuggingStartedBreakpoint(debugContext);
        }
        break;

        case AMDCLDEBUG_EVENT_HALTED:
        {
            OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - AMDCLDEBUG_EVENT_HALTED", OS_DEBUG_LOG_EXTENSIVE);

            // We hit a breakpoint or a step ended:
            shouldBreak = true;

            // Check what was the command that brought us here:
            if (pCurrentKernelDebuggingSession->_nextDebuggingCommand == AMDCLDEBUG_COMMAND_STEP)
            {
                // We just finished a step:
                brkReason = pCurrentKernelDebuggingSession->_continuedStepBreakReason;
            }
            else // _nextDebuggingCommand != AMDCLDEBUG_COMMAND_STEP
            {
                // We just hit a breakpoint:
                brkReason = AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT;
            }

            // This is not the first break:
            pCurrentKernelDebuggingSession->_debuggedKernelFirstCallback = false;
        }
        break;

        case AMDCLDEBUG_EVENT_FINISHED:
        {
            OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - AMDCLDEBUG_EVENT_FINISHED", OS_DEBUG_LOG_EXTENSIVE);

            // Terminate the debug session:
            cs_stat_amdKernelDebuggingManager.onKernelDebuggingFinishedBreakpoint(debugContext);
        }
        break;

        case AMDCLDEBUG_EVENT_ERROR:
        {
            OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - AMDCLDEBUG_EVENT_ERROR", OS_DEBUG_LOG_EXTENSIVE);

            // There has been a problem with the kernel debugging, report it:
            gtString lastErrorString;
            debugLastErrorString(debugContext, lastErrorString);
            cs_stat_openCLMonitorInstance.reportKernelDebuggingFailure(CL_SUCCESS, CL_SUCCESS, lastErrorString);

            // Since a message is shown to the user, we want to always stop after these events:
            pCurrentKernelDebuggingSession->_nextDebuggingCommand = AMDCLDEBUG_COMMAND_STEP;

            // Cleanup, as we are about to exit kernel debugging:
            cs_stat_amdKernelDebuggingManager.cleanupOnKernelDebuggingEnded();
        }
        break;

        case AMDCLDEBUG_EVENT_UNINITIALIZED:
        case AMDCLDEBUG_EVENT_MAX:
        {
            OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - Bad value", OS_DEBUG_LOG_EXTENSIVE);

            // These values should never occur!
            GT_ASSERT(false);
        }
        break;

        default:
        {
            OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - Unexpected value", OS_DEBUG_LOG_EXTENSIVE);
            // Unexpected value!
            GT_ASSERT(false);
        }
        break;
    }

    if (shouldBreak)
    {
        OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - about to break", OS_DEBUG_LOG_EXTENSIVE);

        // Reset the next command value to "continue":
        pCurrentKernelDebuggingSession->_nextDebuggingCommand = AMDCLDEBUG_COMMAND_CONTINUE;

        // Suspend the debugged process:
        cs_stat_amdKernelDebuggingManager.onKernelDebuggingBreakpointHit(debugContext, brkReason);

        OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - after breakpoint", OS_DEBUG_LOG_EXTENSIVE);
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - continuing execution", OS_DEBUG_LOG_EXTENSIVE);

        // Continue:
        pCurrentKernelDebuggingSession->_nextDebuggingCommand = AMDCLDEBUG_COMMAND_CONTINUE;
    }

    // If this is the "finished" event, dispose of the session structure:
    amdclDebugCommand retVal = pCurrentKernelDebuggingSession->_nextDebuggingCommand;

    if ((debugEvent == AMDCLDEBUG_EVENT_FINISHED) || (debugEvent == AMDCLDEBUG_EVENT_ERROR))
    {
        OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - debugging ended, starting cleanup", OS_DEBUG_LOG_EXTENSIVE);

        // This has to be set before the deletion, since functions waiting for kernel debugging to
        // end could hit it just as it's being deleted:
        cs_stat_amdKernelDebuggingManager._pCurrentKernelDebuggingSession = NULL;

        // If this is executed asynchronously:
        if (pCurrentKernelDebuggingSession->_canDebugSessionBeDeleted)
        {
            // The wrapper no longer needs this struct, delete it:
            delete pCurrentKernelDebuggingSession;
        }
        else // !pCurrentKernelDebuggingSession->_canDebugSessionBeDeleted
        {
            // This is synchronous execution, the structure will be released by the wrapper function:
            pCurrentKernelDebuggingSession->_canDebugSessionBeDeleted = true;
        }
    }
    // Breakpoints cannot be manipulated during the FINISHED callback:
    else // ((debugEvent != AMDCLDEBUG_EVENT_FINISHED) && (debugEvent != AMDCLDEBUG_EVENT_ERROR))
    {
        OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - emulating step", OS_DEBUG_LOG_EXTENSIVE);

        // Emulate steps by setting breakpoints on all line numbers we have PCs for:
        cs_stat_amdKernelDebuggingManager.emulateStepCommand(retVal, debugContext);
        cs_stat_amdKernelDebuggingManager._pCurrentKernelDebuggingSession = NULL;
    }

    // Releasing the critical sections is done by csSynchro:
    // cs_stat_amdKernelDebuggingManager._kernelDebuggingCS.leave();

    OS_OUTPUT_DEBUG_LOG(L"csAMDclDebugCallback() - about to leave sync", OS_DEBUG_LOG_EXTENSIVE);

    // Let the API know what the next command is:
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::onKernelDebuggingStartedBreakpoint
// Description: Called when the AMDCLDEBUG_EVENT_STARTED event is hit
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::onKernelDebuggingStartedBreakpoint(amdclDebugContext debugContext)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        //////////////////////////////////////////////////////////////////////////
        // Initialize kernel debugging in the client:
        //////////////////////////////////////////////////////////////////////////

        // Send a "before kernel debugging" event to the client:
        osThreadId currentThreadId = osGetCurrentThreadId();
        int* globalWorkOffsetFromSession = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset;
        int* globalWorkSizeFromSession = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize;
        int* localWorkSizeFromSession = _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize;
        gtSize_t globalWorkOffset[3] = {(globalWorkSizeFromSession[0] > 0) ? (gtSize_t)globalWorkOffsetFromSession[0] : 0,
                                        (globalWorkSizeFromSession[1] > 0) ? (gtSize_t)globalWorkOffsetFromSession[1] : 0,
                                        (globalWorkSizeFromSession[2] > 0) ? (gtSize_t)globalWorkOffsetFromSession[2] : 0
                                       };
        gtSize_t globalWorkSize[3] = {(globalWorkSizeFromSession[0] > 0) ? (gtSize_t)globalWorkSizeFromSession[0] : 1,
                                      (globalWorkSizeFromSession[1] > 0) ? (gtSize_t)globalWorkSizeFromSession[1] : 1,
                                      (globalWorkSizeFromSession[2] > 0) ? (gtSize_t)globalWorkSizeFromSession[2] : 1
                                     };
        gtSize_t localWorkSize[3] = {(globalWorkSizeFromSession[0] > 0) ? (gtSize_t)localWorkSizeFromSession[0] : 1,
                                     (globalWorkSizeFromSession[1] > 0) ? (gtSize_t)localWorkSizeFromSession[1] : 1,
                                     (globalWorkSizeFromSession[2] > 0) ? (gtSize_t)localWorkSizeFromSession[2] : 1
                                    };

        apBeforeKernelDebuggingEvent beforeKernelDebuggingEvent(apBeforeKernelDebuggingEvent::AP_OPENCL_SOFTWARE_KERNEL_DEBUGGING, currentThreadId, currentlyDebuggedKernelWorkDimension(), globalWorkOffset, globalWorkSize, localWorkSize);
        bool rcEve = suForwardEventToClient(beforeKernelDebuggingEvent);
        GT_ASSERT(rcEve);

        // Trigger a breakpoint for the debug start, so the debugger could freeze all other threads:
        // [before|after]TriggeringBreakpoint() are called from the main callback function, so they are not needed here
        // su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
        su_stat_theBreakpointsManager.setBreakReason(AP_BEFORE_KERNEL_DEBUGGING_HIT);
        su_stat_theBreakpointsManager.triggerBreakpointException(_pCurrentKernelDebuggingSession->_debuggedKernelComputeContextId, _pCurrentKernelDebuggingSession->_debuggingKernelFunction);
        // su_stat_theBreakpointsManager.afterTriggeringBreakpoint();

        // Get the binary and pass it to the DWARF parser:

        //////////////////////////////////////////////////////////////////////////
        // TO_DO: Uri, 19/12/10 - there is currently a problem with the debug API removing the DWARF sections of binaries, we're using the OpenCL API instead.
        // TO_DO: Uri, 6/2/11 - The DBE team's code for the get binaries API is not yet fully tested, keeping the workaround code here:
        /*
        cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle((cl_program)(cs_stat_openCLMonitorInstance.programContainingKernel(_currentlyDebuggedKernelHandle)));
        gtSize_t binarySizesSize = 0;
        cs_stat_realFunctionPointers.clGetProgramInfo(programInternalHandle, CL_PROGRAM_BINARY_SIZES, 0, NULL, &binarySizesSize);
        int numberOfBinaries = (int)(binarySizesSize / sizeof(gtSize_t));
        gtSize_t* binarySizes = new gtSize_t[numberOfBinaries];
        cs_stat_realFunctionPointers.clGetProgramInfo(programInternalHandle, CL_PROGRAM_BINARY_SIZES, binarySizesSize, binarySizes, NULL);
        unsigned char** binaries = new unsigned char*[numberOfBinaries];
        for (int i = 0; i < numberOfBinaries; i++)
        {
        binaries[i] = new unsigned char[binarySizes[i]];
        }

        cs_stat_realFunctionPointers.clGetProgramInfo(programInternalHandle, CL_PROGRAM_BINARIES, sizeof(unsigned char*) * numberOfBinaries, binaries, NULL);

        _currentlyDebuggedKernelBinary = binaries[0];
        binaries[0] = NULL;
        _currentlyDebuggedKernelBinarySize = binarySizes[0];

        for (int i = 0; i < numberOfBinaries; i++)
        {
        delete[] binaries[i];
        }
        delete[] binaries;
        */
        //
        //////////////////////////////////////////////////////////////////////////

        amdclDebugError rcSz = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetKernelBinarySize(debugContext, &_pCurrentKernelDebuggingSession->_debuggedKernelBinarySize);
        GT_IF_WITH_ASSERT(rcSz == AMDCLDEBUG_SUCCESS && (_pCurrentKernelDebuggingSession->_debuggedKernelBinarySize > 0))
        {
            GT_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelBinary == NULL);
            _pCurrentKernelDebuggingSession->_debuggedKernelBinary = new gtUByte[_pCurrentKernelDebuggingSession->_debuggedKernelBinarySize];
            amdclDebugError rcBin = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetKernelBinary(debugContext, (void*)_pCurrentKernelDebuggingSession->_debuggedKernelBinary, _pCurrentKernelDebuggingSession->_debuggedKernelBinarySize);
            GT_IF_WITH_ASSERT(rcBin == AMDCLDEBUG_SUCCESS)
            {
                // Parse the binaries:
                csDWARFParser& theDWARFParser = *_pCurrentKernelDebuggingSession->_pDWARFParser;
                bool rcDWI = theDWARFParser.initializeWithBinary((const void*)_pCurrentKernelDebuggingSession->_debuggedKernelBinary, _pCurrentKernelDebuggingSession->_debuggedKernelBinarySize);
                GT_IF_WITH_ASSERT(rcDWI)
                {
                    // If we got DWARF information, let the debug API know what are the registers we are going to access:
                    gtVector<gtUInt64> usedRegisters;
                    bool rcLoc = theDWARFParser.listVariableRegisterLocations(usedRegisters);

                    if (rcLoc)
                    {
                        // Uri, 29/7/14 - Always add the indexed temp x0[0], which is the location of the private stack pointer.
                        usedRegisters.push_back(CS_KERNEL_DEBUGGING_INDEXED_TEMP_REGISTER_LOCATOR(0, 0));

                        // Create a buffer for the locations:
                        gtSize_t numberOfLocations = usedRegisters.size();
                        amdclDebugRegisterLocator* pLocations = new amdclDebugRegisterLocator[numberOfLocations];

                        // Copy the values:
                        for (gtSize_t i = 0; i < numberOfLocations; i++)
                        {
                            pLocations[i] = (amdclDebugRegisterLocator)usedRegisters[i];
                        }

                        // Pass the locations to the API:
                        amdclDebugError rcTrc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetTrackingRegisters(debugContext, pLocations, numberOfLocations);
                        GT_ASSERT(rcTrc == AMDCLDEBUG_SUCCESS);

                        // Delete the buffer
                        delete[] pLocations;
                    }

                    // Get the list of program counters that are mapped to line numbers:
                    gtVector<csDwarfAddressType> mappedAddresses;
                    bool rcPC = theDWARFParser.getMappedAddresses(mappedAddresses);

                    if (rcPC)
                    {
                        // Copy the addresses into the addresses list:
                        gtSize_t numberOfPCs = mappedAddresses.size();

                        for (gtSize_t i = 0; i < numberOfPCs; i++)
                        {
                            _pCurrentKernelDebuggingSession->_debuggedKernelAllPCsWithLineNumbers.push_back((amdclDebugPC)mappedAddresses[i]);
                        }
                    }
                }
            }
        }

        // Set the breakpoints that are in this kernel:
        su_stat_theBreakpointsManager.setKernelSourceBreakpointsDirty(true);
        _pCurrentKernelDebuggingSession->_debuggedKernelBoundToRequestedBreakpoints.clear();
        _pCurrentKernelDebuggingSession->_debuggedKernelRequestedToBoundBreakpoints.clear();
        updateKernelBreakpoints(debugContext);

        // Set the first valid line number that comes up as a new line number, as well as the program counters:
        _pCurrentKernelDebuggingSession->_lastDebuggedKernelLineNumber = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter = CS_AMD_DEBUG_PC_NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelLastProgramCounter = CS_AMD_DEBUG_PC_NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelLastStoppedProgramCounter = CS_AMD_DEBUG_PC_NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelFirstCallback = true;
        _pCurrentKernelDebuggingSession->_debuggedKernelFirstBreakpoint = true;
        _pCurrentKernelDebuggingSession->_continuedStepBreakReason = AP_STEP_OVER_BREAKPOINT_HIT;

        // Create a buffer for the work item execution mask:
        delete[] _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems;
        GT_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems == NULL);
        GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize > 0)
        {
            _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems = new bool[_pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize];
            ::memset(_pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems, 0, sizeof(bool) * _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::onKernelDebuggingFinishedBreakpoint
// Description: Called when the AMDCLDEBUG_EVENT_STARTED event is hit
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::onKernelDebuggingFinishedBreakpoint(amdclDebugContext debugContext)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // Report special failures:
        if (_pCurrentKernelDebuggingSession->_continuedStepBreakReason != AP_STEP_OVER_BREAKPOINT_HIT)
        {
            // We were stepping in or entering for a kernel function name breakpoint, but there was no
            // debuggable code:
            if ((_pCurrentKernelDebuggingSession->_continuedStepBreakReason == AP_STEP_IN_BREAKPOINT_HIT) && (_pCurrentKernelDebuggingSession->_debuggedKernelFirstBreakpoint))
            {
                // We currently only report this for the "step in" scenario, to avoid having the UI flooded
                // with message windows:
                gtString lastErrorString;
                debugLastErrorString(debugContext, lastErrorString);
                cs_stat_openCLMonitorInstance.reportKernelDebuggingFailure(CL_SUCCESS, CL_SUCCESS, lastErrorString);
            }

            _pCurrentKernelDebuggingSession->_continuedStepBreakReason = AP_STEP_OVER_BREAKPOINT_HIT;
        }

        //////////////////////////////////////////////////////////////////////////
        // Terminate kernel debugging in the client:
        //////////////////////////////////////////////////////////////////////////
        // Send an "after kernel debugging" event to the client:
        osThreadId currentThreadId = osGetCurrentThreadId();
        apAfterKernelDebuggingEvent afterKernelDebuggingEvent(currentThreadId);
        bool rcEve = suForwardEventToClient(afterKernelDebuggingEvent);
        GT_ASSERT(rcEve);

        // Trigger a breakpoint for the debug start, so the debugger could release all threads frozen before:
        // [before|after]TriggeringBreakpoint() are called from the main callback function, so they are not needed here
        // su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
        su_stat_theBreakpointsManager.setBreakReason(AP_AFTER_KERNEL_DEBUGGING_HIT);
        su_stat_theBreakpointsManager.triggerBreakpointException(_pCurrentKernelDebuggingSession->_debuggedKernelComputeContextId, _pCurrentKernelDebuggingSession->_debuggingKernelFunction);
        // su_stat_theBreakpointsManager.afterTriggeringBreakpoint();

        // Basic cleanup:
        cleanupOnKernelDebuggingEnded();
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::onKernelDebuggingBreakpointHit
// Description: Called when the debugged process run should be suspended while
//              debugging a kernel, throws the breakpoint and waits on it.
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::onKernelDebuggingBreakpointHit(amdclDebugContext debugContext, apBreakReason brkReason)
{
    // Sanity check:
    GT_ASSERT(_pCurrentKernelDebuggingSession != NULL);

    // Copy the context:
    _pCurrentKernelDebuggingSession->_kernelDebugContext = debugContext;

    // These operations cannot be done during the "debugging started" breakpoint:
    if (!_pCurrentKernelDebuggingSession->_debuggedKernelFirstCallback)
    {
        // Get the program counter:
        _pCurrentKernelDebuggingSession->_debuggedKernelLastProgramCounter = _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter;
        amdclDebugPC newPC = CS_AMD_DEBUG_PC_NULL;
        amdclDebugError rcPC = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetProgramCounter(_pCurrentKernelDebuggingSession->_kernelDebugContext, &newPC);

        if (rcPC == AMDCLDEBUG_SUCCESS)
        {
            _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter = newPC;
        }

        // Update the execution mask:
        updateValidWorkItems();
    }

    // To allow stepping in the current work item and stepping by line instead of by instruction,
    // we allow a step command to be repeated:
    bool continueStepping = false;

    if ((brkReason == AP_STEP_OVER_BREAKPOINT_HIT) || (brkReason == AP_STEP_IN_BREAKPOINT_HIT) || (brkReason == AP_STEP_OUT_BREAKPOINT_HIT) || (brkReason == AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT))
    {
        continueStepping = shouldContinueStepping(brkReason);
    }

    // If this wasn't a step or we need to stop stepping:
    if (!continueStepping)
    {
        // Lock the execution of this function and let CodeXL know we are waiting to be released:
        _pCurrentKernelDebuggingSession->_kernelDebuggingCallbackWaiting = true;

        // Trigger a breakpoint.
        // !!! Note that the su_stat_theBreakpointsManager.[beforeTriggeringBreakpointException|afterTriggeringBreakpointException] calls for this are in the main callback function !!!
        su_stat_theBreakpointsManager.setBreakReason(brkReason);
        _pCurrentKernelDebuggingSession->_continuedStepBreakReason = AP_STEP_OVER_BREAKPOINT_HIT;
        su_stat_theBreakpointsManager.triggerBreakpointException(_pCurrentKernelDebuggingSession->_debuggedKernelComputeContextId, _pCurrentKernelDebuggingSession->_debuggingKernelFunction);
        _pCurrentKernelDebuggingSession->_debuggedKernelFirstBreakpoint = false;

        // Wait for the API to release us:
        osWaitForFlagToTurnOff(_pCurrentKernelDebuggingSession->_kernelDebuggingCallbackWaiting, ULONG_MAX);

        // Record the line number and program counter here, for stepping purposes:
        _pCurrentKernelDebuggingSession->_lastDebuggedKernelLineNumber = getCurrentKernelDebugLineNumber(_pCurrentKernelDebuggingSession->_steppingWorkItem);
        _pCurrentKernelDebuggingSession->_debuggedKernelLastStoppedProgramCounter = _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter;

        // Update the breakpoints selected before resuming:
        updateKernelBreakpoints(_pCurrentKernelDebuggingSession->_kernelDebugContext);
    }
    else
    {
        // Continue stepping:
        _pCurrentKernelDebuggingSession->_nextDebuggingCommand = AMDCLDEBUG_COMMAND_STEP;
        _pCurrentKernelDebuggingSession->_continuedStepBreakReason = brkReason;
    }

    // Clear the context as it is about to be invalidated:
    _pCurrentKernelDebuggingSession->_kernelDebugContext = CS_AMD_DEBUG_CONTEXT_NULL;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::cleanupOnKernelDebuggingEnded
// Description: Perform the cleanup operations needed when kernel debugging ends,
//              whether it is by finished event or by error.
// Author:      Uri Shomroni
// Date:        31/10/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::cleanupOnKernelDebuggingEnded()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // The kernel finished running:
        _pCurrentKernelDebuggingSession->_debuggedKernelCurrentBreakpoints.clear();
        _pCurrentKernelDebuggingSession->_areStepBreakpointsSet = false;
        _pCurrentKernelDebuggingSession->_stepBreakpointsSetSuccessfully = false;

        // If the last command was one of the steps (step in / over / out), we want to break at the next function call, as we stepped out of the kernel:
        if ((_pCurrentKernelDebuggingSession->_nextDebuggingCommand == AMDCLDEBUG_COMMAND_STEP) || (_pCurrentKernelDebuggingSession->_continuedStepBreakReason == AP_STEP_OUT_BREAKPOINT_HIT))
        {
            // Don't do this for when we could not debug a kernel by its name:
            if (_pCurrentKernelDebuggingSession->_continuedStepBreakReason != AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT)
            {
                su_stat_theBreakpointsManager.setBreakpointAtNextMonitoredFunctionCall();
            }
        }

        // Terminate the DWARF session:
        csDWARFParser* pDWARFParser = _pCurrentKernelDebuggingSession->_pDWARFParser;

        if (pDWARFParser != NULL)
        {
            if (pDWARFParser->isInitialized())
            {
                bool rcDWT = _pCurrentKernelDebuggingSession->_pDWARFParser->terminate();
                GT_ASSERT(rcDWT);
            }
        }

        // Let the breakpoints manager know that kernel source breakpoints are not bound
        su_stat_theBreakpointsManager.setKernelSourceBreakpointsDirty(true);

        // Clean up the data structures:
        delete[] _pCurrentKernelDebuggingSession->_debuggedKernelBinary;
        _pCurrentKernelDebuggingSession->_debuggedKernelBinary = NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelBinarySize = 0;
        _pCurrentKernelDebuggingSession->_lastDebuggedKernelLineNumber = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter = CS_AMD_DEBUG_PC_NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelLastProgramCounter = CS_AMD_DEBUG_PC_NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelLastStoppedProgramCounter = CS_AMD_DEBUG_PC_NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelFirstCallback = true;
        delete[] _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems;
        _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems = NULL;
        _pCurrentKernelDebuggingSession->_debuggedKernelBoundToRequestedBreakpoints.clear();
        _pCurrentKernelDebuggingSession->_debuggedKernelRequestedToBoundBreakpoints.clear();

        // Reset the debugged kernel handle, function and dimensions:
        _pCurrentKernelDebuggingSession->_debuggedKernelHandle = OA_CL_NULL_HANDLE;
        _pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramHandle = OA_CL_NULL_HANDLE;
        _pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramSourceFilePath.makeEmpty();
        apContextID nullContextId;
        _pCurrentKernelDebuggingSession->_debuggedKernelComputeContextId = nullContextId;
        _pCurrentKernelDebuggingSession->_debuggingKernelFunction = apMonitoredFunctionsAmount;
        _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset[0] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset[1] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkOffset[2] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[0] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[1] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize[2] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize[0] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize[1] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelLocalWorkSize[2] = -1;
        _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize = -1;
        _pCurrentKernelDebuggingSession->_steppingWorkItem[0] = -1;
        _pCurrentKernelDebuggingSession->_steppingWorkItem[1] = -1;
        _pCurrentKernelDebuggingSession->_steppingWorkItem[2] = -1;
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::updateValidWorkItems
// Description: Updates the valid work items mask in our members
// Author:      Uri Shomroni
// Date:        15/2/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::updateValidWorkItems()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems != NULL)
        {
            // Get the execution mask from the API:
            size_t maskSize = 0;
            const cl_bool* pExecutionMask = NULL;
            amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugGetExecutionMask(_pCurrentKernelDebuggingSession->_kernelDebugContext, &pExecutionMask, &maskSize);

            if ((rc == AMDCLDEBUG_SUCCESS) && (maskSize > 0))
            {
                // Copy the values to our mask:
                GT_ASSERT(_pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize == (int)maskSize);
                int lowestIndex = std::min((int)maskSize, _pCurrentKernelDebuggingSession->_debuggedKernelTotalGlobalWorkSize);
                bool* executionMask = _pCurrentKernelDebuggingSession->_debuggedKernelValidWorkItems;

                for (int i = 0; i < lowestIndex; i++)
                {
                    executionMask[i] = (pExecutionMask[i] == CL_TRUE);
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

                    if (pExecutionMask[i] != CL_TRUE && pExecutionMask[i] != CL_FALSE)
                    {
                        gtString logMsg;
                        logMsg.appendFormattedString(L"Work Item #%d, unexpected execution mask value %#llx", i, (unsigned long long)(pExecutionMask[i]));
                        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), (0 == i) ? OS_DEBUG_LOG_ERROR : OS_DEBUG_LOG_EXTENSIVE);
                    }

#endif
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::shouldContinueSteppingOver
// Description: Returns true if we want to continue the debug API-level stepping.
//              This allows aggregation of several debug API-level steps (step by
//              instruction) to larger steps, such as step by line.
// Author:      Uri Shomroni
// Date:        14/2/2011
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::shouldContinueStepping(apBreakReason brkReason)
{
    (void)(brkReason); // unused

    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // To implement step by line, we need to see if we changed lines. We also consider a step back in PCs to be a
        // new line even if it is not (e.g. if there is a single-line loop code):
        int currentLineNumber = getCurrentKernelDebugLineNumber(_pCurrentKernelDebuggingSession->_steppingWorkItem);

        if ((currentLineNumber <= 0) || ((_pCurrentKernelDebuggingSession->_lastDebuggedKernelLineNumber == currentLineNumber) && (_pCurrentKernelDebuggingSession->_debuggedKernelLastProgramCounter <= _pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter)))
        {
            retVal = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // Uri, 2/8/2011 - this part is only required if emulateStepCommand does
        // not perform the step out / over functionality. It also causes breakpoints
        // inside the stepped over region to be ignored. We need a better way to
        // work this if we want to use the AMDCLDEBUG_COMMAND_STEP again.
        //////////////////////////////////////////////////////////////////////////
        /*
        // To implement step over and step out, we need to compare stack depths:
        if (!retVal)
        {
        // Don't perform the calculations if they are irrelevant:
        if ((brkReason == AP_STEP_OVER_BREAKPOINT_HIT) || (brkReason == AP_STEP_OUT_BREAKPOINT_HIT))
        {
        // Get the stack depths:
        csDWARFParser& theDWARFParser = *_pCurrentKernelDebuggingSession->_pDWARFParser;
        int currentStackDepth = theDWARFParser.addressStackDepth(_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter);
        int lastStoppedStackDepth = theDWARFParser.addressStackDepth(_pCurrentKernelDebuggingSession->_debuggedKernelLastStoppedProgramCounter);

        // Step over requires a stack which is no deeper. Step Out requires a stack which is less deep:
        if ((currentStackDepth > lastStoppedStackDepth) ||
        ((currentStackDepth == lastStoppedStackDepth) && (brkReason == AP_STEP_OUT_BREAKPOINT_HIT)))
        {
        retVal = true;
        }
        }
        }
        */

        /*        // If this is a step in from API debugging or a kernel function breakpoint, don't stop here:
                if ((brkReason == AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT) ||
                    ((brkReason == AP_STEP_IN_BREAKPOINT_HIT) && (_pCurrentKernelDebuggingSession->_debuggedKernelFirstCallback)))
                {
                    retVal = true;
                }
        */

        if (!retVal)
        {
            // If the address stack depth is not at least 1, we are in the middle of nowhere, so continue stepping:
            int currentDepth = _pCurrentKernelDebuggingSession->_pDWARFParser->addressStackDepth(_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter);
            retVal = currentDepth < 1;
        }

        if (!retVal)
        {
            // If we have a work item selected for stepping:
            int* globalWorkSize = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize;
            int* steppingWorkItem = _pCurrentKernelDebuggingSession->_steppingWorkItem;

            if (((globalWorkSize[0] < 0) || (steppingWorkItem[0] > -1)) &&
                ((globalWorkSize[1] < 0) || (steppingWorkItem[1] > -1)) &&
                ((globalWorkSize[2] < 0) || (steppingWorkItem[2] > -1)))
            {
                // Check if we want to step over this line since the selected work item is not valid:
                if (!isWorkItemValid(steppingWorkItem))
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::updateKernelBreakpoints
// Description: Removes all kernel breakpoints, and re-adds only those that are
//              currently existant.
// Author:      Uri Shomroni
// Date:        5/4/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::updateKernelBreakpoints(amdclDebugContext debugContext)
{
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        // If there have been changes to the kernel breakpoints list:
        if (su_stat_theBreakpointsManager.areKernelSourceBreakpointsDirty())
        {
            // Clear all previous breakpoints:
            amdclDebugError retCode = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugClearAllBreakpoints(debugContext);
            GT_ASSERT(retCode == AMDCLDEBUG_SUCCESS);
            _pCurrentKernelDebuggingSession->_debuggedKernelCurrentBreakpoints.clear();
            _pCurrentKernelDebuggingSession->_areStepBreakpointsSet = false;

            // Set up all breakpoints in this kernel:
            const oaCLProgramHandle& containingProgramHandle = _pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramHandle;

            if (containingProgramHandle != OA_CL_NULL_HANDLE)
            {
                // Create an event which will be sent at the end of this:
                apKernelSourceBreakpointsUpdatedEvent breakpointUpdateEvent(osGetCurrentThreadId(), containingProgramHandle);

                // Get the breakpoints:
                gtVector<int> breakpointLineNumbers;
                su_stat_theBreakpointsManager.getBreakpointsInProgram(containingProgramHandle, breakpointLineNumbers);
                osFilePath containingProgramPath(_pCurrentKernelDebuggingSession->_debuggedKernelContainingProgramSourceFilePath);

                // Add the breakpoints:
                int amountOfBreakpoints = (int)breakpointLineNumbers.size();
                gtMap<amdclDebugPC, amdclDebugPC>& boundToRequestedBreakpoints = _pCurrentKernelDebuggingSession->_debuggedKernelBoundToRequestedBreakpoints;
                gtMap<amdclDebugPC, amdclDebugPC>& requestedToBoundBreakpoints = _pCurrentKernelDebuggingSession->_debuggedKernelRequestedToBoundBreakpoints;

                for (int i = 0; i < amountOfBreakpoints; i++)
                {
                    // Translate each into a vector of PCs and set them as breakpoints:
                    gtVector<amdclDebugPC> currentBreakpointAsProgramCounters;
                    int currentLineNumber = breakpointLineNumbers[i];
                    kernelProgramCountersFromLineNumber(debugContext, containingProgramPath, currentLineNumber, currentBreakpointAsProgramCounters);
                    int numberOfPCs = (int)currentBreakpointAsProgramCounters.size();

                    for (int j = 0; j < numberOfPCs; j++)
                    {
                        // Try to bind the breakpoint with some lookahead. Mark the bound PC as such:
                        amdclDebugPC breakpointPC = currentBreakpointAsProgramCounters[j];
                        amdclDebugPC boundPC = breakpointPC;
                        retCode = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetBreakpoint(debugContext, breakpointPC, CS_KERNEL_DEBUGGING_BREAKPOINT_PC_LOOKAHEAD, &boundPC);
                        GT_IF_WITH_ASSERT(retCode == AMDCLDEBUG_SUCCESS)
                        {
                            // Add the binding to the map:
                            boundToRequestedBreakpoints[boundPC] = breakpointPC;
                            requestedToBoundBreakpoints[breakpointPC] = boundPC;

                            // Add the PC to the vector of successful breakpoints:
                            _pCurrentKernelDebuggingSession->_debuggedKernelCurrentBreakpoints.push_back(breakpointPC);

                            // Add the binding to the event:
                            gtString ignored;
                            int boundLineNumber = lineNumberFromKernelProgramCounter(_pCurrentKernelDebuggingSession->_kernelDebugContext, boundPC, ignored);

                            if (boundLineNumber > -1)
                            {
                                breakpointUpdateEvent.addBreakpointBinding(currentLineNumber, boundLineNumber);
                            }
                        }
                    }
                }

                // Send the event:
                bool rcEve = suForwardEventToClient(breakpointUpdateEvent);
                GT_ASSERT(rcEve);
            }

            // Mark that the breakpoints were updated:
            su_stat_theBreakpointsManager.setKernelSourceBreakpointsDirty(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::currentlyDebuggedKernelWorkDimension
// Description: Returns the current work dimension as a number (from the values in the
//              work size vector
// Author:      Uri Shomroni
// Date:        10/4/2011
// ---------------------------------------------------------------------------
int csAMDKernelDebuggingManager::currentlyDebuggedKernelWorkDimension()
{
    int retVal = 0;

    if (_pCurrentKernelDebuggingSession != NULL)
    {
        int* globalWorkSize = _pCurrentKernelDebuggingSession->_debuggedKernelGlobalWorkSize;

        if (globalWorkSize[0] > 0)
        {
            if (globalWorkSize[1] > 0)
            {
                if (globalWorkSize[2] > 0)
                {
                    retVal = 3;
                }
                else
                {
                    retVal = 2;
                }
            }
            else
            {
                retVal = 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::getVariableDetailsInCurrentLocation
// Description: Gets the variable details in the current program counter, if possible.
//              Returns NULL otherwise.
// Author:      Uri Shomroni
// Date:        24/5/2011
// ---------------------------------------------------------------------------
const csDWARFVariable* csAMDKernelDebuggingManager::getVariableDetailsInCurrentLocation(const gtString& variableName)
{
    const csDWARFVariable* retVal = NULL;

    if (_pCurrentKernelDebuggingSession != NULL)
    {
        const csDWARFVariable* pVarDetails = NULL;
        csDWARFParser& theDWARFParser = *_pCurrentKernelDebuggingSession->_pDWARFParser;
        bool rcDW = (theDWARFParser.findClosestScopeContainingVariable((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, variableName, &pVarDetails) != NULL);

        if (!rcDW || (pVarDetails == NULL))
        {
            // If this member has an alias:
            gtString variableNameWithAlias;

            if (matchOpenCLMemberAliases(variableName, variableNameWithAlias))
            {
                // Try with the alias:
                rcDW = (theDWARFParser.findClosestScopeContainingVariable((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter, variableNameWithAlias, &pVarDetails) != NULL);
            }
        }

        // If we found the variable or its aliases:
        if (rcDW && (pVarDetails != NULL))
        {
            retVal = pVarDetails;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::emulateStepCommand
// Description: Emulates step commands by setting breakpoints on all lines we have
//              debug information for.
// Author:      Uri Shomroni
// Date:        20/6/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::emulateStepCommand(amdclDebugCommand& debugCommand, amdclDebugContext debugContext)
{
    GT_IF_WITH_ASSERT(_pCurrentKernelDebuggingSession != NULL)
    {
        if (debugCommand == AMDCLDEBUG_COMMAND_STEP)
        {
            // If we're stepping over or out:
            const apBreakReason& continuedBreakReason = _pCurrentKernelDebuggingSession->_continuedStepBreakReason;

            if ((continuedBreakReason == AP_STEP_OVER_BREAKPOINT_HIT) || (continuedBreakReason == AP_STEP_OUT_BREAKPOINT_HIT))
            {
                // Set the breakpoints from the current PC:
                setBreakpointsFromCurrentPC(debugContext, (continuedBreakReason == AP_STEP_OVER_BREAKPOINT_HIT));
            }
            else // _pCurrentKernelDebuggingSession->_continuedStepBreakReason != AP_STEP_OVER_BREAKPOINT_HIT, AP_STEP_OUT_BREAKPOINT_HIT
            {
                // If change is needed:
                if ((!_pCurrentKernelDebuggingSession->_areStepBreakpointsSet) || (!_pCurrentKernelDebuggingSession->_stepBreakpointsSetSuccessfully))
                {
                    // Set all the breakpoints:
                    setBreakpointsOnAllPCs(debugContext);

                    // Advance the "last program counter" by 1, to make us work properly with 1-line loops:
                    _pCurrentKernelDebuggingSession->_debuggedKernelLastProgramCounter++;
                }
            }

            // Set the real command to "continue":
            debugCommand = AMDCLDEBUG_COMMAND_CONTINUE;
        }
        else if (debugCommand == AMDCLDEBUG_COMMAND_CONTINUE)
        {
            // If change is needed:
            if ((_pCurrentKernelDebuggingSession->_areStepBreakpointsSet) || (!_pCurrentKernelDebuggingSession->_stepBreakpointsSetSuccessfully))
            {
                // Set the real breakpoints only:
                restoreRealBreakpoints(debugContext);
            }
        }
        else
        {
            // Unexpected command!
            GT_ASSERT(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::setBreakpointsOnAllPCs
// Description: Sets a breakpoint on each known PC
// Author:      Uri Shomroni
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::setBreakpointsOnAllPCs(amdclDebugContext debugContext)
{
    // Clear all the breakpoints:
    amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugClearAllBreakpoints(debugContext);
    GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);
    bool breakpointsSuccess = (rc == AMDCLDEBUG_SUCCESS);

    // Add all lines as breakpoints:
    const gtVector<amdclDebugPC>& breakpointPCs = _pCurrentKernelDebuggingSession->_debuggedKernelAllPCsWithLineNumbers;
    gtSize_t numberOfPCs = breakpointPCs.size();

    for (gtSize_t i = 0; i < numberOfPCs; i++)
    {
        // Set the breakpoint:
        rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetBreakpoint(debugContext, breakpointPCs[i], CS_KERNEL_DEBUGGING_BREAKPOINT_PC_LOOKAHEAD, NULL);
        GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);
        breakpointsSuccess = breakpointsSuccess && (rc == AMDCLDEBUG_SUCCESS);
    }

    // Mark our breakpoints are not real:
    _pCurrentKernelDebuggingSession->_areStepBreakpointsSet = true;
    _pCurrentKernelDebuggingSession->_stepBreakpointsSetSuccessfully = breakpointsSuccess;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::setBreakpointsFromCurrentPC
// Description: Sets breakpoints in the context of the current program counter,
//              used to emulate "step over" and "step out" funcitonality.
// Author:      Uri Shomroni
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::setBreakpointsFromCurrentPC(amdclDebugContext debugContext, bool includeCurrentScope)
{
    // Clear all the breakpoints:
    amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugClearAllBreakpoints(debugContext);
    GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);

    // Get the current program counter's scope:
    const csDWARFProgram* pStartingProgram = _pCurrentKernelDebuggingSession->_pDWARFParser->findAddressScope((csDwarfAddressType)_pCurrentKernelDebuggingSession->_debuggedKernelProgramCounter);

    // Start by adding all parent scopes:
    const csDWARFProgram* pCurrentProgram = pStartingProgram;
    GT_IF_WITH_ASSERT(pCurrentProgram != NULL)
    {
        // To step out, ignore the first function's scopes:
        bool isFirstFunction = true;

        // Go up the hierarchy, adding all breakpoints except the CU ones:
        bool goOn = (pCurrentProgram->_pParentProgram != NULL);

        // Iterate up the program hierarchy:
        while (goOn)
        {
            // Add the program counters:
            if (includeCurrentScope || (!isFirstFunction))
            {
                // Add all lines as breakpoints:
                addBreakpointsFromProgram(debugContext, *pCurrentProgram);
            }

            // If the current scope is a function call, we are exiting the first function:
            if (pCurrentProgram->_programScopeType == csDWARFProgram::CS_INLINE_FUNCTION_SCOPE)
            {
                isFirstFunction = false;
            }

            // Continue up the tree:
            pCurrentProgram = pCurrentProgram->_pParentProgram;

            // Stop if we got to the CU program:
            goOn = (pCurrentProgram->_pParentProgram != NULL);
        }
    }

    // Add all real breakpoints, so we won't skip them if they're inside the step over / out:
    const gtVector<amdclDebugPC>& breakpointPCs = _pCurrentKernelDebuggingSession->_debuggedKernelCurrentBreakpoints;
    gtSize_t numberOfPCs = breakpointPCs.size();

    for (gtSize_t i = 0; i < numberOfPCs; i++)
    {
        // Set the breakpoint:
        rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetBreakpoint(debugContext, breakpointPCs[i], CS_KERNEL_DEBUGGING_BREAKPOINT_PC_LOOKAHEAD, NULL);
        GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);
    }

    // Since the program counter will move for the next step, we set the "set successfully"
    // value to false to mark them as stale:
    _pCurrentKernelDebuggingSession->_areStepBreakpointsSet = true;
    _pCurrentKernelDebuggingSession->_stepBreakpointsSetSuccessfully = false;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::addBreakpointsFromProgram
// Description: Adds all the breakpoints mapped inside a program's scope and all
//              its contained (non-function) subscopes.
// Author:      Uri Shomroni
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::addBreakpointsFromProgram(amdclDebugContext debugContext, const csDWARFProgram& program)
{
    // Get the breakpoints:
    const gtVector<csDwarfAddressType>& breakpointPCs = program._programMappedPCs;
    gtSize_t numberOfPCs = breakpointPCs.size();

    for (gtSize_t i = 0; i < numberOfPCs; i++)
    {
        // Set the breakpoint:
        amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetBreakpoint(debugContext, (amdclDebugPC)breakpointPCs[i], CS_KERNEL_DEBUGGING_BREAKPOINT_PC_LOOKAHEAD, NULL);
        GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);
    }

    // Recursively add all the sub scopes that aren't function calls:
    const gtPtrVector<csDWARFProgram*>& childPrograms = program._childPrograms;
    gtSize_t numberOfSubPrograms = childPrograms.size();

    for (gtSize_t i = 0; i < numberOfSubPrograms; i++)
    {
        // Get the child:
        const csDWARFProgram* pCurrentChild = childPrograms[i];

        if (pCurrentChild != NULL)
        {
            // If it is not a function call:
            if (pCurrentChild->_programScopeType != csDWARFProgram::CS_INLINE_FUNCTION_SCOPE)
            {
                // Call this function to add its breakpoints as well:
                addBreakpointsFromProgram(debugContext, *pCurrentChild);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::restoreRealBreakpoints
// Description: Restores the real breakpoint
// Author:      Uri Shomroni
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void csAMDKernelDebuggingManager::restoreRealBreakpoints(amdclDebugContext debugContext)
{
    // Clear all the breakpoints:
    amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugClearAllBreakpoints(debugContext);
    GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);
    bool breakpointsSuccess = (rc == AMDCLDEBUG_SUCCESS);

    // Add all real breakpoints:
    const gtVector<amdclDebugPC>& breakpointPCs = _pCurrentKernelDebuggingSession->_debuggedKernelCurrentBreakpoints;
    gtSize_t numberOfPCs = breakpointPCs.size();

    for (gtSize_t i = 0; i < numberOfPCs; i++)
    {
        // Set the breakpoint:
        rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugSetBreakpoint(debugContext, breakpointPCs[i], CS_KERNEL_DEBUGGING_BREAKPOINT_PC_LOOKAHEAD, NULL);
        GT_ASSERT(rc == AMDCLDEBUG_SUCCESS);
        breakpointsSuccess = breakpointsSuccess && (rc == AMDCLDEBUG_SUCCESS);
    }

    // Mark our breakpoints are real:
    _pCurrentKernelDebuggingSession->_areStepBreakpointsSet = false;
    _pCurrentKernelDebuggingSession->_stepBreakpointsSetSuccessfully = breakpointsSuccess;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::backendFailureErrorInformation
// Description: Get the failure error information as string
// Arguments:   amdclDebugContext debugContext
//              gtString& failureString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        21/8/2012
// ---------------------------------------------------------------------------
bool csAMDKernelDebuggingManager::debugLastErrorString(amdclDebugContext debugContext, gtString& failureString)
{
    bool retVal = false;

    failureString.makeEmpty();

    // Code for finding buffer size to be used when amdopencl fix the code
    // Find the size of the buffer needed:
    size_t bufferSize = 0;
    amdclDebugError rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugUtilGetLastError(debugContext, 0, NULL, &bufferSize);

    if (AMDCLDEBUG_SUCCESS == rc && 0 < bufferSize)
    {
        char* infoBuffer = new char[bufferSize + 1];
        rc = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugUtilGetLastError(debugContext, bufferSize, infoBuffer, NULL);

        if (AMDCLDEBUG_SUCCESS == rc)
        {
            infoBuffer[bufferSize] = (char)0;
            failureString.fromASCIIString(infoBuffer);
            retVal = true;
        }

        delete [] infoBuffer;
    }

    return retVal;
}
