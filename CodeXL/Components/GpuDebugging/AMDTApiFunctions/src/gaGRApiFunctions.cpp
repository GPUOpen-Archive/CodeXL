//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaGRApiFunctions.cpp
///
//==================================================================================

//------------------------------ gaGRApiFunctions.cpp ------------------------------

// POSIX:
#include <limits.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/Events/apBeforeDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelWorkItemChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apCounterType.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apGLDisplayList.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apGPUInfo.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTAPIClasses/Include/apRasterMode.h>
#include <AMDTAPIClasses/Include/apRenderPrimitivesStatistics.h>
#include <AMDTAPIClasses/Include/apSearchDirection.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>
// #include <AMDTPerformanceCounters/Include/pcPerformanceCountersManager.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>
#include <AMDTProcessDebugger/Include/pdProcessDebuggersManager.h>

// Local:
#include <src/gaAPIToSpyConnector.h>
#include <src/gaPersistentDataManager.h>
#include <src/gaPrivateAPIFunctions.h>
#include <src/gaSingletonsDelete.h>
#include <src/gaStringConstants.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// A static instance of the singleton deleter class. Its destructor will delete all
// this GRApiFunctions library singletons.
// Putting this static member in gaSingletonsDelete.cpp does not work (I don't know why)
// So - I put it here. (Yaki 21/4/04)
static gaSingletonsDelete gaSingletonsDeleteInstance;

// A static reference to the persistent data manager:
gaPersistentDataManager& stat_thePersistentDataMgr = gaPersistentDataManager::instance();

// A static reference to the performance counters manager:
// pcPerformanceCountersManager& stat_thePerformanceCountersMgr = pcPerformanceCountersManager::instance();

#define GA_TEXTURES_UPDATE_CHUNK_SIZE 100


// Static members initializations:
gaGRApiFunctions* gaGRApiFunctions::_pMySingleInstance = NULL;

// Aid macro for connecting the global API functions to the gaGRApiFunctions struct:
#define GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(funcName, retType, argsList, argsListNoTypes)\
    retType funcName argsList\
    {\
        retType retVal = gaGRApiFunctions::instance().funcName argsListNoTypes;\
        \
        return retVal;\
    }

#define GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS_VOID_RETVAL(funcName, argsList, argsListNoTypes)\
    void funcName argsList\
    {\
        gaGRApiFunctions::instance().funcName argsListNoTypes;\
    }


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGRApiFunctions
// Description: Constructor
// Author:      Uri Shomroni
// Date:        27/9/2010
// ---------------------------------------------------------------------------
gaGRApiFunctions::gaGRApiFunctions()
{
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::~gaGRApiFunctions
// Description: Destructor
// Author:      Uri Shomroni
// Date:        27/9/2010
// ---------------------------------------------------------------------------
gaGRApiFunctions::~gaGRApiFunctions()
{
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function, if another was not registered
//              beforehand.
// Author:      Uri Shomroni
// Date:        27/9/2010
// ---------------------------------------------------------------------------
gaGRApiFunctions& gaGRApiFunctions::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gaGRApiFunctions;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::registerInstance
// Description: Registers an instance of this class or one of its subclasses.
//              Note this method SHOULD ONLY BE CALLED ONCE, and cannot be called
//              once the instance method was called.
// Author:      Uri Shomroni
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gaGRApiFunctions::registerInstance(gaGRApiFunctions& myInstance)
{
    // Make sure we don't have a previous instance:
    if (_pMySingleInstance != NULL)
    {
        GT_ASSERT_EX((_pMySingleInstance == NULL), GA_STR_gaGRApiFunctionsRegisterdWithNonNullInstance);

        // Avoid memory leaks, anyway:
        delete _pMySingleInstance;
        _pMySingleInstance = NULL;
    }

    // Register the new instance:
    _pMySingleInstance = &myInstance;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIntializeAPIPackage
// Description: Initialize the API package.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIntializeAPIPackage(bool shouldInitPerfCounters)
{
    bool retVal = true;

    OS_OUTPUT_DEBUG_LOG(GA_STR_InitAPIDll, OS_DEBUG_LOG_DEBUG);

    // Initialize the APIClasses dll:
    retVal = apiClassesInitFunc();

    GT_IF_WITH_ASSERT_EX(retVal, GA_STR_FailedToInitAPIDll)
    {
        // Create the Spy connector instance:
        // Unused variable - comment it out
        // gaAPIToSpyConnector& theSpyConnector = gaAPIToSpyConnector::instance();

        OS_OUTPUT_DEBUG_LOG(GA_STR_CreatedAPIToSpyConnector, OS_DEBUG_LOG_DEBUG);

        // Initialize the performance counters manager if requested:
        if (shouldInitPerfCounters)
        {
            bool rc = false; //stat_thePerformanceCountersMgr.initialize();

            // We do not fail the API package initialization, since its the performance
            // counters manager will not manage to initialize on all platforms. But, we
            // would like to get an assertion notification:
            GT_ASSERT(rc);
        }

        OS_OUTPUT_DEBUG_LOG(GA_STR_FinishedInitAPIDll, OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsAPIConnectionActive
// Description:
//   Inputs an API connection type and returns true if such API connection was
//   established with the debugged process.
// Author:      Yaki Tebeka
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsAPIConnectionActive(apAPIConnectionType apiType)
{
    bool retVal = false;

    // In certain Mac applications (e.g. java-based applications, Safari), a SIGTRAP
    // is thrown during loading (by dyld). If this happens between the API initialization
    // and the API thread run starting, CodeXL will get here and wait for a reply,
    // but no API loop is running to answer the query. So we check if the API thread
    // exists already, or not.
    if (pdProcessDebugger::instance().isSpiesAPIThreadRunning())
    {
        // Get the Spy connector instance:
        gaAPIToSpyConnector& theSpyConnector = gaAPIToSpyConnector::instance();
        retVal = theSpyConnector.isAPIConnectionActive(apiType);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsAPIConnectionActiveAndDebuggedProcessSuspended
// Description:
//   Inputs an API connection type and returns true if such API connection was
//   established with the debugged process and the debugged process is in suspended mode.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apAPIConnectionType apiType)
{
    bool retVal = false;

    // If the debugged process is suspended:
    if (gaIsDebuggedProcessSuspended())
    {
        // If the the connection to the OpenGL32.dll spy is active:
        if (gaIsAPIConnectionActive(apiType))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaDebuggedExecutableExists
// Description: Checks if the debugged executable exists.
// Arguments: processCreationData - A process creation data, containing the debugged
//                                  executable path, execution target, etc
// Return Val: bool - true - iff the debugged executable exists.
// Author:      Yaki Tebeka
// Date:        3/8/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaDebuggedExecutableExists(const apDebugProjectSettings& processCreationData)
{
    bool retVal = true;

    // When running on a remote machine or the iPhone device, we assume that the executable exists:
    if (!processCreationData.isRemoteTarget())
    {
        const osFilePath& exePath = processCreationData.executablePath();

        // Verify that the debugged process file exists on disk:
        retVal = exePath.isExecutable();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSystemLibrariesExists
// Description: Checks if the system libraries, required by the debugged project type exists.
// Arguments: processCreationData - A process creation data, containing the debugged project type.
//            missingSystemLibraries - An output string, containing a list of missing system libraries, saperated by newlines.
// Return Val: bool  - true - the required system libraries exist.
//                     false - at least one system library is missing.
// Author:      Yaki Tebeka
// Date:        3/8/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSystemLibrariesExists(const apDebugProjectSettings& processCreationData, gtString& missingSystemLibraries)
{
    bool retVal = true;
    missingSystemLibraries.makeEmpty();

    // Ignore missing system libraries for remote targets:
    if (!processCreationData.isRemoteTarget())
    {
        // Get the system's OpenGL/ES module path:
        gtVector<osFilePath> systemOGLModulePath;
        osGetSystemOpenGLModulePath(systemOGLModulePath);

        // Check the existence of OpenGL libraries:
        bool oglModuleExists = false;
        int numberOfGLPaths = (int)systemOGLModulePath.size();

        for (int i = 0; (i < numberOfGLPaths) && (!oglModuleExists); i++)
        {
            oglModuleExists = systemOGLModulePath[i].exists();
        }

        if (!oglModuleExists)
        {
            retVal = false;

            if (numberOfGLPaths > 0)
            {
                gtString fileName;
                systemOGLModulePath[0].getFileNameAndExtension(fileName);
                missingSystemLibraries += fileName;
            }
            else
            {
                missingSystemLibraries += OS_OPENGL_MODULE_NAME;
            }
        }

        // Get the system's OpenCL module path:
        gtVector<osFilePath> systemOCLModulePaths;
        bool rcCLModule = osGetSystemOpenCLModulePath(systemOCLModulePaths);
        GT_IF_WITH_ASSERT(rcCLModule)
        {
            // Check the existence of OpenCL libraries:
            bool oclModuleExists = false;
            int numberOfCLPaths = (int)systemOCLModulePaths.size();

            for (int i = 0; (i < numberOfCLPaths) && (!oclModuleExists); i++)
            {
                oclModuleExists = systemOCLModulePaths[i].exists();
            }

            if (!oclModuleExists)
            {
                retVal = false;

                if (!missingSystemLibraries.isEmpty())
                {
                    missingSystemLibraries += L"\n";
                }

                if (numberOfCLPaths > 0)
                {
                    gtString fileName;
                    systemOCLModulePaths[0].getFileNameAndExtension(fileName);
                    missingSystemLibraries += fileName;
                }
                else
                {
                    missingSystemLibraries += OS_OPENCL_ICD_MODULE_NAME;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaLaunchDebuggedProcess
// Description: Launch the debugged process.
// Arguments:   processCreationData - The debugged process creation data.
//              launchSuspended - true iff we are using two-step process creation.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaLaunchDebuggedProcess(const apDebugProjectSettings& processCreationData, bool launchSuspended)
{
    bool retVal = false;

    // Update the process creation data:
    // Set environment variables that will be read by the OpenGL Spy:
    // -------------------------------------------------------------
    apDebugProjectSettings processCreationDataWithAPIEnvVariables(processCreationData);

    gtString overrideVarVal;
    bool rcOvr = processCreationDataWithAPIEnvVariables.getEnvironmentVariable(OS_STR_envVar_debuggedAppNameOverride, overrideVarVal);

    if ((!rcOvr) || (L"0" == overrideVarVal))
    {
        // Create an environment variable that will contain the debugged application name:
        osEnvironmentVariable debuggedAppNameEnvVariable;
        debuggedAppNameEnvVariable._name = OS_STR_envVar_debuggedAppName;

        gtString debuggedAppName(L"Unknown");
        processCreationData.executablePath().getFileName(debuggedAppName);
        debuggedAppNameEnvVariable._value = debuggedAppName;
        processCreationDataWithAPIEnvVariables.addEnvironmentVariable(debuggedAppNameEnvVariable);
    }

    // Create an environment variable that will contain the debug log severity:
    osDebugLogSeverity debugLogSeverity = osDebugLog::instance().loggedSeverity();
    osEnvironmentVariable debugLogSeverityEnvVariable;
    debugLogSeverityEnvVariable._name = OS_STR_envVar_debugLogSeverity;
    debugLogSeverityEnvVariable._value = osDebugLogSeverityToString(debugLogSeverity);
    processCreationDataWithAPIEnvVariables.addEnvironmentVariable(debugLogSeverityEnvVariable);

    // Create an environment variable that will contain the spies directory:
    osEnvironmentVariable spiesDirEnvVariable;
    spiesDirEnvVariable._name = OS_STR_envVar_spiesDirectory;
    spiesDirEnvVariable._value = processCreationData.spiesDirectory().asString();
    processCreationDataWithAPIEnvVariables.addEnvironmentVariable(spiesDirEnvVariable);

    // Create an environment variable that will tell the spy whether to monitor performance counters:
    osEnvironmentVariable perfCountersEnvVariable;
    bool shouldInitializePerformanceCounters = processCreationData.shouldInitializePerformanceCounters();
    perfCountersEnvVariable._name = OS_STR_envVar_initializePerformanceCounters;
    perfCountersEnvVariable._value = shouldInitializePerformanceCounters ? OS_STR_envVar_valueTrue : OS_STR_envVar_valueFalse;
    processCreationDataWithAPIEnvVariables.addEnvironmentVariable(perfCountersEnvVariable);

    // Decide which API connection method to use:
    osPortAddress spyAPIPort;
    osPortAddress spyEventsPort;
    gtString spyAPIPipeName;
    gtString incomingEventsPipeName;

    if (processCreationDataWithAPIEnvVariables.isRemoteTarget())
    {
        // For a remote target, we need to create TCP servers:
        spyAPIPort.setAsLocalPortAddress(processCreationDataWithAPIEnvVariables.remoteConnectionAPIPort(), false);
        spyEventsPort.setAsLocalPortAddress(processCreationDataWithAPIEnvVariables.remoteConnectionSpiesEventsPort(), false);

        // Set the API connection environment variables:
        osEnvironmentVariable spiesAPIConnectionIPHostnameEnvVariable;
        spiesAPIConnectionIPHostnameEnvVariable._name = OS_STR_envVar_serverConnectionIPHostname;
        spiesAPIConnectionIPHostnameEnvVariable._value = spyAPIPort.hostName();
        processCreationDataWithAPIEnvVariables.addEnvironmentVariable(spiesAPIConnectionIPHostnameEnvVariable);
        osEnvironmentVariable spiesAPIConnectionIPPortEnvVariable;
        spiesAPIConnectionIPPortEnvVariable._name = OS_STR_envVar_serverConnectionIPPort;
        spiesAPIConnectionIPPortEnvVariable._value.appendFormattedString(L"%u", (unsigned int)spyAPIPort.portNumber());
        processCreationDataWithAPIEnvVariables.addEnvironmentVariable(spiesAPIConnectionIPPortEnvVariable);
    }
    else
    {
        // Generate a unique pipe name:
        gtString apiConnectionSharedMemObjectName;
        gaGenerateUniquePipeName(apiConnectionSharedMemObjectName);

        // Calculate the Spy API pipe name:
        spyAPIPipeName = apiConnectionSharedMemObjectName;
        spyAPIPipeName += GA_SHARED_MEM_OBJ_API_SUFFIX;

        // Calculate the incoming events pipe name:
        incomingEventsPipeName = apiConnectionSharedMemObjectName;
        incomingEventsPipeName += GA_SHARED_MEM_OBJ_INCOMING_EVENTS_SUFFIX;
        processCreationDataWithAPIEnvVariables.setSpiesEventsPipeName(incomingEventsPipeName);

        // Create an environment variable that will contain the spies API connection pipe name:
        osEnvironmentVariable spiesAPISharedMemObjNameEnvVariable;
        spiesAPISharedMemObjNameEnvVariable._name = OS_STR_envVar_APIPipeName;
        spiesAPISharedMemObjNameEnvVariable._value = spyAPIPipeName;
        processCreationDataWithAPIEnvVariables.addEnvironmentVariable(spiesAPISharedMemObjNameEnvVariable);
    }

    //////////////////////////////////////////////////////////////////////////
    // Process Launch:
    //////////////////////////////////////////////////////////////////////////
    // Make sure the process debugger suits the process creation data:
    pdProcessDebuggersManager::instance().adjustProcessDebuggerToProcessCreationData(processCreationDataWithAPIEnvVariables);

    // Initialize it:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
    bool rcPDInit = theProcessDebugger.initializeDebugger(processCreationDataWithAPIEnvVariables);
    GT_ASSERT(rcPDInit);

    // Apply the persistent values:
    bool rcPersInit = stat_thePersistentDataMgr.setProcessDebuggerPersistentDataValues();
    GT_ASSERT(rcPersInit);

    // If we are attempting two-step launching, make sure the process debugger supports it:
    GT_IF_WITH_ASSERT(rcPDInit && (!launchSuspended || theProcessDebugger.doesSupportTwoStepLaunching()))
    {
        // Verify that the debugged executable exists:
        bool exeFileExists = gaDebuggedExecutableExists(processCreationDataWithAPIEnvVariables);
        GT_IF_WITH_ASSERT(exeFileExists)
        {
            // Verify that the system libraries exists:
            gtString ignored;
            bool testForMissingSystemLibraries = false;
            bool sysLibrariesExists = (!testForMissingSystemLibraries) || gaSystemLibrariesExists(processCreationDataWithAPIEnvVariables, ignored);
            GT_IF_WITH_ASSERT(sysLibrariesExists)
            {
                // Store the debugged exe path:
                const osFilePath& exePath = processCreationDataWithAPIEnvVariables.executablePath();
                stat_thePersistentDataMgr.setDebuggedExecutablePath(exePath);

                // Create and store an API init data object:
                apApiFunctionsInitializationData apiInitData(processCreationDataWithAPIEnvVariables);
                apiInitData.setLogFilesDirectoryPath(processCreationDataWithAPIEnvVariables.logFilesFolder().directoryPath());
                stat_thePersistentDataMgr.setAPIInitializationData(apiInitData);

                bool rcAPIConnection = false;

                if (processCreationDataWithAPIEnvVariables.isRemoteTarget())
                {
                    // Initialize a TCP/IP connection:
                    rcAPIConnection = gaInitializeAPIConnection(spyAPIPort, spyEventsPort);
                }
                else // !processCreationDataWithAPIEnvVariables.isRemoteTarget()
                {
                    // Initialize a pipe API connection:
                    rcAPIConnection = gaInitializeAPIConnection(spyAPIPipeName, incomingEventsPipeName);
                }

                GT_IF_WITH_ASSERT(rcAPIConnection)
                {
                    if (launchSuspended)
                    {
                        // Launch the debugged process in suspended mode:
                        retVal = theProcessDebugger.launchDebuggedProcessInSuspendedMode();
                    }
                    else // !launchSuspended
                    {
                        // Launch the debugged process and run it:
                        retVal = theProcessDebugger.launchDebuggedProcess();
                    }
                }

                if (retVal)
                {
                    // The API initialization data may have changed on the process debugger side, update it now:
                    const apDebugProjectSettings* pNewSettings = theProcessDebugger.serverSideDebuggedProcessCreationData();

                    if (NULL != pNewSettings)
                    {
                        apApiFunctionsInitializationData apiInitData2(*pNewSettings);
                        apiInitData2.setLogFilesDirectoryPath(pNewSettings->logFilesFolder().directoryPath());
                        stat_thePersistentDataMgr.setAPIInitializationData(apiInitData2);
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaContinueDebuggedProcessFromSuspendedCreation
// Description: Continues the debugged process run after creating it with
//              gaGRApiFunctions::gaLaunchDebuggedProcess( ..., true)
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaContinueDebuggedProcessFromSuspendedCreation()
{
    bool retVal = false;

    // Sanity check:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
    GT_IF_WITH_ASSERT(theProcessDebugger.doesSupportTwoStepLaunching())
    {
        // Continue the debugged process:
        retVal = theProcessDebugger.continueDebuggedProcessFromSuspendedCreation();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaDebuggedProcessExists
// Description: Returns true iff a debugged process was launched succesfully
//              and didn't terminate yet.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaDebuggedProcessExists()
{
    bool rc = true;

    // Get the process debugger:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

    // Check if there is a debugged process running:
    rc = theProcessDebugger.debuggedProcessExists();

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaTerminateDebuggedProcess
// Description: Terminates the debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaTerminateDebuggedProcess()
{
    bool retVal = false;
    bool isStdTerminationSuccess = false;
    bool isForcedTerminationSuccess = false;

    // If the debugged process exists:
    if (gaDebuggedProcessExists())
    {
        OS_OUTPUT_DEBUG_LOG(L"Debugged process exists.", OS_DEBUG_LOG_DEBUG);

        // Flush out all events, to know if the process was created, if the connections are active, etc:
        apEventsHandler::instance().handlePendingDebugEvent();

        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
        theProcessDebugger.prepareProcessToTerminate();
        bool bSpyConnection = false;

        // Verify that the API connection is active:
        if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
        {
            bSpyConnection = true;

            // Tell the persistent data manager we are about to kill the process manually:
            stat_thePersistentDataMgr.setTerminatingDebuggedProcessThroughAPI();

            // Mark all API connection types as inactive:
            gaAPIToSpyConnector& theSpyConnector = gaAPIToSpyConnector::instance();
            theSpyConnector.beforeForcedDebuggedProcessTermination();

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaTerminateDebuggedProcess;

            spyConnectionSocket.setReadOperationTimeOut(1000);

            // Receive success value:
            spyConnectionSocket >> isStdTerminationSuccess;
            GT_ASSERT(isStdTerminationSuccess);

            if (spyConnectionSocket.isOpen())
            {
                // Close the spy connecting socket:
                spyConnectionSocket.close();
            }
        }

        OS_OUTPUT_DEBUG_LOG((!isStdTerminationSuccess) ? L"Standard termination failed." :
                            L"Standard termination succeeded.", OS_DEBUG_LOG_DEBUG);

        bool wasProcessTerminated = !gaDebuggedProcessExists();
        int waitCount = 0;

        while ((!wasProcessTerminated) && (10 > waitCount++) && (bSpyConnection))
        {
            osSleep(100);
            wasProcessTerminated = !gaDebuggedProcessExists();
        }

        // If we failed, force termination via the process debugger:
        if (!wasProcessTerminated)
        {
            // Amit: Just do it anyway (fix for BUG407390).
            // Get the process debugger.
            //            pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

            // Terminate the debugged process.
            isForcedTerminationSuccess = theProcessDebugger.terminateDebuggedProcess();

            OS_OUTPUT_DEBUG_LOG((!isForcedTerminationSuccess) ? L"Forced termination failed." :
                                L"Forced termination succeeded.", OS_DEBUG_LOG_DEBUG);
        }

        // At least one successful termination counts as an overall success.
        retVal = (isStdTerminationSuccess || isForcedTerminationSuccess);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSuspendDebuggedProcess
// Description: Suspends the run of the debugged process.
//              The suspension will occur when the next OpenGL function call
//              will be performed.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSuspendDebuggedProcess()
{
    bool retVal = false;

    // Get the process debugger:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

    // Verify that a debugged process exists:
    if (theProcessDebugger.debuggedProcessExists())
    {
        // If the debugged process is already suspended:
        if (theProcessDebugger.isDebuggedProcssSuspended())
        {
            // Nothing to be done:
            retVal = true;
        }
        else
        {
            // Make the debugged process break:
            retVal = stat_thePersistentDataMgr.suspendDebuggedProcessExecution();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaResumeDebuggedProcess
// Description: Resumes the run of a suspended debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaResumeDebuggedProcess()
{
    bool retVal = true;

    // Get the process debugger:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

    // Verify that a debugged process exists:
    if (theProcessDebugger.debuggedProcessExists())
    {
        // If the debugged process is suspended:
        if (theProcessDebugger.isDebuggedProcssSuspended())
        {
            if (theProcessDebugger.isAtAPIOrKernelBreakpoint(OS_NO_THREAD_ID))
            {
                // If the API is active (the API may not be active when handling debugged process 2nd chance exceptions):
                if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
                {
                    // Notify observers that the debugged process run is going to be resumed:
                    apBeforeDebuggedProcessRunResumedEvent beforeProcessResumedEvent;
                    apEventsHandler::instance().handleDebugEvent(beforeProcessResumedEvent);

                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // Send the function Id:
                    spyConnectionSocket << (gtInt32)GA_FID_gaResumeDebuggedProcess;
                }
            }

            // Resume the debugged process run:
            retVal = theProcessDebugger.resumeDebuggedProcess();
        }
        else
        {
            // Nothing to be done:
            retVal = true;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////
/// \brief Lock driver threads in host process
///
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 15/05/2016
bool gaGRApiFunctions::gaLockDriverThreads()
{
    OS_OUTPUT_DEBUG_LOG(L"Try lock threads", OS_DEBUG_LOG_DEBUG);

    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        osSocket& spyConnectionSocket = gaSpiesAPISocket();
        spyConnectionSocket << (gtInt32)GA_FID_gaLockDriverThreads;

        bool retVal;

        spyConnectionSocket >> retVal;
    }

    return true;
}

/////////////////////////////////////////////////////
/// \brief Unlock driver threads in host process
///
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 15/05/2016
bool gaGRApiFunctions::gaUnLockDriverThreads()
{
    OS_OUTPUT_DEBUG_LOG(L"Try unlock threads", OS_DEBUG_LOG_DEBUG);

    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        osSocket& spyConnectionSocket = gaSpiesAPISocket();
        spyConnectionSocket << (gtInt32)GA_FID_gaUnlockDriverThreads;

        bool retVal;

        spyConnectionSocket >> retVal;
    }

    return true;
}


/////////////////////////////////////////////////////
/// \brief Suspend threads
///
/// \param thrds a vector of threads native handles
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 12/17/2015
bool gaGRApiFunctions::gaSuspendThreads(const std::vector<osThreadId>& thrds)
{
    bool retVal = true;

    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSuspendThreads;
    spyConnectionSocket << thrds.size();

    for (auto const& it : thrds)
    {
        spyConnectionSocket << it;
    }

    spyConnectionSocket >> retVal;

    return retVal;
}

//////////////////////////////////////////////////////////////////
/// \brief Resume threads. All previously suspended threads
///   stored into the internal structure
///
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 12/17/2015
bool gaGRApiFunctions::gaResumeThreads()
{
    bool retVal = true;

    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaResumeThreads;

    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsDebuggedProcessSuspended
// Description:
//    Returns true iff the below two conditions are met:
//    a. The debugged process exists.
//    b. Its run is suspended.
//
//    A  debugged process can be suspended by a call to gaGRApiFunctions::gaSuspendDebuggedProcess /
//    by hitting a breakpoint / etc.
//
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsDebuggedProcessSuspended()
{
    bool retVal = false;

    // Get the process debugger:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

    // Verify that a debugged process exists:
    if (theProcessDebugger.debuggedProcessExists())
    {
        // Check if the debugged process is suspended:
        retVal = theProcessDebugger.isDebuggedProcssSuspended();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsDuringDebuggedProcessTermination
// Description: Is this the break during the debugged process termination?
// Author:      Uri Shomroni
// Date:        11/11/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsDuringDebuggedProcessTermination()
{
    bool retVal = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsDebugging64BitApplication
// Description: Are we debugging a 64-bit application or a 32-bit one?
// Author:      Uri Shomroni
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsDebugging64BitApplication()
{
    bool retVal = false;

    bool rc64Bit = pdProcessDebugger::instance().isDebugging64BitApplication(retVal);
    GT_ASSERT(rc64Bit);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCurrentDebugSessionLogFilesSubDirectory
// Description: Gets the log files directory for the current debug session.
//              This is the dir path of the form
//              <user-chosen log files dir>/<App name>-<Date>-<Time>/
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        10/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCurrentDebugSessionLogFilesSubDirectory(osFilePath& logFilesPath)
{
    bool retVal = true;

    logFilesPath = stat_thePersistentDataMgr.currentDebugSessionLogFilesSubDirectory();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfDebuggedProcessThreads
// Description: Returns the amount of the debugged process threads.
// Arguments:   threadsAmount - Will get the amount of debugged process threads.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/5/2005
// Implementation notes:
//   We hide the existence of the Spy API thread.
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfDebuggedProcessThreads(int& threadsAmount)
{
    bool retVal = false;
    threadsAmount = 0;

    // Verify that the debugged process is suspended:
    if (gaIsDebuggedProcessSuspended())
    {
        // Get the amount of debugged process threads:
        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
        threadsAmount = theProcessDebugger.amountOfDebuggedProcessThreads();

        // Does the spies API threads exist:
        bool spiesAPIThreadExists = theProcessDebugger.isSpiesAPIThreadRunning();

        // If we NOT are during debugged process termination:
        bool isDuringDebuggedProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        if (!isDuringDebuggedProcessTermination)
        {
            // Hide the spy API thread ids:

            if (spiesAPIThreadExists)
            {
                threadsAmount--;
            }
        }

        retVal = true;
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetThreadId
// Description: Inputs a thread index (in the debugged process threads list)
//              and returns its id.
// Arguments:   threadIndex - The thread index (in the debugged process threads list).
//              threadId - Will get the tread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/5/2005
// Implementation notes:
//   We hide the existence of the Spy API thread.
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetThreadId(int threadIndex, osThreadId& threadId)
{
    bool retVal = false;
    threadId = OS_NO_THREAD_ID;

    // Verify that the debugged process is suspended:
    if (gaIsDebuggedProcessSuspended())
    {
        // Get the spy APIs thread index:
        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
        int spiesAPIThreadIndex = theProcessDebugger.spiesAPIThreadIndex();

        // Check if we are during debugged process termination:
        bool isDuringDebuggedProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        // If the spy API thread is not known yet or we are during debugged process termination:
        if ((spiesAPIThreadIndex == -1) || isDuringDebuggedProcessTermination)
        {
            spiesAPIThreadIndex = INT_MAX;
        }

        // Get the input thread id, ignore (jump over) spy API threads:
        int amountOfThreadsToJumpOver = 0;

        if (spiesAPIThreadIndex <= threadIndex)
        {
            amountOfThreadsToJumpOver++;
        }

        retVal = theProcessDebugger.getThreadId(threadIndex + amountOfThreadsToJumpOver, threadId);
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////
/// \brief Get host breakpoint triggering thread index
///
/// \param threadIndex a index of requested thread
/// \return true if success or false
///
/// \author Vadim Entov
/// \date 21/02/2016
bool gaGRApiFunctions::gaGetBreakpointTriggeringThreadIndex(int& threadIndex)
{
    bool retVal = false;

    if (gaIsDebuggedProcessSuspended())
    {
        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
        retVal = theProcessDebugger.getBreakpointTriggeringThreadIndex(threadIndex);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetThreadCurrentRenderContext
// Description: Inputs a thread id and returns the thread's current render context id
// Arguments:   threadId - The input thread id.
//              contextId - Will get the thread's current render context id,
//                          or 0 if the thread does not have a current render context.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/5/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetThreadCurrentRenderContext(const osThreadId& threadId, int& contextId)
{
    bool retVal = false;
    contextId = 0;

    if (OS_NO_THREAD_ID != threadId)
    {
        // Attempt to get the correlation from our cache:
        gaPersistentDataManager& thePersistentDataMgr = gaPersistentDataManager::instance();
        retVal = thePersistentDataMgr.getThreadCurrentOpenGLContextFromCache(threadId, contextId);

        if (!retVal)
        {
            // Verify that the debugged process is suspended and the API is active:
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the function Id:
                spyConnectionSocket << (gtInt32)GA_FID_gaGetThreadCurrentRenderContext;

                // Send arguments:
                spyConnectionSocket << (gtUInt64)threadId;

                // Perform after API call actions:
                pdProcessDebugger::instance().afterAPICallIssued();

                // Receive success value:
                spyConnectionSocket >> retVal;

                GT_IF_WITH_ASSERT(retVal)
                {
                    // Receive parameters:
                    gtInt32 contextIdAsInt32 = -1;
                    spyConnectionSocket >> contextIdAsInt32;
                    contextId = (int)contextIdAsInt32;
                }
            }

            // If we succeeded, add this value to the cache:
            if (retVal)
            {
                thePersistentDataMgr.cacheThreadToOpenGLContextCorrelation(threadId, contextId);
            }
        }
    }
    else // (OS_NO_THREAD_ID != threadId)
    {
        // Always return context 0 for thread 0:
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetBreakpointTriggeringContextId
// Description: Return the triggering context id. The function try to get an answer
//              from the OpenGL spy, and if it can't, it talks with the OpenCL spy
// Arguments: apContextID& contextId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetBreakpointTriggeringContextId(apContextID& contextId)
{
    bool retVal = false;

    // Verify that the debugged process is suspended and the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetBreakpointTriggeringContextId;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        spyConnectionSocket >> retVal;

        GT_IF_WITH_ASSERT(retVal)
        {
            contextId.readSelfFromChannel(spyConnectionSocket);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderContextCurrentThread
// Description: Inputs a render context id and returns the thread to which
//              it serves as the current context.
// Arguments:   contextId - The input context id.
//              threadId - The output thread id (or OS_NO_THREAD_ID if there is
//                         no thread that has this context as its currnet context)
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/5/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderContextCurrentThread(int contextId, osThreadId& threadId)
{
    bool retVal = false;

    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isInKernelDebugging)
    {
        threadId = OS_NO_THREAD_ID;

        // If the input context is a real context:
        if (contextId > 0)
        {
            // Attempt to get the correlation from our cache:
            gaPersistentDataManager& thePersistentDataMgr = gaPersistentDataManager::instance();
            retVal = thePersistentDataMgr.getOpenGLContextCurrentThreadFromCache(contextId, threadId);

            if (!retVal)
            {
                // Get the amount of debugged process threads:
                int threadsAmount = 0;
                bool rc = gaGetAmountOfDebuggedProcessThreads(threadsAmount);

                if (rc)
                {
                    retVal = true;

                    // Iterate the threads:
                    for (int i = 0; i < threadsAmount; i++)
                    {
                        // Get the current thread id:
                        osThreadId currentThreadId = OS_NO_THREAD_ID;
                        rc = gaGetThreadId(i, currentThreadId);

                        if (rc)
                        {
                            // Get the thread current render context:
                            int currentRenderContextId = -1;
                            rc = gaGetThreadCurrentRenderContext(currentThreadId, currentRenderContextId);

                            // If this is not the NULL context id:
                            if (currentRenderContextId != 0)
                            {
                                // If this is the queried context:
                                if (contextId == currentRenderContextId)
                                {
                                    threadId = currentThreadId;
                                    break;
                                }
                            }
                        }
                    }
                }

                // If we succeeded, add this value to the cache:
                if (retVal)
                {
                    thePersistentDataMgr.cacheThreadToOpenGLContextCorrelation(threadId, contextId);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetThreadCallStack
// Description:
//   Returns the call stack of a debugged process thread.
//   The thread must be suspended before the call to this function.
//
// Arguments:   threadId - The id of the queried thread.
//              pCallStack - Will get a pointer to the thread call stack.
//              hideSpyDLLsFunctions - if true, stack frames that contain spy DLLs
//                                    functions (and all stack frames that appear beneath
//                                    them) will be removed from the output call stack.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/10/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetThreadCallStack(const osThreadId& threadId, osCallStack& callStack, bool hideSpyDLLsFunctions, bool fetchRemoteSourceFiles)
{
    bool retVal = false;

    // Verify that the debugged process is suspended:
    if (gaIsDebuggedProcessSuspended())
    {
        // Get the process debugger:
        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

        // If a process debugger cannot get calls stacks, it's not actively debugging
        // the application, so we need to get the calls stack throught the API:
        if (theProcessDebugger.canGetCallStacks())
        {
            // Get the debugged thread call stack:
            retVal = theProcessDebugger.getDebuggedThreadCallStack(threadId, callStack, hideSpyDLLsFunctions);

            if (retVal)
            {
                theProcessDebugger.fillCallsStackDebugInfo(callStack, hideSpyDLLsFunctions);
            }
        }
        else
        {
            // The process debugger cannot get call stacks, we need to ask the spy to get it for us:
            // Verify that the API is active:
            bool isAPIActive = gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION);
            GT_IF_WITH_ASSERT(isAPIActive)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the function Id:
                spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadGetCallStack;

                // Send the arguments:
                spyConnectionSocket << (gtUInt64)threadId;
                spyConnectionSocket << hideSpyDLLsFunctions;

                // Receive success value:
                spyConnectionSocket >> retVal;

                if (retVal)
                {
                    // Read the call stack:
                    retVal = callStack.readSelfFromChannel(spyConnectionSocket);
                }
            }
        }
    }

    // Localize the file paths in the call stack as needed:
    if (retVal && fetchRemoteSourceFiles)
    {
        retVal = gaRemoteToLocalCallStack(callStack);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCanGetHostVariables
// Description: Returns true iff we can get host variable values.
// Author:      Uri Shomroni
// Date:        10/9/2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaCanGetHostVariables()
{
    bool retVal = gaIsDebuggedProcessSuspended();

    if (retVal)
    {
        retVal = pdProcessDebugger::instance().canGetHostVariables();
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////
/// \brief Check if host debugging available
///
/// \return true - available/false - unavailable
/// \author Vadim Entov
/// \date 05/01/2015
bool gaGRApiFunctions::gaCanGetHostDebugging()
{
    return pdProcessDebugger::instance().canPerformHostDebugging();
}

//////////////////////////////////////////////////////////////////
/// \brief Do host debugging "step in"
///
/// \param thradId an gdb id of stepping thread
///
/// \return true - success/false - failed
/// \author Vadim Entov
/// \date 05/01/2015
bool gaGRApiFunctions::gaHostDebuggerStepIn(osThreadId threadId)
{
    bool retVal = gaIsDebuggedProcessSuspended() && gaCanGetHostDebugging();

    if (retVal)
    {
        retVal = pdProcessDebugger::instance().performHostStep(threadId, pdProcessDebugger::PD_STEP_IN);
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////
/// \brief Do host debugging "step out"
///
/// \param thradId an gdb id of stepping thread
///
/// \return true - success/false - failed
/// \author Vadim Entov
/// \date 05/01/2015
bool gaGRApiFunctions::gaHostDebuggerStepOut(osThreadId threadId)
{
    bool retVal = gaIsDebuggedProcessSuspended() && gaCanGetHostDebugging();

    if (retVal)
    {
        retVal = pdProcessDebugger::instance().performHostStep(threadId, pdProcessDebugger::PD_STEP_OUT);
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////
/// \brief Do host debugging "step over"
///
/// \param thradId an gdb id of stepping thread
///
/// \return true - success/false - failed
/// \author Vadim Entov
/// \date 05/01/2015
bool gaGRApiFunctions::gaHostDebuggerStepOver(osThreadId threadId)
{
    bool retVal = gaIsDebuggedProcessSuspended() && gaCanGetHostDebugging();

    if (retVal)
    {
        retVal = pdProcessDebugger::instance().performHostStep(threadId, pdProcessDebugger::PD_STEP_OVER);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCanGetHostVariables
// Description: Get the list of available locals for the specificed frame context
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/9/2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetThreadLocals(const osThreadId& threadId, int frameIndex, gtVector<gtString>& o_locals)
{
    bool retVal = gaCanGetHostVariables();

    GT_IF_WITH_ASSERT(retVal)
    {
        retVal = pdProcessDebugger::instance().getHostLocals(threadId, frameIndex, o_locals);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCanGetHostVariables
// Description: Get the variable value in the specified frame context.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/9/2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetThreadVariableValue(const osThreadId& threadId, int frameIndex, const gtString& variableName, gtString& o_variableValue, gtString& o_variableValueHex, gtString& o_variableType)
{
    bool retVal = gaCanGetHostVariables();

    GT_IF_WITH_ASSERT(retVal)
    {
        retVal = pdProcessDebugger::instance().getHostVariableValue(threadId, frameIndex, variableName, o_variableValue, o_variableValueHex, o_variableType);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////
/// \brief Check debugged process state
///
/// \return true - debugged process under host breakpoint
/// \author Vadim Entov
/// \date 05/01/2016
bool gaGRApiFunctions::gaIsHostBreakPoint()
{
    bool isAPIBreakpoint = pdProcessDebugger::instance().isAtAPIOrKernelBreakpoint(OS_NO_THREAD_ID);
    return !isAPIBreakpoint;
}

/////////////////////////////////////////////////////////////////////
/// \brief Set host break point to process debugger instance
///
/// \param filePath a source file full name
/// \param lineNumber a breakpoint line number
/// \return true - success, false - failed
///
/// \author Vadim Entov
/// \date 21/01/2016
bool gaGRApiFunctions::gaSetHostBreakpoint(const osFilePath& filePath, int lineNumber)
{
    bool retVal = gaCanGetHostDebugging();

    GT_IF_WITH_ASSERT(retVal)
    {
        if (gaDebuggedProcessExists())
        {
            retVal = pdProcessDebugger::instance().setHostSourceBreakpoint(filePath, lineNumber);
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////
/// \brief Get the requested location for a AP_HOST_BREAKPOINT_HIT event:
///
/// \param bpFile a source file full name
/// \param bpLine a breakpoint line number
/// \return true - success, false - failed
///
/// \author Uri Shomroni
/// \date 24/03/2016
bool gaGRApiFunctions::gaGetHostBreakpointLocation(osFilePath& bpFile, int& bpLine)
{
    bool retVal = gaCanGetHostDebugging();

    GT_IF_WITH_ASSERT(retVal)
    {
        if (gaDebuggedProcessExists() && gaIsHostBreakPoint())
        {
            retVal = pdProcessDebugger::instance().getHostBreakpointLocation(bpFile, bpLine);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfRenderContexts
// Description:
//   Returns the amount of OpenGL contexts created by the debugged application.
//   Notice:
//   Context 0 is the "Non context" context - It will log all the function calls
//   that were called when there is no active context.
//   The "Non context" context is created by the debugger and always exists.
//   Examples:
//   - The first context creation function calls (wglChoosePixelFormat, wglCreateContext,
//     etc) will be logged into it.
//   - When the application calls wglMakeCurrent with a NULL render context - we
//     set context 0 as the active context.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        6/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfRenderContexts(int& contextsAmount)
{
    bool retVal = false;

    // Debug printout:
    OS_OUTPUT_DEBUG_LOG(L"Starting gaGetAmountOfRenderContexts", OS_DEBUG_LOG_DEBUG);

    // Verify that the API is active:
    bool isAPIActive = gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION);
    GT_IF_WITH_ASSERT(isAPIActive)
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfRenderContexts;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        spyConnectionSocket >> retVal;

        GT_IF_WITH_ASSERT(retVal)
        {
            // Receive parameters:
            gtInt32 contextsAmountAsInt32 = 0;
            spyConnectionSocket >> contextsAmountAsInt32;
            contextsAmount = (int)contextsAmountAsInt32;
        }
    }

    // Debug printout:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg = L"Ending gaGetAmountOfRenderContexts. Contexts amount = ";
        debugMsg.appendFormattedString(L"%d", contextsAmount);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaWasContextDeleted
// Description:
// Arguments:   apContextID& contextId
// Return Val:  bool - true iff context was deleted
// Author:      Sigal Algranaty
// Date:        8/8/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaWasContextDeleted(const apContextID& contextId)
{
    bool retVal = stat_thePersistentDataMgr.wasContextDeleted(contextId);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderContextDetails
// Description: inserts into sharingContextId the internal ID of the context from
//              which context # contextId takes its lists or -1 if there isn't such a context
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/6/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderContextDetails(int contextId, apGLRenderContextInfo& renderContextInfo)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetRenderContextDetails;

        // Send arguments:
        spyConnectionSocket << (gtInt32)contextId;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive Render Context information:
            renderContextInfo.readSelfFromChannel(spyConnectionSocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderContextGraphicDetails
// Description: Gets the apGLRenderContextGraphicsInfo that describes the context
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        19/3/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderContextGraphicDetails(int contextId, apGLRenderContextGraphicsInfo& renderContextGraphicsInfo)
{
    bool retVal = true;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetRenderContextGraphicsDetails;

        // Send arguments:
        spyConnectionSocket << (gtInt32)contextId;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive Render Context Graphics information:
            renderContextGraphicsInfo.readSelfFromChannel(spyConnectionSocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderContextLivingShareListsHolder
// Description: Inserts into livingContextID a render context ID of a context
//              which qualifies the following two conditions:
//                  A. It is a living context (not deleted)
//                  B. It belongs to the same sharelists group as contextID.
//              if no such context exists, returns -1 and false.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        9/11/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderContextLivingShareListsHolder(int contextID, int& livingContextID)
{
    bool retVal = true;

    // Check if the context was deleted:
    bool wasDeleted = gaWasContextDeleted(apContextID(AP_OPENGL_CONTEXT, contextID));

    if (wasDeleted)
    {
        // If our context is dead, find a replacement:
        int numberOfContexts = 0;
        bool rcAmount = gaGetAmountOfRenderContexts(numberOfContexts);
        GT_IF_WITH_ASSERT(rcAmount)
        {
            apGLRenderContextInfo contextInfo;
            bool rcShareLists = gaGetRenderContextDetails(contextID, contextInfo);
            GT_IF_WITH_ASSERT(rcShareLists)
            {
                // Find out which spy render context monitor is holding the information
                int origContextSharelists = contextInfo.sharingContextID();

                if (origContextSharelists == -1)
                {
                    origContextSharelists = contextID;
                }

                // Iterate the render contexts, looking for another one with the same resources:
                for (int i = 1; i < numberOfContexts; i++)
                {
                    // See if the current context is living:
                    wasDeleted = gaWasContextDeleted(apContextID(AP_OPENGL_CONTEXT, i));

                    if (!wasDeleted)
                    {
                        // Reset the parameter's allocated object ID to allow overwrite:
                        contextInfo.setAllocatedObjectId(-1, true);

                        // See if the current context has the same resources:
                        rcShareLists = gaGetRenderContextDetails(i, contextInfo);
                        GT_IF_WITH_ASSERT(rcShareLists)
                        {
                            if ((origContextSharelists == i) || (origContextSharelists == contextInfo.sharingContextID()))
                            {
                                livingContextID = i;
                                retVal = true;
                                break;
                            }
                        }
                    }

                    // if this is the last iteration, mark the value of the output as a failure:
                    if (i == numberOfContexts - 1)
                    {
                        livingContextID = -1;
                        retVal = false;
                    }
                }
            }
        }
        else
        {
            livingContextID = contextID;
            retVal = true;
        }
    }
    else
    {
        livingContextID = contextID;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLContextDetails
// Description: Returns an OpenCL context info
// Arguments:   int contextId
//              apCLContext& renderContextInfo
// Return Val:  GA_API bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/12/2009
// ---------------------------------------------------------------------------
GA_API bool gaGRApiFunctions::gaGetOpenCLContextDetails(int contextId, apCLContext& contextInfo)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
    {
        // Check if the context was deleted:
        apContextID clContextId(AP_OPENCL_CONTEXT, contextId);
        bool wasContextDeleted = gaWasContextDeleted(clContextId);

        if (!wasContextDeleted)
        {
            bool rcUpdateCtxData = stat_thePersistentDataMgr.updateContextDataSnapshot(clContextId);
            GT_ASSERT(rcUpdateCtxData);
        }

        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetOpenCLContextDetails;

        // Send arguments:
        spyConnectionSocket << (gtInt32)contextId;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive Render Context information:
            contextInfo.readSelfFromChannel(spyConnectionSocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenGLStateVariables
// Description: Returns the amount of OpenGL state variable that we currently
//              monitor.
// Arguments:   amountOfStateVariables - Will get the output of this function.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/7/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenGLStateVariables(int& amountOfStateVariables)
{
    // Get the amount of OpenGL state variables:
    apOpenGLStateVariablesManager& stateVariablesMgr = apOpenGLStateVariablesManager::instance();
    amountOfStateVariables = stateVariablesMgr.amountOfStateVariables();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenGLStateVariableName
// Description: Inputs an OpenGL state variable id, and returns its name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/7/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenGLStateVariableName(int stateVariableId, gtString& stateVariableName)
{
    // Get the input state variables name:
    apOpenGLStateVariablesManager& stateVariablesMgr = apOpenGLStateVariablesManager::instance();
    stateVariableName = stateVariablesMgr.stateVariableName(stateVariableId);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenGLStateVariableGlobalType
// Description: Inputs an OpenGL state variable id, and returns its global type (OpenGL / OpenGL ES).
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        9/11/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenGLStateVariableGlobalType(int stateVariableId, unsigned int& stateVariableGlobalType)
{
    // Get the input state variables global type:
    apOpenGLStateVariablesManager& stateVariablesMgr = apOpenGLStateVariablesManager::instance();
    stateVariableGlobalType = stateVariablesMgr.stateVariableGlobalType(stateVariableId);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenGLStateVariableId
// Description: Inputs a state variable name and returns its id.
// Arguments:   stateVariableName - The state variable name.
//              stateVariableId - Will get the state variable id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/7/2004
//   We currently use a linear search :-(
//   TO_DO: A better approach will be to hold the state variables
//          sorted alphabetically, and perform a binary search.
//          (Do this in apOpenGLStateVariablesManager)
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenGLStateVariableId(const gtString& stateVariableName, int& stateVariableId)
{
    bool retVal = false;

    // Get the amount of OpenGL state variables:
    apOpenGLStateVariablesManager& stateVariablesMgr = apOpenGLStateVariablesManager::instance();
    int amountOfStateVariables = stateVariablesMgr.amountOfStateVariables();

    for (int i = 0; i < amountOfStateVariables; i++)
    {
        // Get the current state variable name:
        const wchar_t* pCurrentStateVarName = stateVariablesMgr.stateVariableName(i);

        if (stateVariableName == pCurrentStateVarName)
        {
            // We found the input state variable:
            stateVariableId = i;
            retVal = true;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfMonitoredFunctions
// Description: Returns the amount of monitored functions (functions that we
//              can listen to).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/3/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfMonitoredFunctions(int& amountOfFunctions)
{
    // Get the amount of monitored functions:
    apMonitoredFunctionsManager& monitoredFunctionsMgr = apMonitoredFunctionsManager::instance();
    amountOfFunctions = monitoredFunctionsMgr.amountOfMonitoredFunctions();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetMonitoredFunctionName
// Description: Inputs a monitored function ID and returns its name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetMonitoredFunctionName(apMonitoredFunctionId functionId, gtString& functionName)
{
    bool retVal = false;

    // Get the amount of monitored functions:
    apMonitoredFunctionsManager& monitoredFunctionsMgr = apMonitoredFunctionsManager::instance();
    int amountOfMonitoredFuncs = monitoredFunctionsMgr.amountOfMonitoredFunctions();

    // Check that the functionId is in the right range:
    if ((0 <= functionId) && (functionId < amountOfMonitoredFuncs))
    {
        // Get the requested function name:
        functionName = monitoredFunctionsMgr.monitoredFunctionName(functionId);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetMonitoredFunctionId
// Description: Inputs a monitored function name and returns its id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/5/2004
// Implementation Notes:
//   We currently use a linear search :-(
//   TO_DO: A better approach will be to hold the monitored functions
//          sorted alphabetically, and perform a binary search.
//          (Do this in apMonitoredFunctionsManager)
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetMonitoredFunctionId(const gtString& functionName, apMonitoredFunctionId& functionId)
{
    bool retVal = false;

    // Get the amount of monitored functions:
    apMonitoredFunctionsManager& monitoredFunctionsMgr = apMonitoredFunctionsManager::instance();
    int amountOfMonitoredFuncs = monitoredFunctionsMgr.amountOfMonitoredFunctions();

    for (int i = 0; i < amountOfMonitoredFuncs; i++)
    {
        // Get the current function name:
        const wchar_t* pCurrentFuncName = monitoredFunctionsMgr.monitoredFunctionName((apMonitoredFunctionId)i);

        if (functionName == pCurrentFuncName)
        {
            // We found the input function:
            functionId = (apMonitoredFunctionId)i;
            retVal = true;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetMonitoredFunctionType
// Description: Inputs a monitored function id and returns its type as a bitwise
//              mask of apFunctionType.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetMonitoredFunctionType(apMonitoredFunctionId functionId, unsigned int& functionType)
{
    bool retVal = false;

    // Get the amount of monitored functions:
    apMonitoredFunctionsManager& monitoredFunctionsMgr = apMonitoredFunctionsManager::instance();
    int amountOfMonitoredFuncs = monitoredFunctionsMgr.amountOfMonitoredFunctions();

    // Check that the functionId is in the right range:
    if ((0 <= functionId) && (functionId < amountOfMonitoredFuncs))
    {
        // Get the requested function type:
        functionType = monitoredFunctionsMgr.monitoredFunctionType(functionId);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetMonitoredFunctionAPIType
// Description: Inputs a monitored function id and returns its API type as a bitwise
//              mask of apFunctionType.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/3/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetMonitoredFunctionAPIType(apMonitoredFunctionId functionId, unsigned int& functionType)
{
    bool retVal = false;

    // Get the amount of monitored functions:
    apMonitoredFunctionsManager& monitoredFunctionsMgr = apMonitoredFunctionsManager::instance();
    int amountOfMonitoredFuncs = monitoredFunctionsMgr.amountOfMonitoredFunctions();

    // Check that the functionId is in the right range:
    if ((0 <= functionId) && (functionId < amountOfMonitoredFuncs))
    {
        // Get the requested function type:
        functionType = monitoredFunctionsMgr.monitoredFunctionAPIType(functionId);
        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetMonitoredFunctionDeprecationVersion
// Description: Inputs a monitored function id and returns its deprecation versions
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetMonitoredFunctionDeprecationVersion(apMonitoredFunctionId functionId, apAPIVersion& deprectedAtVersion, apAPIVersion& removedAtVersion)
{
    bool retVal = false;

    // Get the amount of monitored functions:
    apMonitoredFunctionsManager& monitoredFunctionsMgr = apMonitoredFunctionsManager::instance();
    int amountOfMonitoredFuncs = monitoredFunctionsMgr.amountOfMonitoredFunctions();

    // Check that the functionId is in the right range:
    if ((0 <= functionId) && (functionId < amountOfMonitoredFuncs))
    {
        // Get the requested function deprecation versions:
        monitoredFunctionsMgr.getMonitoredFunctionDeprecationVersions(functionId, deprectedAtVersion, removedAtVersion);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetDefaultOpenGLStateVariableValue
// Description:
//   Inputs the id of a state variable and returns the default OpenGL / ES value
//   for this state variable in a given context.
// Arguments:   contextId - The input context id.
//              stateVariableId - The id of the queried state variable.
//              aptrStateVariableValue - Will get the state variable default value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/5/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetDefaultOpenGLStateVariableValue(int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetDefaultOpenGLStateVariableValue;

            // Send input variables:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)stateVariableId;

            // Receive success value:
            bool rc = false;
            spyConnectionSocket >> rc;

            if (rc)
            {
                // Read the state variable default value from the channel:
                gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
                spyConnectionSocket >> aptrReadTransferableObj;

                // Verify that we read a parameter object:
                if (aptrReadTransferableObj->isParameterObject())
                {
                    aptrStateVariableValue = (apParameter*)(aptrReadTransferableObj.releasePointedObjectOwnership());
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenGLStateVariableValue
// Description: Returns an OpenGL state variable value in a given context.
// Arguments:   contextId - The input context id.
//              stateVariableId - The id of the queried state variable.
//              aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/7/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenGLStateVariableValue(int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            bool rc = stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            if (rc)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the function Id:
                spyConnectionSocket << (gtInt32)GA_FID_gaGetOpenGLStateVariableValue;

                // Send input variables:
                spyConnectionSocket << (gtInt32)contextId;
                spyConnectionSocket << (gtInt32)stateVariableId;

                // Receive success value:
                rc = false;
                spyConnectionSocket >> rc;

                if (rc)
                {
                    // Read the state variable value from the channel:
                    gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
                    spyConnectionSocket >> aptrReadTransferableObj;

                    // Verify that we read a parameter object:
                    if (aptrReadTransferableObj->isParameterObject())
                    {
                        aptrStateVariableValue = (apParameter*)(aptrReadTransferableObj.releasePointedObjectOwnership());
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaEnableImagesDataLogging
// Description:
//   Enable / Disable textures image data logging.
//   The textures image data will be logged into files that reside under the
//   log files directory.
// Arguments:   isTexturesDataLoggingEnabled - true - enable textures image data logging.
//                                           - false - disable textures image data logging.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/3/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaEnableImagesDataLogging(bool isTexturesImageDataLogged)
{
    bool retVal = false;

    // Set the "Enable Monitored Function Calls Logging" mode:
    retVal = stat_thePersistentDataMgr.enableImagesDataLogging(isTexturesImageDataLogged);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaResetRecordingWasDoneFlag
// Description: Reset the "Recording was done" flag
// Arguments:   isEnabled - Forces the "recording was done" variable
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        2/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaResetRecordingWasDoneFlag(bool isEnabled)
{
    // Reset the "Recording was done" flag
    stat_thePersistentDataMgr.resetRecordingWasDoneFlag(isEnabled);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsImagesDataLogged
// Description: Query the status of textures image data logging.
// Arguments:
//   isTexturesImageDataLogged - Will get the query result:
//   - true - textures image data logging is enabled.
//   - false - textures image data logging is disabled.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/3/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsImagesDataLogged(bool& isTexturesImageDataLogged)
{
    // Get the "Textures Image Data Logged" status:
    isTexturesImageDataLogged = stat_thePersistentDataMgr.isImagesDataLogged();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfTextureUnits
// Description: Returns the amount of mulit-texture texture units that are
//              supported by the queried context.
// Arguments:   contextId - The queried context id.
//              amountOfTextureUnits - Will get the amount of supported texture units.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfTextureUnits(int contextId, int& amountOfTextureUnits)
{
    bool retVal = false;
    amountOfTextureUnits = 0;

    // If the NULL context is queried:
    if (contextId == 0)
    {
        amountOfTextureUnits = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfTextureUnits;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfTextureUnitsAsInt32 = 1;
                spyConnectionSocket >> amountOfTextureUnitsAsInt32;
                amountOfTextureUnits = (int)amountOfTextureUnitsAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetActiveTextureUnit
// Description: Inputs a context id, and returns its active texture unit id.
// Arguments:   contextId - The input render context id.
//              activeTextureUnitId - The output texture unit id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetActiveTextureUnit(int contextId, int& activeTextureUnitId)
{
    bool retVal = false;
    activeTextureUnitId = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetActiveTextureUnit;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 activeTextureUnitIdAsInt32 = 0;
                spyConnectionSocket >> activeTextureUnitIdAsInt32;
                activeTextureUnitId = (int)activeTextureUnitIdAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureUnitName
// Description: Retrieves a texture unit OpenGL name out of its id.
// Arguments:   contextId - The queried context id.
//              textureUnitId - The queried texture unit id.
//              textureUnitName - Will get the output texture unit name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureUnitName(int contextId, int textureUnitId, GLenum& textureUnitName)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureUnitName;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Set the texture unit it:
            spyConnectionSocket << (gtInt32)textureUnitId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 textureUnitNameAsInt32 = GL_TEXTURE0;
                spyConnectionSocket >> textureUnitNameAsInt32;
                textureUnitName = (GLenum)textureUnitNameAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetEnabledTexturingMode
// Description: Returns the currently enabled texturing mode.
// Arguments:   contextId - The queried context id.
//              textureUnitId - The queried texture unit id.
//              isTexturingEnabled - Will get true iff texturing is enabled.
//              enabledTexturingMode - Will get the enabled texturing mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/12/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetEnabledTexturingMode(int contextId,  int textureUnitId, bool& isTexturingEnabled, apTextureType& enabledTexturingMode)
{
    bool retVal = false;
    enabledTexturingMode = AP_UNKNOWN_TEXTURE_TYPE;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetEnabledTexturingMode;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Send the texture unit id:
            spyConnectionSocket << (gtInt32)textureUnitId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                spyConnectionSocket >> isTexturingEnabled;

                gtInt32 enabledTexturingModeAsInt32 = AP_2D_TEXTURE;
                spyConnectionSocket >> enabledTexturingModeAsInt32;
                enabledTexturingMode = (apTextureType)enabledTexturingModeAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetBoundTexture
// Description: Retrieves the name of a texture that is bounded to a texture
//              bind target.
// Arguments:   contextId - The queried context id.
//              textureUnitId - The queried texture unit id.
//              bindTarget - The queried bind target.
//              textureName - The output bounded texture object name, or 0 if there is no
//                            texture object bounded to the input target.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/12/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetBoundTexture(int contextId, int textureUnitId, apTextureType bindTarget, GLuint& textureName)
{
    bool retVal = false;
    textureName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetBoundTexture;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)textureUnitId;
            spyConnectionSocket << (gtInt32)bindTarget;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 textureNameAsUInt32 = 0;
                spyConnectionSocket >> textureNameAsUInt32;
                textureName = (GLuint)textureNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfStaticBuffersObjects
// Description: Inputs an OpenGL context id and returns the amount of static buffers
//              objects currently exist in it.
// Arguments:   contextId - The id of the queried texture.
//              amountOfStaticBuffers - The amount of static buffers existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfStaticBuffersObjects(int contextId, int& amountOfStaticBuffers)
{
    bool retVal = false;

    // If the input context is the NULL context:
    if (contextId == 0)
    {
        amountOfStaticBuffers = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfStaticBuffersObjects;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfStaticBuffersAsInt32 = 0;
                spyConnectionSocket >> amountOfStaticBuffersAsInt32;
                amountOfStaticBuffers = (int)amountOfStaticBuffersAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfPBuffersObjects
// Description: Inputs an OpenGL context id and returns the amount of PBuffers
//              objects currently exist in it.
// Arguments:   amountOfPBuffers - The amount of PBuffers existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfPBuffersObjects(int& amountOfPBuffers)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfPBuffersObjects;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfPBuffersAsInt32 = 0;
            spyConnectionSocket >> amountOfPBuffersAsInt32;
            amountOfPBuffers = (int)amountOfPBuffersAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfSyncObjects
// Description: Return the amount of Sync object currently exist in the OpenGL application
// Arguments:   amountOfPBuffers - The amount of PBuffers existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfSyncObjects(int& amountOfSyncObjects)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfSyncObjects;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfSyncObjectsAsInt32 = 0;
            spyConnectionSocket >> amountOfSyncObjectsAsInt32;
            amountOfSyncObjects = (int)amountOfSyncObjectsAsInt32;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfFBOs
// Description: Inputs an OpenGL context id and returns the amount of FBOs
//              objects currently exist in it.
// Arguments:   contextId - The context id
//              amountOfFBOs - The amount of FBOs existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfFBOs(int contextId, int& amountOfFBOs)
{
    bool retVal = false;

    // If the input context is the NULL context:
    if (contextId == 0)
    {
        amountOfFBOs = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfFBOs;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfFBOsAsInt32 = 0;
                spyConnectionSocket >> amountOfFBOsAsInt32;
                amountOfFBOs = (int)amountOfFBOsAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfVBOs
// Description: Inputs an OpenGL context id and returns the amount of VBOs
//              objects currently exist in it.
// Arguments:   contextId - The context id
//              amountOfVBOs - The amount of VBOs existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfVBOs(int contextId, int& amountOfVBOs)
{
    bool retVal = false;

    // If the input context is the NULL context:
    if (contextId == 0)
    {
        amountOfVBOs = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfVBOs;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfVBOsAsInt32 = 0;
                spyConnectionSocket >> amountOfVBOsAsInt32;
                amountOfVBOs = (int)amountOfVBOsAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetVBODetails
// Description: Returns an VBO details.
// Arguments:   contextId - The id in which the texture resides.
//              vboName - The OpenGL name of the vertex buffer object.
//              vboDetails - Will get the VBO details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetVBODetails(int contextId, GLuint vboName, apGLVBO& vboDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetVBODetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)vboName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = vboDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetVBOAttachment
// Description: Returns an VBO attachment.
// Arguments:   contextId - The id in which the vbo resides.
//              vboName - The OpenGL name of the vertex buffer object.
//              vboDetails - Will get the VBO attachment.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetVBOAttachment(int contextId, GLuint vboName, GLenum& vboLastTarget, gtVector<GLenum>& vboCurrentTargets)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetVBOAttachment;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)vboName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 vboLastTargetAsUInt32 = GL_NONE;
                spyConnectionSocket >> vboLastTargetAsUInt32;
                vboLastTarget = (GLenum)vboLastTargetAsUInt32;

                gtUInt32 vboCurrentAttachmentCount = 0;
                spyConnectionSocket >> vboCurrentAttachmentCount;

                for (gtUInt32 i = 0; vboCurrentAttachmentCount > i; ++i)
                {
                    gtUInt32 currentAttachment = 0;
                    spyConnectionSocket >> currentAttachment;
                    vboCurrentTargets.push_back((GLenum)currentAttachment);
                }
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetFBOName
// Description: Inputs an FBO object id (in its context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the texture resides.
//              fboId - The if of the render buffer object in this context.
//              fboName - The OpenGL name of the FBO.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetFBOName(int contextId, int fboId, GLuint& fboName)
{
    bool retVal = false;
    fboName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetFBOName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)fboId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 fboNameAsUInt32 = 0;
                spyConnectionSocket >> fboNameAsUInt32;
                fboName = (GLuint)fboNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetVBOName
// Description: Inputs an VBO object id (in its context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the vbo resides.
//              fboId - The if of the vertex buffer object in this context.
//              fboName - The OpenGL name of the VBO.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetVBOName(int contextId, int vboId, GLuint& vboName)
{
    bool retVal = false;
    vboName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetVBOName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)vboId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 vboNameAsUInt32 = 0;
                spyConnectionSocket >> vboNameAsUInt32;
                vboName = (GLuint)vboNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetActiveFBO
// Description: Returns the currently active FBO
// Arguments: int contextId
//            GLuint& fboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/6/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetActiveFBO(int contextId, GLuint& fboName)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetActiveFBO;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                gtUInt32 fboNameAsUInt32 = 0;
                spyConnectionSocket >> fboNameAsUInt32;
                fboName = (GLuint)fboNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetFBODetails
// Description: Returns an FBO details.
// Arguments:   contextId - The id in which the texture resides.
//              fboName - The OpenGL name of the render buffer object.
//              fboDetails - Will get the FBO details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetFBODetails(int contextId, GLuint fboName, apGLFBO& fboDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetFBODetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)fboName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = fboDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfRenderBufferObjects
// Description: Inputs an OpenGL context id and returns the amount of render buffers
//              objects currently exist in it.
// Arguments:   contextId - The id of the queried texture.
//              amountOfRenderBuffers - The amount of context existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfRenderBufferObjects(int contextId, int& amountOfRenderBuffers)
{
    bool retVal = false;

    // If the input context is the NULL context:
    if (contextId == 0)
    {
        amountOfRenderBuffers = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfRenderBufferObjects;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfRenderBuffersAsInt32 = 0;
                spyConnectionSocket >> amountOfRenderBuffersAsInt32;
                amountOfRenderBuffers = (int)amountOfRenderBuffersAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderBufferObjectName
// Description: Inputs a render buffer  object id (in its context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the texture resides.
//              renderBufferId - The if of the render buffer object in this context.
//              renderBufferName - The OpenGL name of the render buffer object.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderBufferObjectName(int contextId, int renderBufferId, GLuint& renderBufferName)
{
    bool retVal = false;
    renderBufferName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetRenderBufferObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)renderBufferId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 renderBufferNameAsUInt32 = 0;
                spyConnectionSocket >> renderBufferNameAsUInt32;
                renderBufferName = (GLuint)renderBufferNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfTextureObjects
// Description: Inputs an OpenGL context id and returns the amount of textures
//              objects currently exist in it.
// Arguments:   contextId - The id of the queried texture.
//              amountOfTextures - The amount of context existing in this context.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfTextureObjects(int contextId, int& amountOfTextures)
{
    bool retVal = false;

    // If the input context is the NULL context:
    if (contextId == 0)
    {
        amountOfTextures = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfTextureObjects;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfTexturesAsInt32 = 0;
                spyConnectionSocket >> amountOfTexturesAsInt32;
                amountOfTextures = (int)amountOfTexturesAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureObjectName
// Description: Inputs a texture object id (in its context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the texture resides.
//              textureId - The if of the texture object in this context.
//              textureName - The OpenGL name of the texture object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureObjectName(int contextId, int textureId, GLuint& textureName)
{
    bool retVal = false;
    textureName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)textureId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 textureNameAsUInt32 = 0;
                spyConnectionSocket >> textureNameAsUInt32;
                textureName = (GLuint)textureNameAsUInt32;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureObjectType
// Description: Inputs a texture object id (in its context) and returns its type.
// Arguments:   contextId - The context in which the texture resides.
//              textureId - The if of the texture object in this context.
//              textureType - The texture type
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/8/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureObjectType(int contextId, int textureId, apTextureType& textureType)
{
    bool retVal = false;
    textureType = AP_UNKNOWN_TEXTURE_TYPE;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureObjectType;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)textureId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 textureTypeAsInt = 0;
                spyConnectionSocket >> textureTypeAsInt;
                textureType = (apTextureType)textureTypeAsInt;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfPBufferContentObjects
// Description: Inputs an OpenGL PBuffer object id and returns the amount of static buffers
//              objects currently linked with it.
// Arguments:   pbufferId - The id of the queried PBuffer;
//              amountOfPBufferContentObjects - The amount of static buffers existing in this PBuffer.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        03/09/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfPBufferContentObjects(int pbufferID, int& amountOfPBufferContentObjects)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfPBufferContentObjects;

        // Send the context id:
        spyConnectionSocket << (gtInt32)pbufferID;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfPBufferContentObjectsAsInt32 = 0;
            spyConnectionSocket >> amountOfPBufferContentObjectsAsInt32;
            amountOfPBufferContentObjects = (int)amountOfPBufferContentObjectsAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPBufferStaticBufferType
// Description: Inputs an OpenGL PBuffer object id and a static buffer iter
//              (between 0..Amount of static buffers in pbuffer -1.
//              And function will return the static buffer type.
// Arguments:   pbufferID - The id of the queried PBuffer;
//              staticBufferIter - Static buffer loop iteration 0..amount - 1
//              bufferType - Output static buffer type
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        2/1/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPBufferStaticBufferType(int pbufferID, int staticBufferIter, apDisplayBuffer& bufferType)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetPBufferStaticBufferType;

        // Send the PBuffer id:
        spyConnectionSocket << (gtInt32)pbufferID;

        // Send the staticBuffer Iteration
        spyConnectionSocket << (gtInt32)staticBufferIter;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 bufferTypeAsInt32 = AP_DISPLAY_BUFFER_UNKNOWN;
            spyConnectionSocket >> bufferTypeAsInt32;

            // Cast to apDisplayBuffer
            bufferType = (apDisplayBuffer)bufferTypeAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetStaticBufferObjectDetails
// Description:
//   Returns a static buffer details.
//   Notice: To get updated buffer's raw data, call gaGRApiFunctions::gaUpdateStaticBufferRawData
//           before the call to this function.
//
// Arguments:   contextId - The id in which the texture resides.
//              staticBufferId - The static buffer API id (between 0..Amount of static)
//              bufferType - Output buffer type
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetStaticBufferType(int contextId, int staticBufferId, apDisplayBuffer& bufferType)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetStaticBufferType;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)staticBufferId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                gtInt32 bufferTypeAsInt32 = AP_DISPLAY_BUFFER_UNKNOWN;
                spyConnectionSocket >> bufferTypeAsInt32;
                bufferType = (apDisplayBuffer)bufferTypeAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetStaticBufferObjectDetails
// Description:
//   Returns a static buffer details.
//   Notice: To get updated buffer's raw data, call gaGRApiFunctions::gaUpdateStaticBufferRawData
//           before the call to this function.
//
// Arguments:   contextId - The id in which the texture resides.
//              staticBufferId - The static buffer API id (between 0..Amount of static)
//              staticBufferDetails - Output static buffer object details.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        27/08/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetStaticBufferObjectDetails(int contextId, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetStaticBufferObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)bufferType;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = staticBufferDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateStaticBufferRawData
// Description: Updates (stored to disk) the raw data of a given static buffer.
// Arguments: contextId - The id of the render context in which the buffer resides.
//            staticBufferId - The static buffer id.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/10/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateStaticBufferRawData(int contextId, apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Sanity check:
            if (0 < contextId)
            {
                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

                if (rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Get gaUpdateCurrentThreadStaticBufferRawDataStub address:
                            osProcedureAddress64 gaUpdateCurrentThreadStaticBufferRawDataStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadStaticBufferRawData);
                            GT_IF_WITH_ASSERT(gaUpdateCurrentThreadStaticBufferRawDataStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                            {
                                // Yaki 24/1/2008:
                                // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                                // of data before the first one is read out of the pipe, the write is blocked.
                                // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                                // the Spy side does not read data from the pipe, we need to write all data in one block.

                                // A buffer that will contain the single data block to be written:
                                osRawMemoryStream buff;

                                // Write function arguments into the buffer:
                                buff << (gtInt32)bufferType;

                                // Write the entire buffer into the API socket, thus writing only one block of data:
                                spyConnectionSocket << buff;

                                // Make the "current thread" execute gaUpdateCurrentThreadStaticBufferRawDataStub:
                                rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadStaticBufferRawDataStubAddr);

                                if (rc)
                                {
                                    // Read the return value:
                                    spyConnectionSocket >> retVal;
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateStaticBufferRawData;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the parameters:
                            spyConnectionSocket << (gtInt32)bufferType;

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread that has the render context as "current thread":
                    else
                    {
                        // Yaki 30/1/2008
                        // Make the spy API thread the input render context's current thread to get the buffer data.
                        // This is only possible on Windows and Mac since there a render context only belongs to the
                        // window in which it was created. On Linux (and EGL) the same render context can be used
                        // to render into different drawables, therefore, when the render context is not connected
                        // to a drawable, we cannot know which drawables buffers to get (see glXMakeContextCurrent
                        // for more details).
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        {
                            // Send the function Id:
                            spyConnectionSocket << (gtInt32)GA_FID_gaUpdateStaticBufferRawData;

                            // Send parameters:
                            spyConnectionSocket << (gtInt32)contextId;
                            spyConnectionSocket << (gtInt32)bufferType;

                            // Receive success value:
                            spyConnectionSocket >> retVal;
                        }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateStaticBufferRawData
// Description: Updates all static buffers dimensions according to HDC size
// Arguments: int contextId - the context id
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateStaticBuffersDimension(int contextId)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Sanity check:
            if (0 < contextId)
            {
                bool wasContextDeleted = gaWasContextDeleted(apContextID(AP_OPENGL_CONTEXT, contextId));

                // If the context was deleted, do nothing:
                if (!wasContextDeleted)
                {
                    // Update the context data snapshot (if needed):
                    apContextID id(AP_OPENGL_CONTEXT, contextId);
                    bool rc1 = stat_thePersistentDataMgr.updateContextDataSnapshot(id);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Get the thread that is has the input context as its "current context":
                        osThreadId threadId = OS_NO_THREAD_ID;
                        bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

                        if (rc)
                        {
                            // Get the Spy connecting socket:
                            osSocket& spyConnectionSocket = gaSpiesAPISocket();

                            // If there is a thread that has the input context as its "current context":
                            if (threadId != OS_NO_THREAD_ID)
                            {
                                // Get the process debugger instance:
                                pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                                if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                                {
                                    // Get gaUpdateCurrentThreadStaticBufferRawDataStub address:
                                    osProcedureAddress64 gaUpdateCurrentThreadStaticBuffersDimensionsStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadStaticBuffersDimensions);
                                    GT_IF_WITH_ASSERT(gaUpdateCurrentThreadStaticBuffersDimensionsStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                                    {
                                        // Make the "current thread" execute gaUpdateCurrentThreadStaticBufferRawDataStub:
                                        rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadStaticBuffersDimensionsStubAddr);

                                        if (rc)
                                        {
                                            // Read the return value:
                                            spyConnectionSocket >> retVal;
                                        }
                                    }
                                }
                                else
                                {
#ifdef _SUPPORT_REMOTE_EXECUTION
                                    // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                                    // execute the function using gsSpyBreakpointImplementation:

                                    // Send the command ID:
                                    spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateStaticBuffersDimensions;

                                    // Send the thread ID:
                                    spyConnectionSocket << (gtUInt64)threadId;

                                    // Receive success value:
                                    spyConnectionSocket >> retVal;
#else
                                    // We shouldn't get here on these builds:
                                    GT_ASSERT(false);
#endif
                                }
                            }
                            // Else - If there is no thread that has the render context as "current thread":
                            else
                            {
                                // Yaki 30/1/2008
                                // Make the spy API thread the input render context's current thread to get the buffer data.
                                // This is only possible on Windows and Mac since there a render context only belongs to the
                                // window in which it was created. On Linux (and EGL) the same render context can be used
                                // to render into different drawables, therefore, when the render context is not connected
                                // to a drawable, we cannot know which drawables buffers to get (see glXMakeContextCurrent
                                // for more details).
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                                {
                                    // Send the function Id:
                                    spyConnectionSocket << (gtInt32)GA_FID_gaUpdateStaticBuffersDimensions;

                                    // Send parameters:
                                    spyConnectionSocket << (gtInt32)contextId;

                                    // Receive success value:
                                    spyConnectionSocket >> retVal;
                                }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdatePBuffersDimension
// Description: Updates all pbuffers dimensions according to HDC size
// Arguments: int contextId - the context id
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdatePBuffersDimension(int contextId)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Sanity check:
            if (0 < contextId)
            {
                bool wasContextDeleted = gaWasContextDeleted(apContextID(AP_OPENGL_CONTEXT, contextId));

                // If the context was deleted, do nothing:
                if (!wasContextDeleted)
                {
                    // Get the thread that is has the input context as its "current context":
                    osThreadId threadId = OS_NO_THREAD_ID;
                    bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

                    if (rc)
                    {
                        // Get the Spy connecting socket:
                        osSocket& spyConnectionSocket = gaSpiesAPISocket();

                        // If there is a thread that has the input context as its "current context":
                        if (threadId != OS_NO_THREAD_ID)
                        {
                            // Get the process debugger instance:
                            pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                            if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                            {
                                // Get gaUpdateCurrentThreadStaticBufferRawDataStub address:
                                osProcedureAddress64 gaUpdateCurrentThreadStaticBuffersDimensionsStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadPBuffersDimensions);
                                GT_IF_WITH_ASSERT(gaUpdateCurrentThreadStaticBuffersDimensionsStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                                {
                                    // Make the "current thread" execute gaUpdateCurrentThreadStaticBufferRawDataStub:
                                    rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadStaticBuffersDimensionsStubAddr);

                                    if (rc)
                                    {
                                        // Read the return value:
                                        spyConnectionSocket >> retVal;
                                    }
                                }
                            }
                            else
                            {
#ifdef _SUPPORT_REMOTE_EXECUTION
                                // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                                // execute the function using gsSpyBreakpointImplementation:

                                // Send the command ID:
                                spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdatePBuffersDimensions;

                                // Send the thread ID:
                                spyConnectionSocket << (gtUInt64)threadId;

                                // Receive success value:
                                spyConnectionSocket >> retVal;
#else
                                // We shouldn't get here on these builds:
                                GT_ASSERT(false);
#endif
                            }
                        }
                        // Else - If there is no thread that has the render context as "current thread":
                        else
                        {
                            // Yaki 30/1/2008
                            // Make the spy API thread the input render context's current thread to get the buffer data.
                            // This is only possible on Windows and Mac since there a render context only belongs to the
                            // window in which it was created. On Linux (and EGL) the same render context can be used
                            // to render into different drawables, therefore, when the render context is not connected
                            // to a drawable, we cannot know which drawables buffers to get (see glXMakeContextCurrent
                            // for more details).
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                            {
                                // Send the function Id:
                                spyConnectionSocket << (gtInt32)GA_FID_gaUpdatePBuffersDimensions;

                                // Send parameters:
                                spyConnectionSocket << (gtInt32)contextId;

                                // Receive success value:
                                spyConnectionSocket >> retVal;
                            }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdatePBufferStaticBufferRawData
// Description: Updates (stored to disk) the raw data of a given PBuffer and
//              the relevant static buffer.
// Arguments:   pbufferContextId - The id of the render context
//              in which the PBuffer resides.
//              pbufferID - The id of the PBuffer
//              bufferType - The static buffer type to extract
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdatePBufferStaticBufferRawData(int pbufferContextId, int pbufferID, apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Sanity check:
            if (0 < pbufferContextId)
            {
                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(pbufferContextId, threadId);

                if (rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Get gaUpdateCurrentThreadStaticBufferRawDataStub address:
                            osProcedureAddress64 gaUpdateCurrentThreadPBufferStaticBufferRawDataStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadPBufferStaticBufferRawData);
                            GT_IF_WITH_ASSERT(gaUpdateCurrentThreadPBufferStaticBufferRawDataStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                            {
                                // Yaki 24/1/2008:
                                // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                                // of data before the first one is read out of the pipe, the write is blocked.
                                // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                                // the Spy side does not read data from the pipe, we need to write all data in one block.

                                // A buffer that will contain the single data block to be written:
                                osRawMemoryStream buff;

                                // Write function arguments into the buffer:
                                buff << (gtInt32)pbufferID;
                                buff << (gtInt32)bufferType;

                                // Write the entire buffer into the API socket, thus writing only one block of data:
                                spyConnectionSocket << buff;

                                // Make the "current thread" execute gaUpdateCurrentThreadPBufferStaticBufferRawDataStub:
                                rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadPBufferStaticBufferRawDataStubAddr);

                                if (rc)
                                {
                                    // Read the return value:
                                    spyConnectionSocket >> retVal;
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdatePBufferStaticBufferRawData;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the parameters:
                            spyConnectionSocket << (gtInt32)pbufferID;
                            spyConnectionSocket << (gtInt32)bufferType;

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread attached to this render context, we assume the render
                    // context is attached to the spy, so we ask the spy to get us the data.
                    else
                    {
                        // Send the function Id:
                        spyConnectionSocket << (gtInt32)GA_FID_gaUpdatePBufferStaticBufferRawData;

                        // Send parameters:
                        spyConnectionSocket << (gtInt32)pbufferID;
                        spyConnectionSocket << (gtInt32)bufferType;

                        // Receive success value:
                        spyConnectionSocket >> retVal;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPBufferStaticBufferObjectDetails
// Description: Returns a static buffer details, from the PBuffer static buffers
// Arguments:   pbufferID - The PBuffer id in which the static buffer exists;
//              staticBufferIter - The static buffer iteration (between 0..Amount of static)
//              staticBufferDetails - Output static buffer object details.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        03/09/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPBufferStaticBufferObjectDetails(int pbufferID, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetPBufferStaticBufferObjectDetails;

        // Send parameters:
        spyConnectionSocket << (gtInt32)pbufferID;

        gtInt32 bufferTypeAsInt32 = (gtInt32)bufferType;
        spyConnectionSocket << bufferTypeAsInt32;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = staticBufferDetails.readSelfFromChannel(spyConnectionSocket);
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPBufferObjectDetails
// Description: Returns a PBuffer details.
// Arguments:   pbufferID - The id of the PBuffer to get the details about
//              pbufferDetails - Output PBuffer object details.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        28/08/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPBufferObjectDetails(int pbufferID, apPBuffer& pbufferDetails)
{
    bool retVal = false;

    // If the input pbufferID is valid
    if (pbufferID != -1)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetPBufferObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)pbufferID;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = pbufferDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetSyncObjectDetails
// Description: Returns a sync object details.
// Arguments:   syncObjectID - The id of the sync object to get the details about
//              syncObjectDetails - Output sync object details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetSyncObjectDetails(int syncObjectID, apGLSync& syncObjectDetails)
{
    bool retVal = false;

    // If the input pbufferID is valid
    if (syncObjectID != -1)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetSyncObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)syncObjectID;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = syncObjectDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateRenderBufferRawData
// Description: Updates the render buffer raw data file
// Arguments: int contextId
//            const gtVector<GLuint>& renderBuffersVector
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateRenderBufferRawData(int contextId, const gtVector<GLuint>& renderBuffersVector)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the amount of buffers:
            gtInt64 amountOfRenderBuffers = (gtInt64)renderBuffersVector.size();

            // Sanity check:
            GT_IF_WITH_ASSERT((0 < contextId) && (0 < amountOfRenderBuffers))
            {
                // Make sure we have a living context:
                int otherRC = -2;
                bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
                GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
                {
                    if (otherRC == -1)
                    {
                        // there is no context that would work, do nothing (the function will report the failure anyway)
                    }
                    else
                    {
                        contextId = otherRC;
                    }
                }

                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Since calling this with many textures at once causes problems and hangs with the
                            // osRawMemoryStream, we break it down into manageable chunks
                            int numberOfChunks = (int)amountOfRenderBuffers / GA_TEXTURES_UPDATE_CHUNK_SIZE;

                            if ((amountOfRenderBuffers % GA_TEXTURES_UPDATE_CHUNK_SIZE) > 0)
                            {
                                numberOfChunks++;
                            }

                            for (int j = 0; j < numberOfChunks; j++)
                            {
                                int firstBufferInChunk = GA_TEXTURES_UPDATE_CHUNK_SIZE * j;
                                int chunkEnd = GA_TEXTURES_UPDATE_CHUNK_SIZE * (j + 1);

                                if (chunkEnd > amountOfRenderBuffers)
                                {
                                    chunkEnd = (int)amountOfRenderBuffers;
                                }

                                // Get gaUpdateCurrentThreadRenderBufferRawDataStub address:
                                osProcedureAddress64 gaUpdateCurrentThreadRenderBufferRawDataStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadRenderBufferRawData);
                                GT_IF_WITH_ASSERT(gaUpdateCurrentThreadRenderBufferRawDataStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                                {
                                    // Yaki 24/1/2008:
                                    // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                                    // of data before the first one is read out of the pipe, the write is blocked.
                                    // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                                    // the Spy side does not read data from the pipe, we need to write all data in one block.

                                    // A buffer that will contain the single data block to be written:
                                    osRawMemoryStream buff;

                                    // Write the amount of textures into the buffer:
                                    gtInt64 amountOfBuffersInCurrentChunk = (gtInt64)(chunkEnd - firstBufferInChunk);
                                    buff << amountOfBuffersInCurrentChunk;

                                    // And texture names:
                                    for (int i = firstBufferInChunk; i < chunkEnd; i++)
                                    {
                                        gtUInt32 renderBufferId = (gtUInt32)renderBuffersVector[i];
                                        buff << renderBufferId;
                                    }

                                    // Write the entire buffer into the API socket, thus writing only one block of data:
                                    spyConnectionSocket << buff;

                                    // Make the "current thread" execute gaUpdateCurrentThreadRenderBufferRawDataStub:
                                    rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadRenderBufferRawDataStubAddr);

                                    if (rc)
                                    {
                                        // Read the return value:
                                        spyConnectionSocket >> retVal;
                                    }
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // The problem mentioned above might be happening here too, this should somehow be fixed:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateRenderBufferRawData;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the parameters:
                            spyConnectionSocket << amountOfRenderBuffers;

                            for (int i = 0; i < amountOfRenderBuffers; i++)
                            {
                                gtUInt32 bufferName = (gtUInt32)renderBuffersVector[i];
                                spyConnectionSocket << bufferName;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread that has the render context as "current thread":
                    // We will make the spy API thread the input render context's "current thread" to get
                    // the textures data out of the render context. On Linux (and EGL) the same render context can be used
                    // to render into different drawables, therefore, when the render context is not connected
                    // to a drawable, we cannot know which drawable should we connect to the render context.
                    // to get the textures (see glXMakeContextCurrent for more details).
                    else
                    {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        {
                            // Send the function Id:
                            spyConnectionSocket << (gtInt32)GA_FID_gaUpdateRenderBufferRawData;

                            // Send parameters:
                            spyConnectionSocket << (gtInt32)contextId;

                            // Send amount of textures and then all textures id's
                            spyConnectionSocket << amountOfRenderBuffers;

                            for (int i = 0; i < amountOfRenderBuffers; i++)
                            {
                                gtUInt32 bufferName = (gtUInt32)renderBuffersVector[i];
                                spyConnectionSocket << bufferName;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
                        }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateVBORawData
// Description: Updates the VBO data file
// Arguments: int contextId
//            const gtVector<GLuint>& vboNamesVector
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateVBORawData(int contextId, const gtVector<GLuint>& vboNamesVector)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination:
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the amount of VBOs:
            int amountOfVBOs = vboNamesVector.size();

            // Sanity check:
            GT_IF_WITH_ASSERT((0 < contextId) && (0 < amountOfVBOs))
            {
                // Make sure we have a living context:
                int otherRC = -2;
                bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
                GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
                {
                    if (otherRC == -1)
                    {
                        // there is no context that would work, do nothing (the function will report the failure anyway)
                    }
                    else
                    {
                        contextId = otherRC;
                    }
                }

                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Since calling this with many textures at once causes problems and hangs with the
                            // osRawMemoryStream, we break it down into manageable chunks
                            int numberOfChunks = amountOfVBOs / GA_TEXTURES_UPDATE_CHUNK_SIZE;

                            if ((amountOfVBOs % GA_TEXTURES_UPDATE_CHUNK_SIZE) > 0)
                            {
                                numberOfChunks++;
                            }

                            for (int j = 0; j < numberOfChunks; j++)
                            {
                                int firstBufferInChunk = GA_TEXTURES_UPDATE_CHUNK_SIZE * j;
                                int chunkEnd = GA_TEXTURES_UPDATE_CHUNK_SIZE * (j + 1);

                                if (chunkEnd > amountOfVBOs)
                                {
                                    chunkEnd = amountOfVBOs;
                                }

                                // Get gaUpdateCurrentThreadVBORawDataStub address:
                                osProcedureAddress64 gaUpdateCurrentThreadVBORawDataStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadVBORawData);
                                GT_IF_WITH_ASSERT(gaUpdateCurrentThreadVBORawDataStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                                {
                                    // Yaki 24/1/2008:
                                    // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                                    // of data before the first one is read out of the pipe, the write is blocked.
                                    // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                                    // the Spy side does not read data from the pipe, we need to write all data in one block.

                                    // A buffer that will contain the single data block to be written:
                                    osRawMemoryStream buff;

                                    // Write the amount of textures into the buffer:
                                    gtInt64 amountOfBuffersInCurrentChunk = (gtInt64)(chunkEnd - firstBufferInChunk);
                                    buff << amountOfBuffersInCurrentChunk;

                                    // And VBO names:
                                    for (int i = firstBufferInChunk; i < chunkEnd; i++)
                                    {
                                        gtUInt32 vboName = (gtUInt32)vboNamesVector[i];
                                        buff << vboName;
                                    }

                                    // Write the entire buffer into the API socket, thus writing only one block of data:
                                    spyConnectionSocket << buff;

                                    // Make the "current thread" execute gaUpdateCurrentThreadVBORawDataStub:
                                    rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadVBORawDataStubAddr);

                                    if (rc)
                                    {
                                        // Read the return value:
                                        spyConnectionSocket >> retVal;
                                    }
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // The problem mentioned above probably happens here too, we should make sure to fix it here:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateVBORawData;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the parameters:
                            gtInt64 amountOfVBOs = (gtInt64)vboNamesVector.size();
                            spyConnectionSocket << amountOfVBOs;

                            for (int i = 0; i < amountOfVBOs; i++)
                            {
                                gtUInt32 vboName = (gtUInt32)vboNamesVector[i];
                                spyConnectionSocket << vboName;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread that has the render context as "current thread":
                    // We will make the spy API thread the input render context's "current thread" to get
                    // the textures data out of the render context. On Linux (and EGL) the same render context can be used
                    // to render into different drawables, therefore, when the render context is not connected
                    // to a drawable, we cannot know which drawable should we connect to the render context.
                    // to get the textures (see glXMakeContextCurrent for more details).
                    else
                    {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        {
                            // Send the function Id:
                            spyConnectionSocket << (gtInt32)GA_FID_gaUpdateVBORawData;

                            // Send parameters:
                            spyConnectionSocket << (gtInt32)contextId;

                            // Send amount of VBOs and then all VBO names:
                            gtInt64 amountOfVBOs2 = (gtInt64)vboNamesVector.size();
                            spyConnectionSocket << amountOfVBOs2;

                            for (int i = 0; i < amountOfVBOs2; i++)
                            {
                                gtUInt32 vboName = (gtUInt32)vboNamesVector[i];
                                spyConnectionSocket << vboName;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
                        }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetVBODisplayProperties
// Description: Sets a VBO display properties. Is called when the user sets the
//              VBO display properties
// Arguments: int contextId - render context id
//            oaTexelDataFormat displayFormat - the selected display format
//            GLuint vboName - VBO name
//            int offset - the VBO offset
//            GLsizei stride - the VBO stride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/4/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetVBODisplayProperties(int contextId, GLuint vboName, oaTexelDataFormat displayFormat, int offset, GLsizei stride)
{
    bool retVal = false;

    // This function should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination:
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(0 < contextId)
            {
                // Make sure we have a living context:
                int otherRC = -2;
                bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
                GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
                {
                    if (otherRC == -1)
                    {
                        // there is no context that would work, do nothing (the function will report the failure anyway)
                    }
                    else
                    {
                        contextId = otherRC;
                    }
                }

                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Get gaSetCurrentThreadVBODisplayPropertiesStub address:
                            osProcedureAddress64 gaSetCurrentThreadVBODisplayPropertiesStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaSetCurrentThreadVBODisplayProperties);
                            GT_IF_WITH_ASSERT(gaSetCurrentThreadVBODisplayPropertiesStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                            {
                                // A buffer that will contain the single data block to be written:
                                osRawMemoryStream buff;

                                // Write the VBO name to the API socket:
                                buff << (gtUInt32)vboName;

                                // Write the VBO display format to the API socket:
                                buff << (gtInt32)displayFormat;

                                // Write the VBO offset to the API socket:
                                buff << (gtUInt64)offset;

                                // Write the VBO stride to the API socket:
                                buff << (gtUInt64)stride;

                                // Write the entire buffer into the API socket, thus writing only one block of data:
                                spyConnectionSocket << buff;

                                // Make the "current thread" execute gaSetCurrentThreadVBODisplayPropertiesStubAddr:
                                rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaSetCurrentThreadVBODisplayPropertiesStubAddr);

                                if (rc)
                                {
                                    // Read the return value:
                                    spyConnectionSocket >> retVal;
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadSetVBODisplayProperties;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the parameters:
                            spyConnectionSocket << (gtUInt32)vboName;
                            spyConnectionSocket << (gtInt32)displayFormat;
                            spyConnectionSocket << (gtUInt64)offset;
                            spyConnectionSocket << (gtUInt64)stride;

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread that has the render context as "current thread":
                    // We will make the spy API thread the input render context's "current thread" to get
                    // the textures data out of the render context. On Linux (and EGL) the same render context can be used
                    // to render into different drawables, therefore, when the render context is not connected
                    // to a drawable, we cannot know which drawable should we connect to the render context.
                    // to get the textures (see glXMakeContextCurrent for more details).
                    else
                    {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        {
                            // Send the function Id:
                            spyConnectionSocket << (gtInt32)GA_FID_gaSetVBODisplayProperties;

                            // Send parameters:
                            spyConnectionSocket << (gtInt32)contextId;

                            // Set the VBO name:
                            spyConnectionSocket << (gtUInt32)vboName;

                            // Set the VBO display format:
                            spyConnectionSocket << (gtInt32)displayFormat;

                            // Set the VBO offset:
                            spyConnectionSocket << (gtUInt64)offset;

                            // Set the VBO stride:
                            spyConnectionSocket << (gtUInt64)stride;

                            // Receive success value:
                            spyConnectionSocket >> retVal;
                        }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateTextureRawData
// Description: Updates the texture raw data file
// Arguments:   contextId - The id in which the texture resides.
//              textureId - The texture id (name + mip level) to extract the raw data from
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        1/12/2007
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateTextureRawData(int contextId, const gtVector<apGLTextureMipLevelID>& textureIdsVector)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the amount of textures:
            int amountOfTextures = textureIdsVector.size();

            // Sanity check:
            GT_IF_WITH_ASSERT((0 < contextId) && (0 < amountOfTextures))
            {
                // Make sure we have a living context:
                int otherRC = -2;
                bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
                GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
                {
                    if (otherRC == -1)
                    {
                        // there is no context that would work, do nothing (the function will report the failure anyway)
                    }
                    else
                    {
                        contextId = otherRC;
                    }
                }

                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Since calling this with many textures at once causes problems and hangs with the
                            // osRawMemoryStream, we break it down into manageable chunks
                            int numberOfChunks = amountOfTextures / GA_TEXTURES_UPDATE_CHUNK_SIZE;

                            if ((amountOfTextures % GA_TEXTURES_UPDATE_CHUNK_SIZE) > 0)
                            {
                                numberOfChunks++;
                            }

                            for (int j = 0; j < numberOfChunks; j++)
                            {
                                int firstTextureInChunk = GA_TEXTURES_UPDATE_CHUNK_SIZE * j;
                                int chunkEnd = GA_TEXTURES_UPDATE_CHUNK_SIZE * (j + 1);

                                if (chunkEnd > amountOfTextures)
                                {
                                    chunkEnd = amountOfTextures;
                                }

                                // Get gaUpdateCurrentThreadTextureRawDataStub address:
                                osProcedureAddress64 gaUpdateCurrentThreadTextureRawDataStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadTextureRawData);
                                GT_IF_WITH_ASSERT(gaUpdateCurrentThreadTextureRawDataStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                                {
                                    // Yaki 24/1/2008:
                                    // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                                    // of data before the first one is read out of the pipe, the write is blocked.
                                    // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                                    // the Spy side does not read data from the pipe, we need to write all data in one block.

                                    // A buffer that will contain the single data block to be written:
                                    osRawMemoryStream buff;

                                    // Write the amount of textures into the buffer:
                                    gtInt64 amountOfTexturesInCurrentChunk = (gtInt64)(chunkEnd - firstTextureInChunk);
                                    buff << amountOfTexturesInCurrentChunk;

                                    // And texture names:
                                    for (int i = firstTextureInChunk; i < chunkEnd; i++)
                                    {
                                        apGLTextureMipLevelID textureId = textureIdsVector[i];
                                        buff << (gtUInt32)textureId._textureName;
                                        buff << (gtInt32)textureId._textureMipLevel;
                                    }

                                    // Write the entire buffer into the API socket, thus writing only one block of data:
                                    spyConnectionSocket << buff;

                                    // Make the "current thread" execute gaUpdateCurrentThreadTextureRawDataStub:
                                    rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadTextureRawDataStubAddr);

                                    if (rc)
                                    {
                                        // Read the return value:
                                        spyConnectionSocket >> retVal;
                                    }
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // The problem mentioned above probably happens here too, we need to make sure it no longer does:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateTextureRawData;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the parameters:
                            gtInt64 amountOfTextures = (gtInt64)textureIdsVector.size();
                            spyConnectionSocket << amountOfTextures;

                            for (int i = 0; i < amountOfTextures; i++)
                            {
                                apGLTextureMipLevelID textureId = textureIdsVector[i];
                                spyConnectionSocket << (gtUInt32)textureId._textureName;
                                spyConnectionSocket << (gtInt32)textureId._textureMipLevel;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread that has the render context as "current thread":
                    // We will make the spy API thread the input render context's "current thread" to get
                    // the textures data out of the render context. On Linux (and EGL) the same render context can be used
                    // to render into different drawables, therefore, when the render context is not connected
                    // to a drawable, we cannot know which drawable should we connect to the render context.
                    // to get the textures (see glXMakeContextCurrent for more details).
                    else
                    {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        {
                            // Send the function Id:
                            spyConnectionSocket << (gtInt32)GA_FID_gaUpdateTextureRawData;

                            // Send parameters:
                            spyConnectionSocket << (gtInt32)contextId;

                            // Send amount of textures and then all textures id's
                            gtInt64 amountOfTextures2 = (gtInt64)textureIdsVector.size();
                            spyConnectionSocket << amountOfTextures2;

                            for (int i = 0; i < amountOfTextures2; i++)
                            {
                                apGLTextureMipLevelID textureId = textureIdsVector[i];
                                spyConnectionSocket << (gtUInt32)textureId._textureName;
                                spyConnectionSocket << (gtInt32)textureId._textureMipLevel;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
                        }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateTextureParameters
// Description: Updates textures parameters
// Arguments:   int contextId - the id in whicn the textures reside
//              const gtVector<apGLTextureMipLevelID>& texturesVector - vector of texture ids to extract
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateTextureParameters(int contextId, const gtVector<apGLTextureMipLevelID>& texturesIdsVector, bool shouldUpdateOnlyMemoryParams)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the amount of textures:
            gtInt64 amountOfTextures = (gtInt64)texturesIdsVector.size();

            // Sanity check:
            GT_IF_WITH_ASSERT((0 < contextId) && (0 < amountOfTextures))
            {
                // Make sure we have a living context:
                int otherRC = -2;
                bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
                GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
                {
                    if (otherRC == -1)
                    {
                        // there is no context that would work, do nothing (the function will report the failure anyway)
                    }
                    else
                    {
                        contextId = otherRC;
                    }
                }

                // Get the thread that is has the input context as its "current context":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the Spy connecting socket:
                    osSocket& spyConnectionSocket = gaSpiesAPISocket();

                    // If there is a thread that has the input context as its "current context":
                    if (threadId != OS_NO_THREAD_ID)
                    {
                        // Get the process debugger instance:
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Since calling this with many textures at once causes problems and hangs with the
                            // osRawMemoryStream, we break it down into manageable chunks
                            int numberOfChunks = (int)amountOfTextures / GA_TEXTURES_UPDATE_CHUNK_SIZE;

                            if ((amountOfTextures % GA_TEXTURES_UPDATE_CHUNK_SIZE) > 0)
                            {
                                numberOfChunks++;
                            }

                            for (int j = 0; j < numberOfChunks; j++)
                            {
                                int firstTextureInChunk = GA_TEXTURES_UPDATE_CHUNK_SIZE * j;
                                int chunkEnd = GA_TEXTURES_UPDATE_CHUNK_SIZE * (j + 1);

                                if (chunkEnd > amountOfTextures)
                                {
                                    chunkEnd = (int)amountOfTextures;
                                }

                                // Get gaUpdateCurrentThreadTextureParametersStub address:
                                osProcedureAddress64 gaUpdateCurrentThreadTextureParametersStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadTextureParameters);
                                GT_IF_WITH_ASSERT(gaUpdateCurrentThreadTextureParametersStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                                {
                                    // Yaki 24/1/2008:
                                    // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                                    // of data before the first one is read out of the pipe, the write is blocked.
                                    // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                                    // the Spy side does not read data from the pipe, we need to write all data in one block.

                                    // A buffer that will contain the single data block to be written:
                                    osRawMemoryStream buff;

                                    // Send the update only memory parameters flag:
                                    buff << shouldUpdateOnlyMemoryParams;

                                    // Write the amount of textures into the buffer:
                                    gtInt64 amountOfTexturesInCurrentChunk = (gtInt64)(chunkEnd - firstTextureInChunk);
                                    buff << amountOfTexturesInCurrentChunk;

                                    // And texture names:
                                    for (int i = firstTextureInChunk; i < chunkEnd; i++)
                                    {
                                        apGLTextureMipLevelID textureId = texturesIdsVector[i];

                                        // Write the texture name:
                                        buff << (gtUInt32)textureId._textureName;

                                        // Write the requested mip level:
                                        buff << (gtInt32)textureId._textureMipLevel;
                                    }

                                    // Write the entire buffer into the API socket, thus writing only one block of data:
                                    spyConnectionSocket << buff;

                                    // Make the "current thread" execute gaUpdateCurrentThreadTextureRawDataStub:
                                    rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadTextureParametersStubAddr);

                                    if (rc)
                                    {
                                        // Read the return value:
                                        spyConnectionSocket >> retVal;
                                    }
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:

                            // The problem mentioned above probably happens here too, we should make sure it no longer happens:

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateTextureParameters;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Send the memory parameters only flag:
                            spyConnectionSocket << shouldUpdateOnlyMemoryParams;

                            // Send the parameters:
                            spyConnectionSocket << amountOfTextures;

                            for (int i = 0; i < amountOfTextures; i++)
                            {
                                apGLTextureMipLevelID textureId = texturesIdsVector[i];
                                spyConnectionSocket << (gtUInt32)textureId._textureName;
                                spyConnectionSocket << (gtInt32)textureId._textureMipLevel;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }
                    // Else - If there is no thread that has the render context as "current thread":
                    // We will make the spy API thread the input render context's "current thread" to get
                    // the textures data out of the render context. On Linux (and EGL) the same render context can be used
                    // to render into different drawables, therefore, when the render context is not connected
                    // to a drawable, we cannot know which drawable should we connect to the render context.
                    // to get the textures (see glXMakeContextCurrent  for more details).
                    else
                    {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                        {
                            // Send the function Id:
                            spyConnectionSocket << (gtInt32)GA_FID_gaUpdateTextureParameters;

                            // Send parameters:
                            spyConnectionSocket << (gtInt32)contextId;
                            spyConnectionSocket << shouldUpdateOnlyMemoryParams;

                            // Send amount of textures and then all textures id's
                            spyConnectionSocket << (gtInt64)amountOfTextures;

                            for (int i = 0; i < amountOfTextures; i++)
                            {
                                apGLTextureMipLevelID textureId = texturesIdsVector[i];
                                spyConnectionSocket << (gtUInt32)textureId._textureName;
                                spyConnectionSocket << (gtInt32)textureId._textureMipLevel;
                            }

                            // Receive success value:
                            spyConnectionSocket >> retVal;
                        }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsTextureImagesDirty
// Description:
//   Check if a given texture's images or raw data needs to be updated.
//   A given texture's raw data and image needs to be updated after one or more of the texture's faces
//   was changes by OpenGL. Notice that the textures image is created using the textures raw data,
//   therefore the application should first update the texture's raw data and then the texture's image.
// Arguments:   contextId - The id in which the texture resides.
//              apGLTextureMipLevelID textureMiplevelId - the texture mip level id
//              dirtyImageExists - Output true iff none-updated texture's face images exist.
//              dirtyRawDataExists - Output true iff none-updated texture's face raw data exist.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        6/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsTextureImageDirty(int contextId, apGLTextureMipLevelID textureMiplevelId, bool& dirtyImageExists, bool& dirtyRawDataExists)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaIsTextureImageDirty;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)textureMiplevelId._textureName;
            spyConnectionSocket << (gtInt32)textureMiplevelId._textureMipLevel;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                spyConnectionSocket >> dirtyImageExists;
                spyConnectionSocket >> dirtyRawDataExists;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaMarkAllTextureImagesAsUpdated
// Description: Marks all textures image as updated (not dirty)
// Arguments:   contextId - The id in which the texture resides.
//              textureId - The OpenGL name of the texture object.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        5/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaMarkAllTextureImagesAsUpdated(int contextId, int textureId)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaMarkAllTextureImagesAsUpdated;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)textureId;

            // Receive success value:
            spyConnectionSocket >> retVal;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureMiplevelDataFilePath
// Description:
// Arguments: int contextId - The id in which the texture resides.
//            apGLTextureMipLevelID textureMiplevelId - the requested mip level id
//            int faceIndex - the requested cube map face id
//            osFilePath& filePath - output - the file path
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/1/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureMiplevelDataFilePath(int contextId, apGLTextureMipLevelID textureMiplevelId, int faceIndex, osFilePath& filePath)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureMiplevelDataFilePath;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)textureMiplevelId._textureName;
            spyConnectionSocket << (gtInt32)textureMiplevelId._textureMipLevel;
            spyConnectionSocket << (gtInt32)faceIndex;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = filePath.readSelfFromChannel(spyConnectionSocket);


                // Don't localize the path, as it will be localized by consumers of this function:
                /*
                if (retVal)
                {
                    retVal = gaRemoteToLocalFile(filePath, false);
                }
                */
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureObjectDetails
// Description: Returns a texture details.
// Arguments:   contextId - The id in which the texture resides.
//              textureName - The OpenGL name of the texture object.
//              textureDetails - Will get the texture object details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureObjectDetails(int contextId, GLuint textureName, apGLTexture& textureDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)textureName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = textureDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureObjectThumbnailData
// Description: Returns a texture thumbnail data details.
// Arguments:   contextId - The id in which the texture resides.
//              textureName - The OpenGL name of the texture object.
//              textureThumbDetails - Will get the texture object details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/11/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureObjectThumbnailData(int contextId, GLuint textureName, apGLTextureMiplevelData& textureThumbDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureThumbnailDataObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)textureName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = textureThumbDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureDataObjectDetails
// Description: Returns a texture data details.
// Arguments:   contextId - The id in which the texture resides.
//              textureName - The OpenGL name of the texture object.
//              textureThumbDetails - Will get the texture object details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/11/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureDataObjectDetails(int contextId, GLuint textureName, apGLTextureData& textureData)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureDataObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)textureName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = textureData.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetTextureMemoryDataObjectDetails
// Description: Returns a texture memory data details.
// Arguments:   contextId - The id in which the texture resides.
//              textureName - The OpenGL name of the texture object.
//              textureMemoryData - Will get the texture object details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/5/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetTextureMemoryDataObjectDetails(int contextId, GLuint textureName, apGLTextureMemoryData& textureMemoryData)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetTextureMemoryDataObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)textureName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = textureMemoryData.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderBufferObjectDetails
// Description: Returns a render buffer details.
// Arguments:   contextId - The id in which the texture resides.
//              renderBufferName - The OpenGL name of the render buffer object.
//              renderBufferDetails - Will get the render buffer object details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderBufferObjectDetails(int contextId, GLuint renderBufferName, apGLRenderBuffer& renderBufferDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetRenderBufferObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)renderBufferName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = renderBufferDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfProgramObjects
// Description: Inputs a context Id and returns the amount of program objects
//              that currently exist in it.
// Arguments:   contextId - The input context id.
//              amountOfPrograms - Will get the amount of program objects.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfProgramObjects(int contextId, int& amountOfPrograms)
{
    bool retVal = false;

    // If the queried context is the NULL context:
    if (contextId == 0)
    {
        amountOfPrograms = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfProgramObjects;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfProgramsAsInt32 = 0;
                spyConnectionSocket >> amountOfProgramsAsInt32;
                amountOfPrograms = (int)amountOfProgramsAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetActiveProgramObjectName
// Description: Returns the name of the active program object in a given context.
// Arguments:   contextId - The context id.
//              activeProgramName - Will get the active program name, or 0 if there
//                                  is no active program in the queried context.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/6/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetActiveProgramObjectName(int contextId, GLuint& activeProgramName)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetActiveProgramObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 activeProgramNameAsUInt32 = 0;
                spyConnectionSocket >> activeProgramNameAsUInt32;
                activeProgramName = (GLuint)activeProgramNameAsUInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetProgramObjectName
// Description: Inputs a program object id (in its context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the program object resides.
//              programId - The id of the program object in this context.
//              programName - The OpenGL name of the program object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetProgramObjectName(int contextId, int programId, GLuint& programName)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetProgramObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)programId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 programNameAsUInt32 = 0;
                spyConnectionSocket >> programNameAsUInt32;
                programName = (GLuint)programNameAsUInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetProgramObjectDetails
// Description: Returns a program object details.
// Arguments:   contextId - The context in which the program resides.
//              programName - The program object name.
//              programDetails - Will get the program object details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetProgramObjectDetails(int contextId, GLuint programName, apGLProgram& programDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetProgramObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)programName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = programDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetProgramActiveUniforms
// Description: Returns the active uniforms of a given program.
//              The active uniforms of a program will be returned only after the
//              program was linked. The active uniform values will be returned only
//              if the link was successful.
// Arguments:   contextId - The id of the context in which the program resides.
//              programName - The program name.
//              activeUniforms - Will get the active uniforms names. types and
//                               values (if available).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/5/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetProgramActiveUniforms(int contextId, GLuint programName, apGLItemsCollection& activeUniforms)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetProgramActiveUniforms;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)programName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = activeUniforms.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaLinkProgramObject
// Description: Links a given program object.
// Arguments: contextId - The id of the context in which the program resides.
//            programName - The program name.
//            wasLinkSuccessful - Will get true iff the program link operation
//                                succeeded.
//            linkLog - The link operation log.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaLinkProgramObject(int contextId, GLuint programName, bool& wasLinkSuccessful, gtString& linkLog)
{
    bool retVal = false;

    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    // Verify that the API is active and suspended:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION) && !isInKernelDebugging)
    {
        // Sanity check:
        if (0 < contextId)
        {
            // Make sure we have a living context:
            int otherRC = -2;
            bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
            GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
            {
                if (otherRC == -1)
                {
                    // there is no context that would work, do nothing (the function will report the failure anyway)
                }
                else
                {
                    contextId = otherRC;
                }
            }

            // Get the thread that is has the input context as its "current context":
            osThreadId threadId = OS_NO_THREAD_ID;
            bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

            if (rc)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // If there is a thread that has the input context as its "current context":
                if (threadId != OS_NO_THREAD_ID)
                {
                    // Get the process debugger instance:
                    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                    if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                    {
                        // Get gaLinkProgramObjectStub address:
                        osProcedureAddress64 gaLinkProgramObjectStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaLinkCurrentThreadProgramObject);
                        GT_IF_WITH_ASSERT(gaLinkProgramObjectStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                        {
                            // Yaki 24/1/2008:
                            // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                            // of data before the first one is read out of the pipe, the write is blocked.
                            // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                            // the Spy side does not read data from the pipe, we need to write all data in one block.

                            // A buffer that will contain the single data block to be written:
                            osRawMemoryStream buff;

                            // Write the context id and shader name into the buffer:
                            buff << (gtUInt32)programName;

                            // Write the entire buffer into the API socket, thus writing only one block of data:
                            spyConnectionSocket << buff;

                            // Make the "current thread" execute gaLinkProgramObjectStub:
                            rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaLinkProgramObjectStubAddr);

                            if (rc)
                            {
                                // Read the return value:
                                spyConnectionSocket >> retVal;

                                if (retVal)
                                {
                                    spyConnectionSocket >> wasLinkSuccessful;
                                    spyConnectionSocket >> linkLog;
                                }
                            }
                        }
                    }
                    else
                    {
#ifdef _SUPPORT_REMOTE_EXECUTION
                        // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                        // execute the function using gsSpyBreakpointImplementation:

                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadLinkProgramObject;

                        // Send the thread ID:
                        spyConnectionSocket << (gtUInt64)threadId;

                        // Send the parameters:
                        spyConnectionSocket << (gtUInt32)programName;

                        // Receive success value:
                        spyConnectionSocket >> retVal;

                        if (retVal)
                        {
                            spyConnectionSocket >> wasLinkSuccessful;
                            spyConnectionSocket >> linkLog;
                        }

#else
                        // We shouldn't get here on these builds:
                        GT_ASSERT(false);
#endif
                    }
                }
                // Else - If there is no thread that has the render context as "current thread":
                // We will make the spy API thread the input render context's "current thread" to get
                // the render context programs link. On Linux (and EGL) the same render context can be used
                // to render into different drawables, therefore, when the render context is not connected
                // to a drawable, we cannot know which drawable should we connect to the render context.
                // to get the textures (see glXMakeContextCurrent for more details).
                else
                {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    {
                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaLinkProgramObject;

                        // Send the context ID:
                        spyConnectionSocket << (gtInt32)contextId;

                        // Send the program name:
                        spyConnectionSocket << (gtUInt32)programName;

                        // Receive success value:
                        spyConnectionSocket >> retVal;

                        if (retVal)
                        {
                            spyConnectionSocket >> wasLinkSuccessful;
                            spyConnectionSocket >> linkLog;
                        }
                    }
#endif
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaValidateProgramObject
// Description:
//  Validates a given program object. The validation process checks to see whether
//  the executables contained in the program object can execute given the render
//  context current OpenGL state.
// Arguments: contextId - The id of the context in which the program resides.
//            programName - The program name.
//            wasValidationSuccessful - true means that the program is guaranteed
//                                      to execute given the current state.
//                                    -  false means that the program is guaranteed
//                                       to not execute.
//            validationLog - The validation operation log.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaValidateProgramObject(int contextId, GLuint programName, bool& wasValidationSuccessful, gtString& validationLog)
{
    bool retVal = false;

    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    // Verify that the API is active and suspended:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION) && !isInKernelDebugging)
    {
        // Sanity check:
        if (0 < contextId)
        {
            // Make sure we have a living context:
            int otherRC = -2;
            bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
            GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
            {
                if (otherRC == -1)
                {
                    // there is no context that would work, do nothing (the function will report the failure anyway)
                }
                else
                {
                    contextId = otherRC;
                }
            }

            // Get the thread that is has the input context as its "current context":
            osThreadId threadId = OS_NO_THREAD_ID;
            bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

            if (rc)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // If there is a thread that has the input context as its "current context":
                if (threadId != OS_NO_THREAD_ID)
                {
                    // Get the process debugger instance:
                    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                    if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                    {
                        // Get gaValidateProgramObjectStub address:
                        osProcedureAddress64 gaValidateProgramObjectStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaValidateCurrentThreadProgramObject);
                        GT_IF_WITH_ASSERT(gaValidateProgramObjectStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                        {
                            // Yaki 24/1/2008:
                            // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                            // of data before the first one is read out of the pipe, the write is blocked.
                            // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                            // the Spy side does not read data from the pipe, we need to write all data in one block.

                            // A buffer that will contain the single data block to be written:
                            osRawMemoryStream buff;

                            // Write the context id and the program name into the buffer:
                            buff << (gtUInt32)programName;

                            // Write the entire buffer into the API socket, thus writing only one block of data:
                            spyConnectionSocket << buff;

                            // Make the "current thread" execute gaValidateProgramObjectStub:
                            rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaValidateProgramObjectStubAddr);

                            if (rc)
                            {
                                // Read the return value:
                                spyConnectionSocket >> retVal;

                                if (retVal)
                                {
                                    spyConnectionSocket >> wasValidationSuccessful;
                                    spyConnectionSocket >> validationLog;
                                }
                            }
                        }
                    }
                    else
                    {
#ifdef _SUPPORT_REMOTE_EXECUTION
                        // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                        // execute the function using gsSpyBreakpointImplementation:

                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadValidateProgramObject;

                        // Send the thread ID:
                        spyConnectionSocket << (gtUInt64)threadId;

                        // Send the parameters:
                        spyConnectionSocket << (gtUInt32)programName;

                        // Receive success value:
                        spyConnectionSocket >> retVal;

                        if (retVal)
                        {
                            spyConnectionSocket >> wasValidationSuccessful;
                            spyConnectionSocket >> validationLog;
                        }

#else
                        // We shouldn't get here on these builds:
                        GT_ASSERT(false);
#endif
                    }
                }
                // Else - If there is no thread that has the render context as "current thread":
                // We will make the spy API thread the input render context's "current thread" to get
                // the render context programs validate. On Linux (and EGL) the same render context can be used
                // to render into different drawables, therefore, when the render context is not connected
                // to a drawable, we cannot know which drawable should we connect to the render context.
                // to get the textures (see glXMakeContextCurrent for more details).
                else
                {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    {
                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaValidateProgramObject;

                        // Send the context ID:
                        spyConnectionSocket << (gtInt32)contextId;

                        // Send the program name:
                        spyConnectionSocket << (gtUInt32)programName;

                        // Receive success value:
                        spyConnectionSocket >> retVal;

                        if (retVal)
                        {
                            spyConnectionSocket >> wasValidationSuccessful;
                            spyConnectionSocket >> validationLog;
                        }
                    }
#endif
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfShaderObjects
// Description: Inputs a context Id and returns the amount of shader objects
//              that currently exist in it.
// Arguments:   contextId - The input context id.
//              amountOfShaders - Will get the amount of shader objects.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfShaderObjects(int contextId, int& amountOfShaders)
{
    bool retVal = false;

    // If the input context the NULL context:
    if (contextId == 0)
    {
        amountOfShaders = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfShaderObjects;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfShadersAsInt32 = 0;
                spyConnectionSocket >> amountOfShadersAsInt32;
                amountOfShaders = (int)amountOfShadersAsInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetShaderObjectName
// Description: Inputs a shader object id (in its context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the shader object resides.
//              shaderId - The id of the shader object in this context.
//              shaderName - The OpenGL name of the shader object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetShaderObjectName(int contextId, int shaderId, GLuint& shaderName)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetShaderObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)shaderId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 shaderNameAsUInt32 = 0;
                spyConnectionSocket >> shaderNameAsUInt32;
                shaderName = (GLuint)shaderNameAsUInt32;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetShaderObjectDetails
// Description: Returns a shader object details.
// Arguments:   contextId - The context in which the shader resides.
//              shaderName - The shader object name.
//              aptrShaderDetails - Will get an object that contains the shader
//                                  object details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetShaderObjectDetails(int contextId, GLuint shaderName, gtAutoPtr<apGLShaderObject>& aptrShaderDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENGL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetShaderObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)shaderName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
                spyConnectionSocket >> aptrReadTransferableObj;
                aptrShaderDetails = (apGLShaderObject*)(aptrReadTransferableObj.releasePointedObjectOwnership());
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaMarkShaderObjectSourceCodeAsForced
// Description:
//   Marks the source code of a given shader object as "forced" by the debugger.
//   From this moment on, the debugged program cannot change the shader's source
//   code.
// Arguments: contextId - The context in which the shader resides.
//            shaderName - The shader object name.
//            isSourceCodeForced - true iff the shader source code is forced.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaMarkShaderObjectSourceCodeAsForced(int contextId, GLuint shaderName, bool isSourceCodeForced)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaMarkShaderObjectSourceCodeAsForced;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)shaderName;
            spyConnectionSocket << isSourceCodeForced;

            // Receive success value:
            spyConnectionSocket >> retVal;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetShaderObjectSourceCode
// Description: Sets a given shader object source code.
// Arguments: contextId - The context in which the shader resides.
//            shaderName - The shader object name.
//            sourceCodeFile - The path of a file that contains the new shader
//                             source code.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/11/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetShaderObjectSourceCode(int contextId, GLuint shaderName, const osFilePath& sourceCodeFile)
{
    // TO_DO: handle remote debugging!
    bool retVal = false;

    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    // Verify that the API is active and suspended:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION) && !isInKernelDebugging)
    {
        // Sanity check:
        if (0 < contextId)
        {
            // Make sure we have a living context:
            int otherRC = -2;
            bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
            GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
            {
                if (otherRC == -1)
                {
                    // there is no context that would work, do nothing (the function will report the failure anyway)
                }
                else
                {
                    contextId = otherRC;
                }
            }

            // Get the thread that is has the input context as its "current context":
            osThreadId threadId = OS_NO_THREAD_ID;
            bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

            if (rc)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // If there is a thread that has the input context as its "current context":
                if (threadId != OS_NO_THREAD_ID)
                {
                    // Get the process debugger instance:
                    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                    if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                    {
                        // Get gaSetShaderObjectSourceCodeStub address:
                        osProcedureAddress64 gaSetShaderObjectSourceCodeStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaSetCurrentThreadShaderObjectSourceCode);
                        GT_IF_WITH_ASSERT(gaSetShaderObjectSourceCodeStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                        {
                            // Yaki 24/1/2008:
                            // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                            // of data before the first one is read out of the pipe, the write is blocked.
                            // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                            // the Spy side does not read data from the pipe, we need to write all data in one block.

                            // A buffer that will contain the single data block to be written:
                            osRawMemoryStream buff;

                            // Write the context id and shader name into the buffer:
                            buff << (gtUInt32)shaderName;

                            // Add the shader's source code file path:
                            bool rc1 = sourceCodeFile.writeSelfIntoChannel(buff);
                            GT_IF_WITH_ASSERT(rc1)
                            {
                                // Write the entire buffer into the API socket, thus writing only one block of data:
                                spyConnectionSocket << buff;

                                // Make the "current thread" execute gaSetShaderObjectSourceCodeStub:
                                rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaSetShaderObjectSourceCodeStubAddr);

                                if (rc)
                                {
                                    // Read the return value:
                                    spyConnectionSocket >> retVal;
                                }
                            }
                        }
                    }
                    else
                    {
#ifdef _SUPPORT_REMOTE_EXECUTION
                        // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                        // execute the function using gsSpyBreakpointImplementation:

                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadSetShaderObjectSourceCode;

                        // Send the thread ID:
                        spyConnectionSocket << (gtUInt64)threadId;

                        // Send the parameters:
                        spyConnectionSocket << (gtUInt32)shaderName;
                        bool rcPath = sourceCodeFile.writeSelfIntoChannel(spyConnectionSocket);
                        GT_ASSERT(rcPath);

                        // Receive success value:
                        spyConnectionSocket >> retVal;
#else
                        // We shouldn't get here on these builds:
                        GT_ASSERT(false);
#endif
                    }
                }
                // Else - If there is no thread that has the render context as "current thread":
                // We will make the spy API thread the input render context's "current thread" to set
                // the render context shaders' source. On Linux (and EGL) the same render context can be used
                // to render into different drawables, therefore, when the render context is not connected
                // to a drawable, we cannot know which drawable should we connect to the render context.
                // to get the textures (see glXMakeContextCurrent for more details).
                else
                {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    {
                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaSetShaderObjectSourceCode;

                        // Send the context ID:
                        spyConnectionSocket << (gtInt32)contextId;

                        // Send the shader name:
                        spyConnectionSocket << (gtUInt32)shaderName;

                        // Send the Source Code Path:
                        bool rcPath = sourceCodeFile.writeSelfIntoChannel(spyConnectionSocket);
                        GT_ASSERT(rcPath);

                        // Receive success value:
                        spyConnectionSocket >> retVal;
                    }
#endif
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCompileShaderObject
// Description: Compiles a given shader object.
// Arguments: contextId - The context in which the shader resides.
//            shaderName - The shader object name.
//            wasCompilationSuccessful - will get true iff the compilation
//                                       succeeded.
//            compilationLog - Will get the compile operation log.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaCompileShaderObject(int contextId, GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog)
{
    bool retVal = false;

    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    // Verify that the API is active and suspended:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION) && !isInKernelDebugging)
    {
        // Sanity check:
        if (0 < contextId)
        {
            // Make sure we have a living context:
            int otherRC = -2;
            bool rcLivingContext = gaGetRenderContextLivingShareListsHolder(contextId, otherRC);
            GT_IF_WITH_ASSERT(rcLivingContext && (otherRC != -2))
            {
                if (otherRC == -1)
                {
                    // there is no context that would work, do nothing (the function will report the failure anyway)
                }
                else
                {
                    contextId = otherRC;
                }
            }

            // Get the thread that is has the input context as its "current context":
            osThreadId threadId = OS_NO_THREAD_ID;
            bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

            if (rc)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // If there is a thread that has the input context as its "current context":
                if (threadId != OS_NO_THREAD_ID)
                {
                    // Get the process debugger instance:
                    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                    if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                    {
                        // Get gaCompileShaderObjectStub address:
                        osProcedureAddress64 gaCompileShaderObjectStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaCompileCurrentThreadShaderObject);
                        GT_IF_WITH_ASSERT(gaCompileShaderObjectStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                        {
                            // Yaki 24/1/2008:
                            // On CentOS 3.8, pipes can contain only one written block of data. If you write another block
                            // of data before the first one is read out of the pipe, the write is blocked.
                            // Since gaBeforeDirectAPIFunctionExecution makes sure that, until the function is executed,
                            // the Spy side does not read data from the pipe, we need to write all data in one block.

                            // A buffer that will contain the single data block to be written:
                            osRawMemoryStream buff;

                            // Write the shader name into the buffer:
                            buff << (gtUInt32)shaderName;

                            // Write the entire buffer into the API socket, thus writing only one block of data:
                            spyConnectionSocket << buff;

                            // Make the "current thread" execute gaCompileShaderObjectStub:
                            rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaCompileShaderObjectStubAddr);

                            if (rc)
                            {
                                // Read the return value:
                                spyConnectionSocket >> retVal;

                                if (retVal)
                                {
                                    // Read output arguments:
                                    spyConnectionSocket >> wasCompilationSuccessful;
                                    spyConnectionSocket >> compilationLog;
                                }
                            }
                        }
                    }
                    else
                    {
#ifdef _SUPPORT_REMOTE_EXECUTION
                        // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                        // execute the function using gsSpyBreakpointImplementation:

                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadCompileShaderObject;

                        // Send the thread ID:
                        spyConnectionSocket << (gtUInt64)threadId;

                        // Send the parameters:
                        spyConnectionSocket << (gtUInt32)shaderName;

                        // Receive success value:
                        spyConnectionSocket >> retVal;

                        if (retVal)
                        {
                            // Read output arguments:
                            spyConnectionSocket >> wasCompilationSuccessful;
                            spyConnectionSocket >> compilationLog;
                        }

#else
                        // We shouldn't get here on these builds:
                        GT_ASSERT(false);
#endif
                    }
                }
                // Else - If there is no thread that has the render context as "current thread":
                // We will make the spy API thread the input render context's "current thread" to get
                // the render context shaders compiile. On Linux (and EGL) the same render context can be used
                // to render into different drawables, therefore, when the render context is not connected
                // to a drawable, we cannot know which drawable should we connect to the render context.
                // to get the textures (see glXMakeContextCurrent for more details).
                else
                {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
                    {
                        // Send the command ID:
                        spyConnectionSocket << (gtInt32)GA_FID_gaCompileShaderObject;

                        // Send the context ID:
                        spyConnectionSocket << (gtInt32)contextId;

                        // Send the shader name:
                        spyConnectionSocket << (gtUInt32)shaderName;

                        // Receive success value:
                        spyConnectionSocket >> retVal;

                        if (retVal)
                        {
                            // Read output arguments:
                            spyConnectionSocket >> wasCompilationSuccessful;
                            spyConnectionSocket >> compilationLog;
                        }
                    }
#endif
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfDisplayLists
// Description: Inputs a context Id and returns the amount of display lists objects
//              that currently exist in it.
// Arguments: int contextID
//            int& amountOfDisplayLists
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfDisplayLists(int contextID, int& amountOfDisplayLists)
{
    bool retVal = false;

    // If the queried context is the NULL context:
    if (contextID == 0)
    {
        amountOfDisplayLists = 0;
        retVal = true;
    }
    else if (contextID > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfDisplayLists;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextID;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfDisplayListsAsInt32 = 0;
                spyConnectionSocket >> amountOfDisplayListsAsInt32;
                amountOfDisplayLists = (int)amountOfDisplayListsAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetDisplayListObjectName
// Description: Gets the OpenGL name of a display list from its internal index:
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetDisplayListObjectName(int contextID, int displayListIndex, GLuint& displayListName)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextID > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetDisplayListObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextID;
            spyConnectionSocket << (gtInt32)displayListIndex;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive name:
                gtUInt32 displayListNameAsUInt32 = 0;
                spyConnectionSocket >> displayListNameAsUInt32;
                displayListName = (GLuint)displayListNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetDisplayListObjectDetails
// Description: Returns a displat list details
// Arguments: int contextID
//            int displayListId
//            apGLDisplayList& pDisplayListDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetDisplayListObjectDetails(int contextID, GLuint displayListName, apGLDisplayList& displayListDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextID > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetDisplayListObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextID;
            spyConnectionSocket << (gtUInt32)displayListName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = displayListDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfCurrentFrameFunctionCalls
// Description: Returns the amount of monitored function calls made to a given
//              context at its current rendering frame.
// Arguments:   apContextID - The id of the context that this function queries.
//              amountOfFunctionCalls - The amount of monitored function calls
//                                      made to this context at its current
//                                      rendering frame.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfCurrentFrameFunctionCalls(const apContextID& contextID, int& amountOfFunctionCalls)
{
    bool retVal = false;

    // Arguments check:
    if (contextID.isValid())
    {
        // Get the right function ID according to context type, and connected APIs:
        apAPIConnectionType apiConnectionType;
        apAPIFunctionId functionId = gaFindMultipleAPIsFunctionID(GA_FID_gaGetAmountOfCurrentFrameFunctionCalls, contextID._contextType, apiConnectionType);

        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apiConnectionType))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)functionId;

            // Send the context ID:
            spyConnectionSocket << (gtInt32)contextID._contextId;

            // Perform after API call actions:
            pdProcessDebugger::instance().afterAPICallIssued();

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfFunctionCallsAsInt32 = 0;
                spyConnectionSocket >> amountOfFunctionCallsAsInt32;
                amountOfFunctionCalls = (int)amountOfFunctionCallsAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCurrentFrameFunctionCall
// Description:
//   Returns the details of a function call, made in a given context at its
//   current rendering frame.
//
// Arguments:   contextId - The id of the context that this function queries.
//              callIndex - The call index (in this context current frame).
//              aptrFunctionCall - The returned function call details.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCurrentFrameFunctionCall(const apContextID& contextID, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    bool rc = false;

    // Arguments check:
    if (contextID.isValid())
    {
        gaPersistentDataManager& thePersistentDataMgr = gaPersistentDataManager::instance();

        rc = thePersistentDataMgr.getCachedFunctionCall(contextID, callIndex, aptrFunctionCall);

        if (!rc)
        {
            // Get the right function ID according to context type, and connected APIs:
            apAPIConnectionType apiConnectionType;
            apAPIFunctionId functionId = gaFindMultipleAPIsFunctionID(GA_FID_gaGetCurrentFrameFunctionCall, contextID._contextType, apiConnectionType);

            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apiConnectionType))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the function Id:
                spyConnectionSocket << (gtInt32) functionId;

                // Send the context id:
                spyConnectionSocket << (gtInt32)contextID._contextId;

                // Send the call index:
                spyConnectionSocket << (gtInt32)callIndex;

                // Perform after API call actions:
                pdProcessDebugger::instance().afterAPICallIssued();

                // Receive success value:
                spyConnectionSocket >> rc;

                if (rc)
                {
                    // Read the function call details from the channel:
                    rc = osReadTransferableObjectFromChannel<apFunctionCall>(spyConnectionSocket, aptrFunctionCall);
                }
            }

            if (rc)
            {
                // If we succeeded, cache the result in the persistent data manager:
                thePersistentDataMgr.cacheFunctionCall(contextID, callIndex, aptrFunctionCall);
            }
        }
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetLastFunctionCall
// Description: Returns the details of the last monitored function call executed
//              in a queried render context.
// Arguments:   contextId - The queried render context.
//              aptrFunctionCall - Will get the function call details.
//                                 Notice that in profiling mode, only the function
//                                 id will be filled.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetLastFunctionCall(const apContextID& contextID, gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    bool retVal = false;

    // Arguments check:
    if (contextID.isValid())
    {
        // Get the API connection type for this context:
        apAPIConnectionType apiConnectionType;
        apAPIFunctionId functionId = gaFindMultipleAPIsFunctionID(GA_FID_gaGetLastFunctionCall, contextID._contextType, apiConnectionType);

        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apiConnectionType))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)functionId;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextID._contextId;

            // Perform after API call actions:
            pdProcessDebugger::instance().afterAPICallIssued();

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Read the function call details from the channel:
                retVal = osReadTransferableObjectFromChannel<apFunctionCall>(spyConnectionSocket, aptrFunctionCall);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaFindCurrentFrameFunctionCall
// Description: Search the current frame function calls list using a function
//              call sub-string.
// Arguments:   contextId - The id of the context who's log will be searched.
//              searchDirection - The search direction.
//              searchStartIndex - The index of the function call from which the
//                                 search will begin.
//              searchedString - The sub-string to which this function will search for.
//                               (Each queried function call is translated into string using
//                                apFunctionCall::asString(), then the sub-string is searched
//                                in the function call string)
//              isCaseSensitiveSearch - true to perform case sensitive search.
//              foundIndex - The index of the found function call, or -1 if the search
//                           didn't find any function call that contains the input search
//                           string.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaFindCurrentFrameFunctionCall(const apContextID& contextID, apSearchDirection searchDirection,
                                                      int searchStartIndex, const gtString& searchedString,
                                                      bool isCaseSensitiveSearch, int& foundIndex)
{
    bool rc = false;

    // Arguments check:
    if (contextID.isValid())
    {
        // Get the API connection type for this context:
        apAPIConnectionType apiConnectionType;
        apAPIFunctionId functionId = gaFindMultipleAPIsFunctionID(GA_FID_gaFindCurrentFrameFunctionCall, contextID._contextType, apiConnectionType);

        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apiConnectionType))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)functionId;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextID._contextId;

            // Send the search direction
            spyConnectionSocket << (gtInt32)searchDirection;

            // Send the search start index:
            spyConnectionSocket << (gtInt32)searchStartIndex;

            // Send the search string:
            spyConnectionSocket << searchedString;

            // Send case sensitive:
            spyConnectionSocket << isCaseSensitiveSearch;

            // Perform after API call actions:
            pdProcessDebugger::instance().afterAPICallIssued();

            // Receive success value:
            spyConnectionSocket >> rc;

            if (rc)
            {
                // Receive the found index:
                spyConnectionSocket >> foundIndex;
            }
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaClearFunctionCallsStatistics
// Description: Clear the function calls statistics
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaClearFunctionCallsStatistics()
{
    bool retVal = false;

    bool isConnectionActive = false;

    // Verify that the API is active (for OpenGL API):
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaClearFunctionCallsStatistics;

        // Receive success value:
        spyConnectionSocket >> retVal;

        isConnectionActive = true;
    }

    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaClearOpenCLFunctionCallsStatistics;

        // Receive success value:
        spyConnectionSocket >> retVal;

        isConnectionActive = true;
    }

    if (!isConnectionActive)
    {
        retVal = !(gaDebuggedProcessExists());
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCurrentFrameFunctionCallDeprecationDetails
// Description: Returns the details of a function call deprecation, made in a
//              given context at its current rendering frame.
// Arguments:   contextId - The id of the context that this function queries.
//              callIndex - The call index (in this context current frame).
//              aptrFunctionCall - The returned function call details.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCurrentFrameFunctionCallDeprecationDetails(int contextId, int callIndex, apFunctionDeprecation& functionDeprecationDetails)
{
    bool rc = false;

    // Arguments check:
    if (contextId >= 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetCurrentFrameFunctionCallDeprecationDetails;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Send the call index:
            spyConnectionSocket << (gtInt32)callIndex;

            // Receive success value:
            spyConnectionSocket >> rc;

            if (rc)
            {
                // Read the function call deprecation details from the channel:
                rc = functionDeprecationDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetLastFrameFunctionCallsStatistics
// Description: Retrieves the calls statistics of the last rendered frame.
//              (amount of time that each function was called, etc)
// Arguments:   contextId - The queried context.
//              apStatistics* pStatistics - a statistics object. Statistics object
//              should already be allocated
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/1/2006
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCurrentStatistics(const apContextID& contextID, apStatistics* pStatistics)
{
    bool retVal = false;

    // Arguments check:
    if (contextID.isValid() && (pStatistics != NULL))
    {
        // Get the API connection type for this context:
        apAPIConnectionType apiConnectionType;
        apAPIFunctionId functionId = gaFindMultipleAPIsFunctionID(GA_FID_gaGetCurrentStatistics, contextID._contextType, apiConnectionType);

        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apiConnectionType))
            // Verify that the API is active:
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)functionId;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextID._contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                GT_IF_WITH_ASSERT(pStatistics != NULL)
                {
                    retVal = pStatistics->readSelfFromChannel(spyConnectionSocket);
                }
            }
        }
        else
        {
            retVal = !(gaDebuggedProcessExists());

            if (retVal)
            {
                pStatistics->clearFunctionCallsStatistics();
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsInOpenGLBeginEndBlock
// Description: Inputs a context id and returns true iff the context is currently
//              within a glBegin-glEnd block.
// Author:      Yaki Tebeka
// Date:        4/7/2006
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsInOpenGLBeginEndBlock(int contextId)
{
    bool retVal = false;

    // Arguments check:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaIsInOpenGLBeginEndBlock;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetRenderPrimitivesStatistics
// Description: Retrieves the render primitives statistics
// Arguments:   contextId - The queried context.
//              apRenderPrimitivesStatistics& renderPrimitivesStatistics
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetRenderPrimitivesStatistics(int contextId, apRenderPrimitivesStatistics& renderPrimitivesStatistics)
{
    bool retVal = false;

    // Arguments check:
    if (contextId >= 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetRenderPrimitivesStatistics;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                retVal = renderPrimitivesStatistics.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLHandleObject
// Description: Return an OpenCL handle representing object
// Arguments: void* openCLHandlePtr
//            apCLObjectID& clObjectIdDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLHandleObjectDetails(oaCLHandle openCLHandlePtr, apCLObjectID& clObjectIdDetails)
{
    bool retVal = false;

    gaPersistentDataManager& thePersistentDataMgr = gaPersistentDataManager::instance();

    // Check if this ID was cached:
    retVal = thePersistentDataMgr.getCacheOpenCLObjectID(openCLHandlePtr, clObjectIdDetails);

    if (!retVal)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
        {
            // Get the OpenCL Spy connecting socket:
            osSocket& openCLSpyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            openCLSpyConnectionSocket << (gtInt32)GA_FID_gaGetOpenCLHandleObjectDetails;

            // Send the search direction
            openCLSpyConnectionSocket << (gtUInt64)openCLHandlePtr;

            // Perform after API call actions:
            pdProcessDebugger::instance().afterAPICallIssued();

            // Receive success value:
            openCLSpyConnectionSocket >> retVal;

            if (retVal)
            {
                // Read the object details:
                clObjectIdDetails.readSelfFromChannel(openCLSpyConnectionSocket);
            }
        }

        // If we succeeded, add this ID to the cache:
        if (retVal)
        {
            thePersistentDataMgr.cacheOpenCLObjectID(openCLHandlePtr, clObjectIdDetails);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLContexts
// Description: Returns the amount of OpenCL contexts in the current debugged application
// Arguments:   amountOfContexts - The amount of OpenCL contexts
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLContexts(int& amountOfContexts)
{
    bool retVal = false;

    amountOfContexts = 0;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLContexts;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfRenderContextsAsInt32 = 0;
            openCLSpyAPISocket >> amountOfRenderContextsAsInt32;
            amountOfContexts = (int)amountOfRenderContextsAsInt32;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLProgramObjects
// Description: Returns the amount of OpenCL for 'contextId' context
// Arguments:   contextId - OpenCL context id
//              amountOfPrograms - The amount of OpenCL program created at this context
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLProgramObjects(int contextId, int& amountOfPrograms)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Update the context data snapshot (if needed):
        apContextID id(AP_OPENCL_CONTEXT, contextId);
        stat_thePersistentDataMgr.updateContextDataSnapshot(id);

        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLProgramObjects;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfProgramsAsInt32 = 0;
            openCLSpyAPISocket >> amountOfProgramsAsInt32;
            amountOfPrograms = (int)amountOfProgramsAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLProgramObjectDetails
// Description: Returns the amount of OpenCL programs for 'contextId' context
// Arguments:   contextId - OpenCL context id
//              int programIndex - the program index within the context
//              programDetails - the program object details
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLProgramObjectDetails(int contextId, int programIndex, apCLProgram& programDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Update the context data snapshot (if needed):
        apContextID id(AP_OPENCL_CONTEXT, contextId);
        stat_thePersistentDataMgr.updateContextDataSnapshot(id);

        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLProgramObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the program index:
        openCLSpyAPISocket << (gtInt32)programIndex;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = programDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetOpenCLProgramCode
// Description: Sets a program's source code
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetOpenCLProgramCode(oaCLProgramHandle programHandle, const osFilePath& newSourcePath)
{
    // TO_DO: handle remote debugging!
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the spies connecting socket:
        osSocket& spiesAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        spiesAPISocket << (gtInt32)GA_FID_gaSetOpenCLProgramCode;

        // Write the program handle:
        spiesAPISocket << (gtUInt64)programHandle;

        // Write the file path:
        newSourcePath.writeSelfIntoChannel(spiesAPISocket);

        // Receive success value:
        spiesAPISocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaBuildOpenCLProgram
// Description: Builds an OpenCL program and re-links it all its kernels in our
//              handle mapping.
// Arguments:   programHandle - the requested program (external) handle.
// Arguments:   pFailedProgramData - will get failure data if the build fails
//              and we know why. It is the caller's responsibility to release
//              this if allocated.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaBuildOpenCLProgram(oaCLProgramHandle programHandle, apCLProgram*& pFailedProgramData)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the spies connecting socket:
        osSocket& spiesAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        spiesAPISocket << (gtInt32)GA_FID_gaBuildOpenCLProgram;

        // Write the program handle:
        spiesAPISocket << (gtUInt64)programHandle;

        // Receive success value:
        spiesAPISocket >> retVal;

        // Receive whether we got failure data:
        bool isFailedProgramDataAvailable = false;
        spiesAPISocket >> isFailedProgramDataAvailable;

        // If failure data is available, create an object and read it:
        if (isFailedProgramDataAvailable)
        {
            pFailedProgramData = new apCLProgram(OA_CL_NULL_HANDLE);

            pFailedProgramData->readSelfFromChannel(spiesAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLProgramHandleFromSourceFilePath
// Description: If sourceFilePath is the source code for a program, returns its
//              handle. Otherwise, returns OA_CL_NULL_HANDLE.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLProgramHandleFromSourceFilePath(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath, oaCLProgramHandle& programHandle)
{
    // TO_DO: handle path readback!
    bool retVal = false;
    programHandle = OA_CL_NULL_HANDLE;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the spies connecting socket:
        osSocket& spiesAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        spiesAPISocket << (gtInt32)GA_FID_gaGetOpenCLProgramHandleFromSourceFilePath;

        // Write the file path:
        sourceFilePath.writeSelfIntoChannel(spiesAPISocket);

        // Receive success value:
        spiesAPISocket >> retVal;

        // Receive the handle:
        gtUInt64 programHandleAsUInt64 = 0;
        spiesAPISocket >> programHandleAsUInt64;
        programHandle = (oaCLProgramHandle)programHandleAsUInt64;

        // Get the new temporary source code file path:
        newTempSourceFilePath.readSelfFromChannel(spiesAPISocket);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetKernelSourceFilePath
// Description: Transfer list of cl program files in the startup project
// Arguments:   vector of file paths to .cl programs
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        20/4/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetKernelSourceFilePath(gtVector<osFilePath>& programsFilePath)
{
    bool retVal = true;

    // Set the spy kernel source code file paths:
    retVal = stat_thePersistentDataMgr.setKernelSourceCodePath(programsFilePath);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCodeLocationFromKernelSourceBreakpoint
// Description: Translate the information in breakpoint into a source location (in the program's source)
// Author:      Uri Shomroni
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool gaCodeLocationFromKernelSourceBreakpoint(const apKernelSourceCodeBreakpoint& breakpoint, osFilePath& sourceFilePath, int& lineNum)
{
    bool retVal = false;

    // Get the program ID:
    apCLObjectID programObjectId;
    bool rcHand = gaGetOpenCLHandleObjectDetails(breakpoint.programHandle(), programObjectId);
    GT_IF_WITH_ASSERT(rcHand && (programObjectId._objectType == OS_TOBJ_ID_CL_PROGRAM))
    {
        // Get the program details:
        apCLProgram programDetails(OA_CL_NULL_HANDLE);
        bool rcProg = gaGetOpenCLProgramObjectDetails(programObjectId._contextId, programObjectId._objectId, programDetails);
        GT_IF_WITH_ASSERT(rcProg)
        {
            // Return the parameters only if we have both:
            sourceFilePath = programDetails.sourceCodeFilePath();
            gaRemoteToLocalFile(sourceFilePath, true);
            lineNum = breakpoint.lineNumber();
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLKernelObjectDetails
// Description: Return kernel object details
// Arguments: cl_program programHandle
//            cl_kernel kernelHandle
//            apCLKernel& kernelDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLKernelObjectDetails(int contextId, oaCLKernelHandle kernelHandle, apCLKernel& kernelDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Update the context data snapshot (if needed):
        apContextID id(AP_OPENCL_CONTEXT, contextId);
        stat_thePersistentDataMgr.updateContextDataSnapshot(id);

        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLKernelObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the kernel handle:
        openCLSpyAPISocket << (gtUInt64)kernelHandle;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = kernelDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingLocation
// Description: Gets the current kernel debugging location.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingLocation(oaCLProgramHandle& debuggedProgramHandle, int& currentLineNumber)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingLocation;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtUInt64 debuggedProgramHandleAsUInt64 = 0;
            openCLSpyAPISocket >> debuggedProgramHandleAsUInt64;
            debuggedProgramHandle = (oaCLProgramHandle)debuggedProgramHandleAsUInt64;
            gtInt32 currentLineNumberAsInt32 = -1;
            openCLSpyAPISocket >> currentLineNumberAsInt32;
            currentLineNumber = (int)currentLineNumberAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCurrentlyDebuggedKernelDetails
// Description: If we are during kernel debugging, returns the details of the
//              currently debugged kernel
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/12/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCurrentlyDebuggedKernelDetails(apCLKernel& kernelDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetCurrentlyDebuggedKernelDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = kernelDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCurrentlyDebuggedKernelCallStack
// Description: Gets the debug stack of the currently debugged kernel.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCurrentlyDebuggedKernelCallStack(osCallStack& kernelStack)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetCurrentlyDebuggedKernelCallStack;

        // Send the parameters:
        gaPersistentDataManager& thePersistentDataManager = gaPersistentDataManager::instance();
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(0);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(1);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(2);

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = kernelStack.readSelfFromChannel(openCLSpyAPISocket);

            // Localize the file paths in the call stack as needed:
            if (retVal)
            {
                retVal = gaRemoteToLocalCallStack(kernelStack);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetKernelDebuggingCommand
// Description: Sets the debugging command for the kernel debugging API.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetKernelDebuggingCommand(apKernelDebuggingCommand command)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaSetKernelDebuggingCommand;

        // Send the parameter:
        openCLSpyAPISocket << (gtUInt32)command;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaKernelDebuggingGlobalWorkOffset
// Description: Sets dimension to the work offset for the coordinate-th coordinate.
//              If the work dimension is too low to include it, returns 0.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/3/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingGlobalWorkOffset(int coordinate, int& dimension)
{
    int dim = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkOffset(coordinate);

    bool retVal = (dim > -1);

    if (retVal)
    {
        dimension = dim;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaKernelDebuggingGlobalWorkOffset
// Description: Get the work offset for each of the x, y, z coordinates
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/3/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingGlobalWorkOffset(int& xOffset, int& yOffset, int& zOffset)
{
    xOffset = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkOffset(0);
    yOffset = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkOffset(1);
    zOffset = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkOffset(2);

    // Make sure values are positive:
    bool retVal = (xOffset >= 0) && (yOffset >= 0) && (zOffset >= 0);

    if (xOffset < 0) { xOffset = 0; }

    if (yOffset < 0) { yOffset = 0; }

    if (zOffset < 0) { zOffset = 0; }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaKernelDebuggingGlobalWorkSize
// Description: Sets dimension to the work size for the coordinate-th coordinate.
//              If the work dimension is too low to include it, returns 0.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/1/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingGlobalWorkSize(int coordinate, int& dimension)
{
    int dim = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkSize(coordinate);

    bool retVal = (dim > -1);

    if (retVal)
    {
        dimension = dim;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingGlobalWorkSize
// Description: Return the global work size for each of the coordinates
// Arguments:   int& xDimension
//              int& yDimension
//              int& zDimension
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingGlobalWorkSize(int& xDimension, int& yDimension, int& zDimension)
{
    // Get the X global work size:
    xDimension = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkSize(0);

    // Get the Y global work size:
    yDimension = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkSize(1);

    // Get the Z global work size:
    zDimension = stat_thePersistentDataMgr.kernelDebuggingGlobalWorkSize(2);

    bool retVal = ((xDimension >= 0) && (yDimension >= 0) && (zDimension >= 0));

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaKernelDebuggingLocalWorkSize
// Description: Sets dimension to the work group size for the coordinate-th coordinate.
//              If the work dimension is too low to include it, returns 0.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/3/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingLocalWorkSize(int coordinate, int& dimension)
{
    int dim = stat_thePersistentDataMgr.kernelDebuggingLocalWorkSize(coordinate);

    bool retVal = (dim > -1);

    if (retVal)
    {
        dimension = dim;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingLocalWorkSize
// Description: Get the kernel debugging local size for each of the coordinates
// Arguments:   int& xDimension - x coordinate size
//              int& yDimension - y coordinate size
//              int& zDimension - z coordinate size
//              int& amountOfDimensions - the kernel dimensions amount
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingLocalWorkSize(int& xDimension, int& yDimension, int& zDimension, int& amountOfDimensions)
{
    bool retVal = false;

    // Reset the amount of dimensions:
    amountOfDimensions = 0;

    // Get each of the dimensions:
    xDimension = stat_thePersistentDataMgr.kernelDebuggingLocalWorkSize(0);
    yDimension = stat_thePersistentDataMgr.kernelDebuggingLocalWorkSize(1);
    zDimension = stat_thePersistentDataMgr.kernelDebuggingLocalWorkSize(2);
    retVal = ((xDimension >= 0) && (yDimension >= 0) && (zDimension >= 0));

    if (xDimension < 0) { xDimension = 0; }

    if (yDimension < 0) { yDimension = 0; }

    if (zDimension < 0) { zDimension = 0; }

    // Set the amount of dimensions:
    if (xDimension > 0) { amountOfDimensions++; }

    if (yDimension > 0) { amountOfDimensions++; }

    if (zDimension > 0) { amountOfDimensions++; }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetKernelDebuggingCurrentWorkItemCoordinate
// Description: Sets the position in the coordinate-th dimension of the current work
//              item to value. Will fail if the value is out of range or if the index
//              is out of range.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/1/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int value)
{
    bool retVal = false;

    // Get the current work item value:
    int previousValue = 0;
    bool rc = gaGetKernelDebuggingCurrentWorkItemCoordinate(coordinate, previousValue);
    GT_IF_WITH_ASSERT(rc)
    {
        retVal = true;

        if (previousValue != value)
        {
            // Set the new value:
            retVal = stat_thePersistentDataMgr.setKernelDebuggingCurrentWorkItemCoordinate(coordinate, value);

            // Register a changed work item event:
            apKernelWorkItemChangedEvent workItemEvent(coordinate, value);
            apEventsHandler::instance().registerPendingDebugEvent(workItemEvent);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingCurrentWorkItemCoordinate
// Description: Gets the value of the coordinate-th dimension of the current work item
//              position (0 = x, 1 = y, 2 = z).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/1/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int& value)
{
    int val = stat_thePersistentDataMgr.kernelDebuggingCurrentWorkItemCoordinate(coordinate);

    bool retVal = (val > -1);

    if (retVal)
    {
        value = val;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingCurrentWorkItem
// Description: Gets the value of the current work item
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingCurrentWorkItem(int& xValue, int& yValue, int& zValue)
{
    bool retVal = true;
    xValue = stat_thePersistentDataMgr.kernelDebuggingCurrentWorkItemCoordinate(0);
    yValue = stat_thePersistentDataMgr.kernelDebuggingCurrentWorkItemCoordinate(1);
    zValue = stat_thePersistentDataMgr.kernelDebuggingCurrentWorkItemCoordinate(2);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsInKernelDebugging
// Description: Return true iff the application is currently within a kernel
//              debugging session
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/3/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsInKernelDebugging()
{
    bool retVal  = stat_thePersistentDataMgr.isInKernelDebugging();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetKernelSteppingWorkItem
// Description: Sets a work item condition for the next step command. The stepping
//              will stop only if the selected work item is hit.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/2/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetKernelSteppingWorkItem(const int coordinate[3])
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaSetKernelSteppingWorkItem;

        // Send the parameters:
        openCLSpyAPISocket << (gtInt32)coordinate[0];
        openCLSpyAPISocket << (gtInt32)coordinate[1];
        openCLSpyAPISocket << (gtInt32)coordinate[2];

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateKernelSteppingWorkItemToCurrentCoordinate
// Description: Updates the stepping work item to the current work item by using
//              gaGetKernelDebuggingCurrentWorkItemCoordinate and gaSetKernelSteppingWorkItem
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/2/2012
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateKernelSteppingWorkItemToCurrentCoordinate()
{
    // Get the current work item:
    int stepToWorkItem[3] = { -1, -1, -1};

    // if (shouldStepByWorkItem)
    {
        gaGetKernelDebuggingCurrentWorkItemCoordinate(0, stepToWorkItem[0]);
        gaGetKernelDebuggingCurrentWorkItemCoordinate(1, stepToWorkItem[1]);
        gaGetKernelDebuggingCurrentWorkItemCoordinate(2, stepToWorkItem[2]);
    }

    bool retVal = gaSetKernelSteppingWorkItem(stepToWorkItem);
    GT_ASSERT(retVal);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsWorkItemValid
// Description: Returns true iff kernel debugging is on and the given work item
//              is valid in the execution mask.
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsWorkItemValid(const int coordinate[3])
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaIsWorkItemValid;

        // Send the parameters:
        openCLSpyAPISocket << (gtInt32)coordinate[0];
        openCLSpyAPISocket << (gtInt32)coordinate[1];
        openCLSpyAPISocket << (gtInt32)coordinate[2];

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetFirstValidWorkItem
// Description: Returns the index of the first work item that is valid in the
//              execution mask. If no such item exists, returns {-1, -1, -1}
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetFirstValidWorkItem(int wavefrontIndex, int coordinate[3])
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetFirstValidWorkItem;

        // Send the parameter:
        openCLSpyAPISocket << (gtInt32)wavefrontIndex;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        // Receive parameters:
        if (retVal)
        {
            gtInt32 coordinateXAsInt32 = 0;
            openCLSpyAPISocket >> coordinateXAsInt32;
            coordinate[0] = (int)coordinateXAsInt32;
            gtInt32 coordinateYAsInt32 = 0;
            openCLSpyAPISocket >> coordinateYAsInt32;
            coordinate[1] = (int)coordinateYAsInt32;
            gtInt32 coordinateZAsInt32 = 0;
            openCLSpyAPISocket >> coordinateZAsInt32;
            coordinate[2] = (int)coordinateZAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCanGetKernelVariableValue
// Description: Returns true iff a variable of the given name exists in the current
//              debugging scope
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaCanGetKernelVariableValue(const gtString& variableName)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaCanGetKernelVariableValue;

        // Send the variable name:
        openCLSpyAPISocket << variableName;

        // Send the current work item coordinate:
        gaPersistentDataManager& thePersistentDataManager = gaPersistentDataManager::instance();
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(0);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(1);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(2);

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingVariableValueString
// Description: Gets a string value of the kernel variable named variableName
//              at the work item index specified.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/1/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingVariableValueString(const gtString& variableName, const int workItem[3], gtString& variableValue, gtString& variableValueHex, gtString& variableType)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingVariableValueString;

        // Send the variable name:
        openCLSpyAPISocket << variableName;

        // Send the work item coordinates:
        openCLSpyAPISocket << (gtInt32)workItem[0];
        openCLSpyAPISocket << (gtInt32)workItem[1];
        openCLSpyAPISocket << (gtInt32)workItem[2];

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;

        // Read the values string:
        if (retVal)
        {
            openCLSpyAPISocket >> variableValue;
            openCLSpyAPISocket >> variableValueHex;
            openCLSpyAPISocket >> variableType;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingVariableMembers
// Description: Gets the list of immediate children for the variable - its memebers
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingVariableMembers(const gtString& variableName, gtVector<gtString>& memberNames)
{
    bool retVal = false;
    memberNames.clear();

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingVariableMembers;

        // Send the variable name:
        openCLSpyAPISocket << variableName;

        // Send the current work item coordinate:
        gaPersistentDataManager& thePersistentDataManager = gaPersistentDataManager::instance();
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(0);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(1);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(2);

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;

        // Read the member names:
        if (retVal)
        {
            // Get the vector size:
            gtInt32 numberOfMembers = -1;
            openCLSpyAPISocket >> numberOfMembers;
            gtString currentMember;

            for (gtInt32 i = 0; i < numberOfMembers; i++)
            {
                // Get the values:
                openCLSpyAPISocket >> currentMember;
                memberNames.push_back(currentMember);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingAvailableVariables
// Description: Gets a list of the names of all variables currently available in
//              the current scope
// Arguments:   getLeaves - false = get the names of the base variables
//                          true  = get the names of the lowest-level members
//              stackFrameDepth - 0+ = get the variables from the given stack level and global scopes
//                              - -1 = get all variables from all levels
//                              - -2- = get only global scope variables
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingAvailableVariables(gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth)
{
    bool retVal = false;

    variableNames.clear();

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingAvailableVariables;

        // Send the parameters:
        gaPersistentDataManager& thePersistentDataManager = gaPersistentDataManager::instance();
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(0);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(1);
        openCLSpyAPISocket << (gtInt32)thePersistentDataManager.kernelDebuggingCurrentWorkItemCoordinate(2);
        openCLSpyAPISocket << getLeaves;
        openCLSpyAPISocket << (gtInt32)stackFrameDepth;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the amount of variables:
            gtUInt32 amountOfVariables = 0;
            openCLSpyAPISocket >> amountOfVariables;

            gtString currentVarName;

            for (gtUInt32 i = 0; i < amountOfVariables; i++)
            {
                // Read the current name:
                openCLSpyAPISocket >> currentVarName;

                // Add it to the vector:
                variableNames.push_back(currentVarName);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingAmountOfActiveWavefronts
// Description: Gets the number of kernel debugging wavefronts currently active
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/4/2013
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingAmountOfActiveWavefronts(int& amountOfWavefronts)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingAmountOfActiveWavefronts;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the wavefront index:
            gtInt32 amountOfWavefrontsAsInt32 = -2;
            openCLSpyAPISocket >> amountOfWavefrontsAsInt32;

            amountOfWavefronts = (int)amountOfWavefrontsAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingActiveWavefrontID
// Description: Gets the id of a kernel debugging wavefront by its current index.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        30/10/2013
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingActiveWavefrontID(int wavefrontIndex, int& wavefrontId)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingActiveWavefrontID;

        // Send the parameter:
        openCLSpyAPISocket << (gtInt32)wavefrontIndex;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the wavefront index:
            gtInt32 wavefrontIdAsInt32 = -2;
            openCLSpyAPISocket >> wavefrontIdAsInt32;

            wavefrontId = (int)wavefrontIdAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelDebuggingWavefrontIndex
// Description: Gets a Work item's wavefront index from its coordinate
// Arguments:   coordinate - the work item's coordinate
//              wavefrontIndex - the wavefront index, or -1 if the work item is not valid.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/3/2013
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelDebuggingWavefrontIndex(const int coordinate[3], int& wavefrontIndex)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelDebuggingWavefrontIndex;

        // Send the parameters:
        openCLSpyAPISocket << (gtInt32)coordinate[0];
        openCLSpyAPISocket << (gtInt32)coordinate[1];
        openCLSpyAPISocket << (gtInt32)coordinate[2];

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Read the return value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the wavefront index:
            gtInt32 wavefrontIndexAsInt32 = -2;
            openCLSpyAPISocket >> wavefrontIndexAsInt32;

            wavefrontIndex = (int)wavefrontIndexAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateKernelVariableValueRawData
// Description: Attempts to parse variable name, outputting the values for all
//              work items to a file (returned in variableRawData).
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateKernelVariableValueRawData(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableRawData)
{
    bool retVal = false;

    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaUpdateKernelVariableValueRawData;

        // Write the program handle:
        openCLSpyAPISocket << variableName;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            openCLSpyAPISocket >> variableTypeSupported;
            retVal = variableRawData.readSelfFromChannel(openCLSpyAPISocket);

            // Localize the file path as needed:
            if (retVal)
            {
                retVal = gaRemoteToLocalFile(variableRawData, false);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetKernelSourceCodeBreakpointResolution
// Description: For a kernel source breakpoint that was successfully added,
//              gets the line number where the breakpoint was resolved
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetKernelSourceCodeBreakpointResolution(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber)
{
    bool retVal = false;

    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetKernelSourceCodeBreakpointResolution;

        // Write the breakpoint details:
        openCLSpyAPISocket << (gtUInt64)programHandle;
        openCLSpyAPISocket << (gtInt32)requestedLineNumber;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the resolution details:
            gtInt32 resolvedLineNumberAsInt32 = -1;
            openCLSpyAPISocket >> resolvedLineNumberAsInt32;
            resolvedLineNumber = (int)resolvedLineNumberAsInt32;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetKernelDebuggingEnable
// Description: Sets kernel debugging enable flag in the PDM
// Arguments:   bool kernelEnable
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetKernelDebuggingEnable(bool kernelEnable)
{
    bool retVal = true;

    gaPersistentDataManager::instance().setKernelDebuggingEnable(kernelEnable);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetMultipleKernelDebugDispatchMode
// Description: Sets multiple kernel dispatch mode in the PDM
// Arguments:   apMultipleKernelDebuggingDispatchMode mode
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/7/2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode)
{
    bool retVal = true;

    gaPersistentDataManager::instance().setMultipleKernelDebugDispatchMode(mode);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLDeviceObjectDetails
// Description: Return an object containing the queried OpenCL device details.
// Arguments: deviceId - The queried device API ID.
//            deviceDetails - Will get the queried device details.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLDeviceObjectDetails(int deviceId, apCLDevice& deviceDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLDeviceObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the program handle:
        openCLSpyAPISocket << (gtInt32)deviceId;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = deviceDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLPlatformAPIID
// Description: Return a platform name (internal indexing) for the platform id
// Arguments:   platformId - The requested platform id
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLPlatformAPIID(gtUInt64 platformId, int& platformName)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLPlatformAPIID;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the program handle:
        openCLSpyAPISocket << (gtUInt64)platformId;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 platformNameAsInt32;
            openCLSpyAPISocket >> platformNameAsInt32;
            platformName = (int)platformNameAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLBufferObjects
// Description: Returns the amount of OpenCL for 'contextId' context
// Arguments:   contextId - OpenCL context id
//              amountOfBuffers - The amount of OpenCL buffer for this context
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLBufferObjects(int contextId, int& amountOfBuffers)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLBufferObjects;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfBuffersAsInt32 = 0;
            openCLSpyAPISocket >> amountOfBuffersAsInt32;
            amountOfBuffers = (int)amountOfBuffersAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLBufferObjectDetails
// Description: Returns the amount of OpenCL buffer created at this context
// Arguments:   contextId - OpenCL context id
//              int bufferIndex - the program index within the context
//              bufferDetails - the program object details
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLBufferObjectDetails(int contextId, int bufferIndex, apCLBuffer& bufferDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLBufferObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the program index:
        openCLSpyAPISocket << (gtInt32)bufferIndex;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = bufferDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLSubBufferObjectDetails
// Description: Returns an OpenCL sub buffer details
// Arguments:   contextId - OpenCL context id
//              int subBufferIndex - the program index within the context
//              subBufferDetails - the program object details
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLSubBufferObjectDetails(int contextId, int subBufferName, apCLSubBuffer& subBufferDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLSubBufferObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the program index:
        openCLSpyAPISocket << (gtInt32)subBufferName;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = subBufferDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetCLSubBufferDisplayProperties
// Description: Sets an OpenCL sub-buffer display properties
// Arguments:   int contextId
//              int subBufferId
//              oaTexelDataFormat displayFormat
//              int offset
//              gtSize_t stride
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetCLSubBufferDisplayProperties(int contextId, int subBufferId, oaTexelDataFormat displayFormat, int offset, gtSize_t stride)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(0 <= contextId)
    {
        bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        if (!isDuringProcessTermination)
        {
            // Verify that the API is active and suspended:
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the function Id:
                spyConnectionSocket << (gtInt32)GA_FID_gaSetCLSubBufferDisplayProperties;

                // Send parameters:
                spyConnectionSocket << (gtInt32)contextId;

                // Set the VBO name:
                spyConnectionSocket << (gtInt32)subBufferId;

                // Set the VBO display format:
                spyConnectionSocket << (gtInt32)displayFormat;

                // Set the VBO offset:
                spyConnectionSocket << (gtUInt64)offset;

                // Set the VBO stride:
                spyConnectionSocket << (gtUInt64)stride;

                // Receive success value:
                spyConnectionSocket >> retVal;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateOpenCLBufferRawData
// Description: Updates the OpenCL buffer data file
// Arguments: int contextId
//            gtVector<int> bufferIdsVector - buffer ids
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateOpenCLSubBufferRawData(int contextId, const gtVector<int>& subBufferIdsVector)
{
    bool retVal = false;

    // Get the amount of buffers:
    int amountOfSubBuffers = subBufferIdsVector.size();

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= contextId) && (0 < amountOfSubBuffers))
    {
        // Don't update data during process termination:
        bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        if (!isDuringProcessTermination)
        {
            // Verify that the API is active and suspended:
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the command ID:
                spyConnectionSocket << (gtInt32)GA_FID_gaUpdateOpenCLSubBufferRawData;

                // Send the context id:
                spyConnectionSocket << (gtInt64)contextId;

                // Send the parameters:
                spyConnectionSocket << (gtInt64)amountOfSubBuffers;

                for (int i = 0; i < amountOfSubBuffers; i++)
                {
                    gtInt32 subBufferId = (gtInt32)subBufferIdsVector[i];
                    spyConnectionSocket << subBufferId;
                }

                // Receive success value:
                spyConnectionSocket >> retVal;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateOpenCLBufferRawData
// Description: Updates the OpenCL buffer data file
// Arguments: int contextId
//            gtVector<int> bufferIdsVector - buffer ids
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateOpenCLBufferRawData(int contextId, const gtVector<int>& bufferIdsVector)
{
    bool retVal = false;

    // Get the amount of buffers:
    int amountOfBuffers = bufferIdsVector.size();

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= contextId) && (0 < amountOfBuffers))
    {
        // Don't update data during process termination:
        bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        if (!isDuringProcessTermination)
        {
            // Verify that the API is active and suspended:
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the command ID:
                spyConnectionSocket << (gtInt32)GA_FID_gaUpdateOpenCLBufferRawData;

                // Send the context id:
                spyConnectionSocket << (gtInt64)contextId;

                // Send the parameters:
                spyConnectionSocket << (gtInt64)amountOfBuffers;

                for (int i = 0; i < amountOfBuffers; i++)
                {
                    gtInt32 bufferId = (gtInt32)bufferIdsVector[i];
                    spyConnectionSocket << bufferId;
                }

                // Receive success value:
                spyConnectionSocket >> retVal;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetCLBufferDisplayProperties
// Description: Set a CL buffer display properties (data format, stride, and offset)
// Arguments: int contextId
//            int bufferId
//            oaTexelDataFormat displayFormat
//            int displayOffset
//            gtSize_t displayStride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetCLBufferDisplayProperties(int contextId, int bufferId, oaTexelDataFormat displayFormat, int offset, gtSize_t stride)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(0 <= contextId)
    {
        bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        if (!isDuringProcessTermination)
        {
            // Verify that the API is active and suspended:
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the function Id:
                spyConnectionSocket << (gtInt32)GA_FID_gaSetCLBufferDisplayProperties;

                // Send parameters:
                spyConnectionSocket << (gtInt32)contextId;

                // Set the VBO name:
                spyConnectionSocket << (gtInt32)bufferId;

                // Set the VBO display format:
                spyConnectionSocket << (gtInt32)displayFormat;

                // Set the VBO offset:
                spyConnectionSocket << (gtUInt64)offset;

                // Set the VBO stride:
                spyConnectionSocket << (gtUInt64)stride;

                // Receive success value:
                spyConnectionSocket >> retVal;
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLimageObjects
// Description: Returns the amount of OpenCL image for 'contextId' context
// Arguments:   contextId - OpenCL context id
//              amountOfBuffers - The amount of OpenCL images for this context
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLImageObjects(int contextId, int& amountOfImages)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLImageObjects;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfImagesAsInt32 = 0;
            openCLSpyAPISocket >> amountOfImagesAsInt32;
            amountOfImages = (int)amountOfImagesAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLImageObjectDetails
// Description: Returns the amount of OpenCL buffer created at this context
// Arguments:   contextId - OpenCL context id
//              int textureIndex - the image index within the context
//              textureDetails - the program object details
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLImageObjectDetails(int contextId, int imageIndex, apCLImage& imageDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLImageObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the program index:
        openCLSpyAPISocket << (gtInt32)imageIndex;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = imageDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdateOpenCLImageRawData
// Description: Updates the OpenCL texture data file
// Arguments: int contextId
//            gtVector<int> texturesIdsVector - texture ids
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdateOpenCLImageRawData(int contextId, const gtVector<int>& imagesIdsVector)
{
    bool retVal = false;

    // Get the amount of images:
    int amountOfImages = imagesIdsVector.size();

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= contextId) && (0 < amountOfImages))
    {
        // Don't update data during process termination:
        bool isDuringProcessTermination = stat_thePersistentDataMgr.isDuringDebuggedProcessTermination();

        if (!isDuringProcessTermination)
        {
            // Verify that the API is active and suspended:
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Send the command ID:
                spyConnectionSocket << (gtInt32)GA_FID_gaUpdateOpenCLImageRawData;

                // Send the context id:
                spyConnectionSocket << (gtInt64)contextId;

                // Send the parameters:
                spyConnectionSocket << (gtInt64)amountOfImages;

                for (int i = 0; i < amountOfImages; i++)
                {
                    gtInt32 imageId = (gtInt32)imagesIdsVector[i];
                    spyConnectionSocket << imageId;
                }

                // Receive success value:
                spyConnectionSocket >> retVal;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLPipeObjects
// Description: Returns the amount of OpenCL for 'contextId' context
// Arguments:   contextId - OpenCL context id
//              amountOfPipes - The amount of OpenCL pipes for this context
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLPipeObjects(int contextId, int& amountOfPipes)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLPipeObjects;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            gtInt32 amountOfPipesAsInt32 = 0;
            openCLSpyAPISocket >> amountOfPipesAsInt32;
            amountOfPipes = (int)amountOfPipesAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLPipeObjectDetails
// Description: Returns the amount of OpenCL pipe created at this context
// Arguments:   contextId - OpenCL context id
//              int pipeIndex - the pipe index within the context
//              pipeDetails - the pipe object details
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLPipeObjectDetails(int contextId, int pipeIndex, apCLPipe& pipeDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLPipeObjectDetails;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Write the context id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the pipe index:
        openCLSpyAPISocket << (gtInt32)pipeIndex;

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Receive parameters:
            retVal = pipeDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfCommandQueues
// Description: Gets the amount of command queues in a given OpenCL context
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfCommandQueues(int contextId, int& amountOfQueues)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfCommandQueues;

        // Write the Context Id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the return value:
            gtInt32 amountOfQueuesAsInt32 = -1;
            openCLSpyAPISocket >> amountOfQueuesAsInt32;
            amountOfQueues = (int)amountOfQueuesAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCommandQueueDetails
// Description: Get the object representing a command queue by its index:
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCommandQueueDetails(int contextId, int queueIndex, apCLCommandQueue& commandQueueDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // If we are not suspended, use the last snapshot we got:
        if (gaIsDebuggedProcessSuspended())
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENCL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);
        }

        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetCommandQueueDetails;

        // Write the Context Id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the queue Index:
        openCLSpyAPISocket << (gtInt32)queueIndex;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            commandQueueDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfCommandsInQueue
// Description: Get the number of commands logged in a queue
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfCommandsInQueue(oaCLCommandQueueHandle hQueue, int& amountOfCommands)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfCommandsInQueue;

        // Send the queue handle:
        openCLSpyAPISocket << (gtUInt64)hQueue;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            gtInt32 amountOfCommandsAsInt32 = -1;
            openCLSpyAPISocket >> amountOfCommandsAsInt32;
            amountOfCommands = (int)amountOfCommandsAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfEventsInQueue
// Description: Get the number of events logged in a queue
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/2/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfEventsInQueue(oaCLCommandQueueHandle hQueue, int& amountOfEvents)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfEventsInQueue;

        // Send the queue handle:
        openCLSpyAPISocket << (gtUInt64)hQueue;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            gtInt32 amountOfEventsAsInt32 = -1;
            openCLSpyAPISocket >> amountOfEventsAsInt32;
            amountOfEvents = (int)amountOfEventsAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetEnqueuedCommandDetails
// Description: Gets the object representing a specific command in a queue
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetEnqueuedCommandDetails(oaCLCommandQueueHandle hQueue, int commandIndex, gtAutoPtr<apCLEnqueuedCommand>& aptrCommand)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetEnqueuedCommandDetails;

        // Send the queue handle:
        openCLSpyAPISocket << (gtUInt64)hQueue;

        // Send the command index:
        openCLSpyAPISocket << (gtInt32)commandIndex;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            gtAutoPtr<osTransferableObject> aptrReadTransferableObject;
            openCLSpyAPISocket >> aptrReadTransferableObject;

            // Make sure we got an OpenCL enqueued command:
            retVal = aptrReadTransferableObject->isCLEnqueuedCommandObject();

            if (retVal)
            {
                aptrCommand = (apCLEnqueuedCommand*)(aptrReadTransferableObject.releasePointedObjectOwnership());
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLSamplers
// Description: Gets the amount of samplers in a given OpenCL context
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/3/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLSamplers(int contextId, int& amountOfSamplers)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLSamplers;

        // Write the Context Id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the return value:
            gtInt32 amountOfSamplersAsInt32 = -1;
            openCLSpyAPISocket >> amountOfSamplersAsInt32;
            amountOfSamplers = (int)amountOfSamplersAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLSamplerObjectDetails
// Description: Get the object representing an OpenCL sampler by its index.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/3/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLSamplerObjectDetails(int contextId, int samplerIndex, apCLSampler& samplerDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // If we are not suspended, use the last snapshot we got:
        if (gaIsDebuggedProcessSuspended())
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENCL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);
        }

        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLSamplerObjectDetails;

        // Write the Context Id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the sampler Index:
        openCLSpyAPISocket << (gtInt32)samplerIndex;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            samplerDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfOpenCLEvents
// Description: Gets the amount of events in a given OpenCL context
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfOpenCLEvents(int contextId, int& amountOfEvents)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetAmountOfOpenCLEvents;

        // Write the Context Id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            // Get the return value:
            gtInt32 amountOfEventsAsInt32 = -1;
            openCLSpyAPISocket >> amountOfEventsAsInt32;
            amountOfEvents = (int)amountOfEventsAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetOpenCLEventObjectDetails
// Description: Get the object representing an OpenCL event by its index.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetOpenCLEventObjectDetails(int contextId, int eventIndex, apCLEvent& eventDetails)
{
    bool retVal = false;

    // Verify that the OpenCL API is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // If we are not suspended, use the last snapshot we got:
        if (gaIsDebuggedProcessSuspended())
        {
            // Update the context data snapshot (if needed):
            apContextID id(AP_OPENCL_CONTEXT, contextId);
            stat_thePersistentDataMgr.updateContextDataSnapshot(id);
        }

        // Get the OpenCL Spy connecting socket:
        osSocket& openCLSpyAPISocket = gaSpiesAPISocket();

        // Send the function Id:
        openCLSpyAPISocket << (gtInt32)GA_FID_gaGetOpenCLEventObjectDetails;

        // Write the Context Id:
        openCLSpyAPISocket << (gtInt32)contextId;

        // Write the event Index:
        openCLSpyAPISocket << (gtInt32)eventIndex;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        openCLSpyAPISocket >> retVal;

        if (retVal)
        {
            eventDetails.readSelfFromChannel(openCLSpyAPISocket);
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfRegisteredAllocatedObjects
// Description: Returns the number of objects registered in the spy allocated
//              objects manager.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        12/11/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfRegisteredAllocatedObjects(unsigned int& numberOfObjects)
{
    bool retVal = false;

    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfRegisteredAllocatedObjects;

        // Get the RetVal:
        spyConnectionSocket >> retVal;

        GT_IF_WITH_ASSERT(retVal)
        {
            // Get the result:
            gtUInt32 numberOfObjectsAsUInt32 = 0;
            spyConnectionSocket >> numberOfObjectsAsUInt32;
            numberOfObjects = (unsigned int)numberOfObjectsAsUInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAllocatedObjectCreationStack
// Description: Gets the creation calls stack for the object whose allocated
//              Object Id is supplied
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAllocatedObjectCreationStack(int allocatedObjectId, osCallStack& callStack)
{
    bool retVal = false;

    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAllocatedObjectCreationStack;

        // Send the object Id:
        spyConnectionSocket << (gtInt32)allocatedObjectId;

        // Get the return value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            retVal = callStack.readSelfFromChannel(spyConnectionSocket);

            GT_IF_WITH_ASSERT(retVal)
            {
                // Get the process debugger:
                pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                theProcessDebugger.fillCallsStackDebugInfo(callStack);

                // Localize the file paths in the call stack as needed:
                retVal = gaRemoteToLocalCallStack(callStack);
            }
        }
        else
        {
            apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
            gaGetDebuggedProcessExecutionMode(currentExecMode);
            bool areCreationStacksCollected = true;
            gaGetCollectAllocatedObjectCreationCallsStacks(areCreationStacksCollected);

            // Not having creation calls stacks is normal if:
            // 1. We are in Profile Mode.
            // 2. The user chose not to collect creation calls stacks.
            GT_ASSERT((!areCreationStacksCollected) || (currentExecMode == AP_PROFILING_MODE));
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCollectAllocatedObjectsCreationCallsStacks
// Description: Sets the flag of whether object creation calls stacks are collected
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaCollectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks)
{
    bool retVal = false;

    // Set the "Collect Allocated Objects Creation Calls Stacks" mode:
    retVal = stat_thePersistentDataMgr.setAllocatedObjectsCreationCallsStacksCollection(collectCreationStacks);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetCollectAllocatedObjectCreationCallsStacks
// Description: Gets the flag of whether object creation calls stacks are collected
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCollectAllocatedObjectCreationCallsStacks(bool& collectCreationStacks)
{
    bool retVal = true;

    collectCreationStacks = stat_thePersistentDataMgr.areAllocatedObjectsCreationCallsStacksCollected();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaFindStringMarker
// Description: Search the current frame function calls list for a string marker.
//              (See GL_GREMEDY_string_marker extension for more details).
// Arguments:   contextId - The id of the context who's log will be searched.
//              searchDirection - The search direction.
//              searchStartIndex - The index of the function call from which the
//                                 search will begin.
//              foundIndex - The index of the found string marker, or -1 if the search
//                           didn't find any string marker when searching the search
//                           direction.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/2/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaFindStringMarker(int contextId, apSearchDirection searchDirection,
                                          int searchStartIndex, int& foundIndex)
{
    bool retVal = false;

    // Arguments check:
    if (contextId >= 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaFindStringMarker;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Send the search direction
            spyConnectionSocket << (gtInt32)searchDirection;

            // Send the search start index:
            spyConnectionSocket << (gtInt32)searchStartIndex;

            // Perform after API call actions:
            pdProcessDebugger::instance().afterAPICallIssued();

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive the found index:
                gtInt32 foundIndexAsInt32 = -1;
                spyConnectionSocket >> foundIndexAsInt32;
                foundIndex = (int)foundIndexAsInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetBreakpoint
// Description: Sets a debugged process breakpoint.
// Arguments:   breakpoint - The breakpoint to be set.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.setBreakpoint(breakpoint);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetKernelBreakpointProgramHandle
// Description: Set a kernel source code breakpoint program handle
// Arguments:   int breakpointIndex
//              oaCLProgramHandle programHandle
//              bool isResolved
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetKernelBreakpointProgramHandle(int breakpointIndex, oaCLProgramHandle programHandle)
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.setKernelSourceCodeBreakpointProgramHandle(breakpointIndex, programHandle);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoveBreakpoint
// Description: Removes a debugged process breakpoint
// Arguments: int breakpointId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/6/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoveBreakpoint(int breakpointId)
{
    bool retVal = false;

    // Get the breakpoint from the breakpoints list:
    gtAutoPtr<apBreakPoint> aptrBreakpoint;
    bool rc = stat_thePersistentDataMgr.getBreakpoint(breakpointId, aptrBreakpoint);
    GT_IF_WITH_ASSERT(rc)
    {
        if (aptrBreakpoint->type() == OS_TOBJ_ID_GENERIC_BREAKPOINT)
        {
            // Down cast it to apGenericBreakpoint:
            apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
            {
                // Remove the generic breakpoint:
                retVal = gaRemoveGenericBreakpoint(pGenericBreakpoint->breakpointType());
            }
        }
        else
        {
            // Remove the breakpoint:
            retVal = stat_thePersistentDataMgr.removeBreakpoint(breakpointId);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoveGenericBreakpoint
// Description: Removes a generic breakpoint
// Arguments:   breakpointType - the type of the generic breakpoint that should be removed
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/7/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoveGenericBreakpoint(apGenericBreakpointType breakpointType)
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.removeGenericBreakpoint(breakpointType);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfBreakpoints
// Description: Returns the amount of existing debugged process breakpoints.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfBreakpoints(int& amountOfBreakpoints)
{
    bool retVal = true;

    // Get the breakpoints amount:
    amountOfBreakpoints = stat_thePersistentDataMgr.amountOfBreakpoints();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetBreakpoint
// Description: Returns a debugged process breakpoint data.
// Arguments:   breakPointId - The id of the queried breakpoint.
//              breakpoint - Output variable - the breakpoint data.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetBreakpoint(int breakPointId, gtAutoPtr<apBreakPoint>& aptrBreakpoint)
{
    bool retVal = true;

    // Get the breakpoint:
    retVal = stat_thePersistentDataMgr.getBreakpoint(breakPointId, aptrBreakpoint);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetBreakpointIndex
// Description: Tries to find the index of an active breakpoint that matches
//              the supplied breakpoint in type and parameters.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/10/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetBreakpointIndex(const apBreakPoint& breakpoint, int& breakpointId)
{
    bool retVal = true;

    // Get the breakpoint:
    retVal = stat_thePersistentDataMgr.getBreakpointIndex(breakpoint, breakpointId);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoveAllBreakpoints
// Description: Removes all debugged process breakpoint.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        21/5/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoveAllBreakpoints()
{
    bool retVal = false;

    // Remove all the registered breakpoints:
    retVal = stat_thePersistentDataMgr.removeAllBreakpoints();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoveAllBreakpointsByType
// Description: Removes all debugged process breakpoint of a specific type
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        6/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoveAllBreakpointsByType(osTransferableObjectType breakpointType)
{
    bool retVal = true;

    // Remove all the registered debugged process breakpoints:
    int amountOfBreakpoint = 0;
    bool rc = gaGetAmountOfBreakpoints(amountOfBreakpoint);

    GT_IF_WITH_ASSERT(rc)
    {
        retVal = true;

        for (int nBreakpoint = amountOfBreakpoint - 1; nBreakpoint >= 0; nBreakpoint--)
        {
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            // Get the breakpoint from the breakpoints list:
            rc = stat_thePersistentDataMgr.getBreakpoint(nBreakpoint, aptrBreakpoint);
            retVal = retVal && rc;

            GT_IF_WITH_ASSERT(rc)
            {
                // Check if point type is of the type needed to be removed:
                if (aptrBreakpoint->type() == breakpointType)
                {
                    // Remove current break point and stay on same point since points are moved:
                    retVal = retVal && gaRemoveBreakpoint(nBreakpoint);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetBreakpointHitCount
// Description: Set the hit count of the requested breakpoint
// Arguments:   int breakpointIndex
//              int hitCount
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetBreakpointHitCount(int breakpointIndex, int hitCount)
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.setBreakpointHitCount(breakpointIndex, hitCount);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoveAllBreakpointsByState
// Description: Remove any breakpoint whose state matches the state parameter
// Arguments:   BreakpointState state
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoveAllBreakpointsByState(const apBreakPoint::State state)
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.removeAllBreakpointsByState(state);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaTemporarilyDisableAllBreakpoints
// Description: Set the state of all enabled breakpoints to Temporarily Disabled
// Arguments:   na
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaTemporarilyDisableAllBreakpoints()
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.temporarilyDisableAllBreakpoints();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaTemporarilyDisableAllBreakpoints
// Description: Set the state of any breakpoint which is currently 'Temporarily Disabled' to 'Enabled'
// Arguments:   na
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaEnableAllBreakpointsByState(const apBreakPoint::State state)
{
    bool retVal = false;

    // Set the breakpoint:
    retVal = stat_thePersistentDataMgr.enableAllBreakpointsByState(state);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaWasOpenGLDataRecordedInDebugSession
// Description:
//              This function return true iff openGL data recording was
//              done during this debug session;
//
//              meaning either:
//              1. Recording is "on" at the moment - we are recording data.
//              2. During this debug session, recording was "on" for some
//                 time, then we disabled it. In this case we also return
//                 true, since for some time - recording was on.
//
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        29/1/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaWasOpenGLDataRecordedInDebugSession()
{
    bool retVal = false;

    // Remove all the registered breakpoints:
    retVal = stat_thePersistentDataMgr.wasOpenGLDataRecordedInDebugSession();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetEnableAllBreakpointsStatus
// Description: Gets the enabled and checked status of the "enable / disable all
//              function breakpoints" command.
// Arguments:   bool& isEnableAllBreakpointsChecked
//            bool& isEnableAllBreakpointsEnabled
// Return Val:  bool - Success / failure.
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetEnableAllBreakpointsStatus(bool& isEnableAllBreakpointsChecked, bool& isEnableAllBreakpointsEnabled)
{
    bool retVal = false;

    // Get the amount of active breakpoints:
    int amountOfBreakpoints = 0;
    bool rc1 = gaGetAmountOfBreakpoints(amountOfBreakpoints);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
        isEnableAllBreakpointsEnabled = true;
        isEnableAllBreakpointsChecked = false;

        if (amountOfBreakpoints == 0)
        {
            // If there are no breakpoints, we want to show this as unchecked and disabled.
            isEnableAllBreakpointsEnabled = false;
        }

        // Iterate on the active breakpoints
        for (int i = 0; i < amountOfBreakpoints; i++)
        {
            // Get the current breakpoint
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
            retVal = retVal && rc2;
            GT_IF_WITH_ASSERT(rc2)
            {
                // Get the breakpoint type:
                osTransferableObjectType curentBreakpointType = aptrBreakpoint->type();

                if (curentBreakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
                {
                    // Down cast it to apMonitoredFunctionBreakPoint:
                    apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
                    GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
                    {
                        // Get the Breakpoint's status (i.e. Checked / Unchecked)
                        bool isChecked = pFunctionBreakpoint->isEnabled();
                        isEnableAllBreakpointsChecked = isEnableAllBreakpointsChecked || isChecked;
                    }
                }
            }

            if (isEnableAllBreakpointsChecked)
            {
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaBreakOnNextMonitoredFunctionCall
// Description:
//   Caused the debugged application to trigger a breakpoint event on the
//   next call a monitored function.
//   This function enables implementing "One step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaBreakOnNextMonitoredFunctionCall()
{
    bool retVal = false;

    // Set it to trigger a breakpoint event at the next call to a monitored
    // function:
    retVal = stat_thePersistentDataMgr.setBreakPointOnNextMonitoredFunctionCall();

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaBreakOnNextDrawFunctionCall
// Description:
//   Caused the debugged application to trigger a breakpoint event on the
//   next draw monitored function.
//   This function enables implementing "Draw step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        25/5/2006
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaBreakOnNextDrawFunctionCall()
{
    bool retVal = false;

    // Set it to trigger a breakpoint event at the next draw function:
    retVal = stat_thePersistentDataMgr.setBreakPointOnNextDrawFunctionCall();

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaBreakOnNextFrame
// Description:
//   Caused the debugged application to trigger a breakpoint event on the
//   next frame terminator.
//   This function enables implementing "Frame step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/7/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaBreakOnNextFrame()
{
    bool retVal = false;

    // Set it to trigger a breakpoint event at the next frame terminator:
    retVal = stat_thePersistentDataMgr.setBreakPointOnNextFrame();

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaBreakInMonitoredFunctionCall
// Description: Steps into the monitored function we are currently suspended on.
//              If this function does not support this, sets a breakpoint on the
//              next function call (like gaBreakOnNextMonitoredFunctionCall).
//              This function should be followed by a call to gaResumeDebuggedProcess().
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaBreakInMonitoredFunctionCall()
{
    bool retVal = false;

    // Set the spy to break inside this function call or at the next one:
    retVal = stat_thePersistentDataMgr.setBreakPointInMonitoredFunctionCall();

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaClearAllStepFlags
// Description: Clears all the flags set by gaBreak* in the spy.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/2/2016
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaClearAllStepFlags()
{
    bool retVal = false;

    // Set the spy to not break on the next API call(s):
    retVal = stat_thePersistentDataMgr.clearAllStepFlags();

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetGenericBreakpointStatus
// Description: Get the status (existence & enable) of generic breakpoint
// Arguments:   apGenericBreakpointType breakpointType
//              bool& doesExist - do we have a breapoint of this type
//              bool& isEnabled - is the breakpoint enabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/7/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetGenericBreakpointStatus(apGenericBreakpointType breakpointType, bool& doesExist, bool& isEnabled)
{
    bool retVal = false;

    // Set the initial values for the output parameters:
    doesExist = false;
    isEnabled = false;

    // Get the current amount of breakpoints:
    int amountOfBreakpoints = 0;
    bool rc = gaGetAmountOfBreakpoints(amountOfBreakpoints);
    GT_IF_WITH_ASSERT(rc)
    {
        retVal = true;

        // Iterate the breakpoints and search for the requested one:
        for (int i = 0; i < amountOfBreakpoints ; i++)
        {
            // Get the breakpoint from the breakpoints list:
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            rc = stat_thePersistentDataMgr.getBreakpoint(i, aptrBreakpoint);
            retVal = retVal && rc;

            GT_IF_WITH_ASSERT(rc)
            {
                // Check if point type is of the type needed to be removed:
                if (aptrBreakpoint->type() == OS_TOBJ_ID_GENERIC_BREAKPOINT)
                {
                    // Down cast the breakpoint to a generic breakpoint:
                    apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
                    GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
                    {
                        if (pGenericBreakpoint->breakpointType() == breakpointType)
                        {
                            // The breakpoint exist:
                            doesExist = true;

                            // Set the breakpoint enable status:
                            isEnabled = pGenericBreakpoint->isEnabled();
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaDeleteLogFilesWhenDebuggedProcessTerminates
// Description:
//   Manages the deletion of the debugged process log files.
//   Inputs a deleteLogFiles boolean variable.
//   - true - The log files will be deleted when the debugged process is terminated.
//   - false - The log files will remain on disk.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/1/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaDeleteLogFilesWhenDebuggedProcessTerminates(bool deleteLogFiles)
{
    // Set this flag:
    stat_thePersistentDataMgr.deleteLogFilesWhenDebuggedProcessTerminates(deleteLogFiles);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaStartMonitoredFunctionsCallsLogFileRecording
// Description:
//   Start recording the monitored function calls into the
//   monitored functions log file.
//   If this is the first call to this function during the current process debug
//   session - the log file will be initialized.
//   From the second and on call to this function (during the current process
//   debug session) - the monitored functions logging will be appended to the
//   existing log file content.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/8/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaStartMonitoredFunctionsCallsLogFileRecording()
{
    bool retVal = false;

    // Make the text log file active:
    retVal = stat_thePersistentDataMgr.setHTMLLogFileRecordingMode(true);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaStopMonitoredFunctionsCallsLogFileRecording
// Description: Stops recoding into the monitored functions log file.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/8/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaStopMonitoredFunctionsCallsLogFileRecording()
{
    bool retVal = true;

    // If a debugged process exists:
    bool rc1 = gaDebuggedProcessExists();

    if (rc1)
    {
        // Make the text log file inactive:
        retVal = stat_thePersistentDataMgr.setHTMLLogFileRecordingMode(false);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Flag that recording was done during this debug session
            bool rc2 = gaResetRecordingWasDoneFlag(true);
            GT_ASSERT(rc2);

            // After stopping the calls log recording, we suspended the debugged process.
            // This is done because we want to flush all textures images to disk
            bool rc3 = gaSuspendDebuggedProcess();
            GT_ASSERT(rc3);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsMonitoredFunctionsCallsLogFileRecordingActive
// Description: Checks if the monitored functions log file recoding is active.
// Arguments:   isActive - Output parameter. Will get true iff log file recording
//                         is active.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsMonitoredFunctionsCallsLogFileRecordingActive(bool& isActive)
{
    isActive = stat_thePersistentDataMgr.isHTMLLogFileRecordingOn();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetContextLogFilePath
// Description: Retrieves the path of a render context monitored functions calls log file.
// Arguments:   contextId - The queried context id.
//              logFileExists - will get true iff a monitored functions calls log file exist
//                              for the queried context.
//              filePath - Will get the output file path.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/3/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetContextLogFilePath(apContextID contextID, bool& logFileExists, osFilePath& filePath)
{
    bool retVal = false;
    logFileExists = false;

    // Arguments check:
    if (contextID.isValid())
    {
        // Get the right function ID according to context type, and connected APIs:
        apAPIConnectionType apiConnectionType;
        apAPIFunctionId functionId = gaFindMultipleAPIsFunctionID(GA_FID_gaGetContextLogFilePath, contextID._contextType, apiConnectionType);

        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apiConnectionType))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)functionId;

            // Send arguments:
            spyConnectionSocket << (gtInt32)contextID._contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive output parameters:
                spyConnectionSocket >> logFileExists;

                if (logFileExists)
                {
                    gtString filePathAsString;
                    spyConnectionSocket >> filePathAsString;
                    filePath = filePathAsString;

                    retVal = gaRemoteToLocalFile(filePath, false);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaFlushLogFileAfterEachFunctionCall
// Description: Set/Unset the flush after every OpenGL function call.
// Arguments:   flushAfterEachFunctionCall - true = set, false = unset..
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaFlushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall)
{
    bool retVal = false;

    // Set the "Flush after every OpenGL function call Implementation":
    retVal = stat_thePersistentDataMgr.flushLogFileAfterEachFunctionCall(flushAfterEachFunctionCall);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsLogFileFlushedAfterEachFunctionCall
// Description: Sets isFlushAfterEachFunctionCall to true iff the debugged application
//              is Flush after every OpenGL function call.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsLogFileFlushedAfterEachFunctionCall(bool& isFlushAfterEachFunctionCall)
{
    // Get the "Flush after every OpenGL function call Impl" status:
    isFlushAfterEachFunctionCall = stat_thePersistentDataMgr.isLogFileFlushedAfterEachFunctionCall();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetSlowMotionDelay
// Description:
//   Sets the "slow motion" delay - A delay that will be added to each monitored
//   function call.
//   Setting the delay to 0 turns off the slow motion mode.
//
// Arguments:   delayTimeUnits - The slow motion delay in abstract time units.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetSlowMotionDelay(int delayTimeUnits)
{
    bool retVal = false;

    // Sanity test:
    if (0 <= delayTimeUnits)
    {
        // Make the "slow motion" delay:
        retVal = stat_thePersistentDataMgr.setSlowMotionDelay(delayTimeUnits);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetSlowMotionDelay
// Description: Retrieves the "slow motion" delay.
//              A value of 0 means that "slow motion" is turned off.
// Arguments:   delayTimeUnits - Will get the slow motion delay in abstract time units.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetSlowMotionDelay(int& delayTimeUnits)
{
    // Make the "slow motion" delay:
    delayTimeUnits = stat_thePersistentDataMgr.slowMotionDelay();

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaForceOpenGLFlush
// Description:
//   Force OpenGL to flush its buffers after every OpenGL function call.
//   This enables viewing the debugged application drawing interactively.
//
// Arguments:   isOpenGLFlushForced - true - force OpenGL to flush its buffers
//                                           after every OpenGL function call
//                                    false - turn off forced flushing - let the
//                                            OpenGL implementation decide when to
//                                            flush its buffers.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaForceOpenGLFlush(bool isOpenGLFlushForced)
{
    bool retVal = false;

    // Set the "Force OpenGL flush":
    retVal = stat_thePersistentDataMgr.forceOpenGLFlush(isOpenGLFlushForced);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsOpenGLFlushForced
// Description:
//  Checks the status of the "Force OpenGL flush" mode.
//  (See gaGRApiFunctions::gaForceOpenGLFlush)
//
// Arguments:
//  isOpenGLFlushForced - Output variable - will get:
//                        true - if "Force OpenGL flush" mode is on.
//                        false - if "Force OpenGL flush" mode is off.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsOpenGLFlushForced(bool& isOpenGLFlushForced)
{
    // Get the "Force OpenGL flush" status:
    isOpenGLFlushForced = stat_thePersistentDataMgr.isOpenGLFlushForced();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetInteractiveBreakMode
// Description:
//   Turns on / off the "Interactive break" mode.
//   When in "Interactive break" mode, whenever the debugged application breaks,
//   we force the flush / SwapBuffers of the active render context to display
//   it current context to the user.
//
// Arguments: isInteractiveBreakOn - true to turn on "Interactive break" mode.
//                                   false to rurn off "Interactive break" mode.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetInteractiveBreakMode(bool isInteractiveBreakOn)
{
    bool retVal = false;

    // Set the "Interactive break" mode:
    retVal = stat_thePersistentDataMgr.setInteractiveBreakMode(isInteractiveBreakOn);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetDebuggedProcessExecutionMode
// Description: Sets the debugged process execution mode: Debug/ Profiling/ Redundant state change
// Arguments: executionMode - the new execution mode to be applied.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetDebuggedProcessExecutionMode(apExecutionMode executionMode)
{
    bool retVal = false;

    // Set the Execution mode:
    retVal = stat_thePersistentDataMgr.setDebuggedProcessExecutionMode(executionMode);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetDebuggedProcessExecutionMode
// Description: gets the current debugged process execution mode: debug / profile / analyze
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        4/8/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetDebuggedProcessExecutionMode(apExecutionMode& executionMode)
{
    bool retVal = true;

    executionMode = stat_thePersistentDataMgr.getDeubggedProcessExecutionMode();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsInteractiveBreakOn
// Description: Queries the status of the "Interactive break" mode
// Return Val: bool  - true - "Interactive break" mode is on.
//                   - false - "Interactive break" mode is off.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsInteractiveBreakOn()
{
    // Get the "Interactive break" status:
    bool isInteractiveBreakOn = stat_thePersistentDataMgr.isInteractiveBreakOn();
    return isInteractiveBreakOn;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaForceOpenGLPolygonRasterMode
// Description: Forces OpenGL to use an input polygon raster mode.
// Arguments:   rasterMode - The input polygon raster mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaForceOpenGLPolygonRasterMode(apRasterMode rasterMode)
{
    bool retVal = false;

    // Verify that we input a supported raster mode:
    if ((rasterMode == AP_RASTER_POINTS) ||
        (rasterMode == AP_RASTER_LINES) ||
        (rasterMode == AP_RASTER_FILL))
    {
        // Set the "Force Raster mode":
        retVal = stat_thePersistentDataMgr.forceOpenGLPolygonRasterMode(rasterMode);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaCancelOpenGLPolygonRasterModeForcing
// Description: Cancels OpenGL raster mode forcing that was turned on by
//              gaGRApiFunctions::gaForceOpenGLPolygonRasterMode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaCancelOpenGLPolygonRasterModeForcing()
{
    bool retVal = false;

    // Cancel the "Force Raster mode":
    retVal = stat_thePersistentDataMgr.cancelOpenGLPolygonRasterModeForcing();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsOpenGLPolygonRasterModeForced
// Description: Checks if "Force raster" mode is on.
//              (See gaGRApiFunctions::gaForceOpenGLPolygonRasterMode)
// Arguments:   isForced - Will get true iff "Force raster" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsOpenGLPolygonRasterModeForced(bool& isForced)
{
    // Check if "Force Raster mode" is on:
    isForced = stat_thePersistentDataMgr.isStubOperationForced(AP_OPENGL_FORCED_POLYGON_RASTER_MODE);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetForceOpenGLPolygonRasterMode
// Description: Get the "Forced raster" mode raster mode.
//              (See gaGRApiFunctions::gaForceOpenGLPolygonRasterMode).
// Arguments:   rasterMode - Will get the "Forced raster" mode raster mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetForceOpenGLPolygonRasterMode(apRasterMode& rasterMode)
{
    // Get the "Force Raster mode" rater mode:
    rasterMode = stat_thePersistentDataMgr.forcedOpenGLRasterMode();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSwitchToNULLOpenGLImplementation
// Description: Switched the debugged application to run on top of the NULL
//              OpenGL implementation.
// Arguments:   isNULLOpenGLImplOn - true - Run the debugged application on top of
//                                          the NULL OpenGL implementation.
//                                 - false - Run the debugged application on top of
//                                           the regular OpenGL implementation.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/2/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSwitchToNULLOpenGLImplementation(bool isNULLOpenGLImplOn)
{
    bool retVal = false;

    // Set the "Null Implementation":
    retVal = stat_thePersistentDataMgr.forceOpenGLNullDriver(isNULLOpenGLImplOn);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsUnderNULLOpenGLImplementation
// Description: Returns true iff the debugged application runs on top of
//              the NULL OpenGL implementation.
// Author:      Yaki Tebeka
// Date:        27/2/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsUnderNULLOpenGLImplementation(bool& isUnderNULLOpenGLImplementation)
{
    // Get the "NULL OpenGL Impl" status:
    isUnderNULLOpenGLImplementation = stat_thePersistentDataMgr.isOpenGLNullDriverForced();

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaForceOpenGLStub
// Description: Forces the debugged application for a single stub operation
// Arguments:   apOpenGLForcedModeType openGLStubType
//            bool isStubForced
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaForceOpenGLStub(apOpenGLForcedModeType openGLStubType, bool isStubForced)
{
    bool retVal = false;

    retVal = stat_thePersistentDataMgr.forceOpenGLStub(openGLStubType, isStubForced);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsOpenGLStubForced
// Description: Checks if a stub operation is forced
// Arguments:   apOpenGLForcedModeType openGLStubType
//            bool& isStubForced
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsOpenGLStubForced(apOpenGLForcedModeType openGLStubType, bool& isStubForced)
{
    // Get the "Stub" status:
    isStubForced = stat_thePersistentDataMgr.isStubOperationForced(openGLStubType);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsOpenCLExecutionOn
// Description: Checks if an OpenCL operations in the debugged application
// Arguments:   bool& isExecutionOn
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsOpenCLExecutionOn(apOpenCLExecutionType executionType, bool& isExecutionOn)
{
    // Get the "Is Execution On" status:
    isExecutionOn = stat_thePersistentDataMgr.isOpenCLOperationExecutionOn(executionType);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetOpenCLExecution
// Description: Cancel / Resume OpenCL operations in the debugged application
// Arguments:   apOpenCLExecutionType executionType
//            bool isExecutionOn
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetOpenCLExecution(apOpenCLExecutionType executionType, bool isExecutionOn)
{
    bool retVal = false;
    retVal = stat_thePersistentDataMgr.setOpenCLOperationExecution(executionType, isExecutionOn);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfPerformanceCounters
// Description: Returns the amount of available performance counters.
// Arguments:   countersAmount - The amount of performance counters.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/6/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfPerformanceCounters(int& countersAmount)
{
    countersAmount = 0; //stat_thePerformanceCountersMgr.amountOfCounters();
    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfPerformanceCountersByType
// Description: Returns amount of counter of the input type
// Arguments:   apCounterType counterType - requested counters' type
//              countersAmount - the amount of counters of this type found
// Return Val:  int - the counters amount
// Author:      Sigal Algranaty
// Date:        19/3/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfPerformanceCountersByType(apCounterType counterType, int& countersAmount)
{
    (void)(counterType); // unused
    // _TO_DO: implement the return value of the manager
    bool retVal = true;
    countersAmount = 0;// retVal = stat_thePerformanceCountersMgr.amountOfCountersByType(counterType, countersAmount);
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaAddOSCounter
// Description:
//   Adds an Operating System exiting counter to the available performance counters.
// Arguments: newCounterInfo - A class holding the information of the counter to be added.
//            newCounterIndex - Will get the added counter index.
//            isNewCounter - Will get true iff the counter is a new counter that does
//                           not already exist in the system.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaAddOSCounter(const apCounterInfo& newCounterInfo, int& newCounterIndex, bool& isNewCounter)
{
    (void)(newCounterInfo); // unused
    (void)(newCounterIndex); // unused
    (void)(isNewCounter); // unused
    bool retVal = false;
    // retVal = stat_thePerformanceCountersMgr.addOSCounter(newCounterInfo, newCounterIndex, isNewCounter);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoveOSCounter
// Description: Removes an Operating System exiting counter from the available
//              performance counters. The only counters that can be removed are
//              additional counters, that were added by gaGRApiFunctions::gaAddOSCounter.
// Arguments: counterIndex - the counter index, returned by gaGRApiFunctions::gaAddOSCounter.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoveOSCounter(int counterIndex)
{
    (void)(counterIndex); // unused
    bool retVal = false;
    // retVal = stat_thePerformanceCountersMgr.removeOSCounter(counterIndex);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPerformanceCounterInfo
// Description: Queries the details of a given performance counter.
// Arguments:   counterIndex - The performance counter index.
// Return Val:  const apCounterInfo* - Will get the counter information, or NULL
//                                     if the queried counter does not exist.
// Author:      Yaki Tebeka
// Date:        30/6/2005
// ---------------------------------------------------------------------------
const apCounterInfo* gaGRApiFunctions::gaGetPerformanceCounterInfo(int counterIndex)
{
    (void)(counterIndex); // unused
    const apCounterInfo* retVal = NULL; //stat_thePerformanceCountersMgr.counterInfo(counterIndex);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        bool gaGRApiFunctions::gaGetPerformanceCounterID
// Description: The function gets a counter global index, and returns a counter id,
//              according to the specific reader
// Arguments: int counterIndex - the global indexing of the counter
//            int& counterLocalIndex - output - the counter internal index
// Return Val: bool - Success/Failure
// Author:      Sigal Algranaty
// Date:        25/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPerformanceCounterLocalIndex(int counterIndex, int& counterLocalIndex)
{
    (void)(counterIndex); // unused
    (void)(counterLocalIndex); // unused
    bool retVal = false;
    //retVal = stat_thePerformanceCountersMgr.getCounterLocalIndex(counterIndex, counterLocalIndex);
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPerformanceCounterType
// Description: Returns the counter type
// Arguments: int counterIndex
//            apCounterType& counterType
// Return Val: bool - Success/Failure
// Author:      Sigal Algranaty
// Date:        10/3/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPerformanceCounterType(int counterIndex, apCounterType& counterType)
{
    (void)(counterIndex); // unused
    (void)(counterType); // unused
    bool retVal = false;
    //retVal = stat_thePerformanceCountersMgr.getCounterType(counterIndex, counterType);
    return retVal;

}
// ---------------------------------------------------------------------------
// Name:        bool gaGRApiFunctions::gaGetPerformanceCounterIndex
// Description: The function gets a counter id, and returns a global counter index
// Arguments:   apCounterID& counterId - the counter id
//              int counterIndex - output - the global indexing of the counter
// Return Val: bool - Success/Failure
// Author:      Sigal Algranaty
// Date:        25/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPerformanceCounterIndex(const apCounterID& counterId, int& counterIndex)
{
    (void)(counterId); // unused
    (void)(counterIndex); // unused
    bool retVal = false;
    //retVal = stat_thePerformanceCountersMgr.getCounterIndex(counterId, counterIndex);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaOnQueueCreatedEvent
// Description: Handling an OpenCL queue creation event
// Arguments:   const apOpenCLQueueCreatedEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void gaGRApiFunctions::gaOnQueueCreatedEvent(const apOpenCLQueueCreatedEvent& eve)
{
    (void)(eve); // unused
    // stat_thePerformanceCountersMgr.onQueueCreatedEvent(eve);
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaOnQueueDeletedEvent
// Description: Handling an OpenCL queue deletion event
// Arguments:   const apOpenCLQueueDeletedEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void gaGRApiFunctions::gaOnQueueDeletedEvent(const apOpenCLQueueDeletedEvent& eve)
{
    (void)(eve); // unused
    // stat_thePerformanceCountersMgr.onQueueDeletedEvent(eve);
}

/// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaActivatePerformanceCounter
// Description: Activate / Deactivate a given performance counter.
// Arguments: counterIndex - The id of the counter to be activate / deactivate.
//            const apQueueID& queueID - the counter queue ID
//            isCounterActive - true - make the counter active.
//                              false - deactivate the counter.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/2/2008
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaActivatePerformanceCounter(const apCounterActivationInfo& counterActivationInfo)
{
    (void)(counterActivationInfo); // unused
    bool retVal = false; // stat_thePerformanceCountersMgr.activateCounter(counterActivationInfo);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaUpdatePerformanceCountersValues
// Description: Updates the performance counter values to their current value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/6/2005
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaUpdatePerformanceCountersValues()
{
    bool retVal = false; // stat_thePerformanceCountersMgr.updateCountersValues();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPerformanceCountersValues
// Description: Retrieves the last updated value a given performance counter.
// Return Val:  const double* - A const pointer to the last updated
//                              performance counter value.
// Author:      Yaki Tebeka
// Date:        30/6/2005
// ---------------------------------------------------------------------------
double gaGRApiFunctions::gaGetPerformanceCounterValue(const apCounterID& counterId)
{
    (void)(counterId); // unused
    double retVal = 0.0; // stat_thePerformanceCountersMgr.counterValue(counterId);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsGLDebugOutputSupported
// Description: Returns true iff OpenGL debug output functionality is supported by
//              the current hardware and driver.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsGLDebugOutputSupported()
{
    // Will get true iff OpenGL debug output functionality is supported by the current hardware and driver:
    static bool stat_isGLDebugOutputSupported = false;

    // Contains true iff this is the first call to this function:
    static bool stat_isFirstCall = true;

    // If this is the first call to this function:
    if (stat_isFirstCall)
    {
        // Check first if this is an ATI vendor:
        oaOpenGLRenderContext* pDefaultRenderContext = oaOpenGLRenderContext::getDefaultRenderContext();
        GT_IF_WITH_ASSERT(pDefaultRenderContext != NULL)
        {
            stat_isFirstCall = false;

            stat_isGLDebugOutputSupported = pDefaultRenderContext->isExtensionSupported(L"GL_ARB_debug_output");

            if (!stat_isGLDebugOutputSupported)
            {
                stat_isGLDebugOutputSupported = pDefaultRenderContext->isExtensionSupported(L"GL_AMD_debug_output");

                if (!stat_isGLDebugOutputSupported)
                {
                    stat_isGLDebugOutputSupported = pDefaultRenderContext->isExtensionSupported(L"GL_AMDX_debug_output");
                }
            }
        }
    }

    return stat_isGLDebugOutputSupported;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaEnableGLDebugOutputLogging
// Description:
// Arguments:   bool isGLDebugOutputIntegrationEnabled
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaEnableGLDebugOutputLogging(bool isGLDebugOutputLoggingEnabled)
{
    bool retVal = false;

    (void)(isGLDebugOutputLoggingEnabled); // Resolve the compiler warning for the Linux variant
    // Debug output mechanism supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        retVal = stat_thePersistentDataMgr.enableGLDebugOutputLogging(isGLDebugOutputLoggingEnabled);
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetGLDebugOutputLoggingEnabledStatus
// Description:
// Arguments:   bool& isGLDebugOutputLoggingEnabled
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetGLDebugOutputLoggingEnabledStatus(bool& isGLDebugOutputLoggingEnabled)
{
    bool retVal = false;

    (void)(isGLDebugOutputLoggingEnabled); // Resolve the compiler warning for the Linux variant
    // OpenGL Debug output is currently supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        stat_thePersistentDataMgr.getGLDebugOutputLoggingStatus(isGLDebugOutputLoggingEnabled);
        retVal = true;
    }
#endif
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetGLDebugOutputSeverityEnabled
// Description: Get the debug output severity
// Arguments:   apGLDebugOutputSeverity& severity
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool& enabled)
{
    bool retVal = false;

    (void)(severity); // Resolve the compiler warning for the Linux variant
    (void)(enabled);
    // OpenGL Debug output is currently supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        enabled = stat_thePersistentDataMgr.getGLDebugOutputSeverityEnabled(severity);
        retVal = true;
    }
#endif
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetGLDebugOutputSeverityEnabled
// Description: Sets debug output severity
// Arguments:   apGLDebugOutputSeverity debugOutputSeverity
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity debugOutputSeverity, bool enabled)
{
    bool retVal = false;

    (void)(debugOutputSeverity); // Resolve the compiler warning for the Linux variant
    (void)(enabled);
    // Debug output is currently supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        retVal = stat_thePersistentDataMgr.setGLDebugOutputSeverityEnabled(debugOutputSeverity, enabled);
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetGLDebugOutputKindMask
// Description: Sets debug output category mask
// Arguments:   const gtUInt64& debugReportMask
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetGLDebugOutputKindMask(const gtUInt64& debugCategoryMask)
{
    bool retVal = false;

    (void)(debugCategoryMask); // Resolve the compiler warning for the Linux variant
    // Debug output is currently supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        retVal = stat_thePersistentDataMgr.setGLDebugOutputKindMask(debugCategoryMask);
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetGLDebugOutputKindMask
// Description: Retrieves the current debug output category mask
// Arguments:   gtUInt64& debugReportMask
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetGLDebugOutputKindMask(gtUInt64& debugCategoryMask)
{
    bool retVal = false;

    (void)(debugCategoryMask); // Resolve the compiler warning for the Linux variant
    // Debug output is currently supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        debugCategoryMask = stat_thePersistentDataMgr.getGLDebugOutputKindMask();
        retVal = true;
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaMarkContextAsDeleted
// Description: Marks a context as deleted
// Arguments:   apContextID deletedContextID
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/7/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaMarkContextAsDeleted(const apContextID& deletedContextID)
{
    bool retVal = false;

    // Mark the context as deleted in the persistant data manager:
    retVal = stat_thePersistentDataMgr.markContextAsDeleted(deletedContextID);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaDoesDebugForcedContextExist
// Description: Check if there is an existing context with debug flag forced
// Arguments:   bool isDebugContext - the context is forced on/off
//              bool& isDebugContextForced - output
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/7/2010
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaDoesDebugForcedContextExist(bool isDebugContext, bool& isDebugContextForced)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaDoesDebugForcedContextExist;

        // Send the arguments:
        spyConnectionSocket << isDebugContext;

        // Perform after API call actions:
        pdProcessDebugger::instance().afterAPICallIssued();

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the answer:
            spyConnectionSocket >> isDebugContextForced;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfLocalMachineGPUs
// Description: Retrieves the amount of local machine GPUs.
// Arguments: Will get the amount of local machine GPUs.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/3/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfLocalMachineGPUs(int& GPUsAmount)
{
    (void)(GPUsAmount); // unused
    bool retVal = false;

    // retVal = stat_thePerformanceCountersMgr.getAmountOfLocalMachineGPUs(GPUsAmount);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetLocalMachineGPUInformation
// Description: Retrieves information about a local machine GPU.
// Arguments: GPUIndex - The index of the queried GPU [0, GPUs amount -1].
//            GPUInfo - Will get the queried GPU information.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/3/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetLocalMachineGPUInformation(int GPUIndex, apGPUInfo& GPUInfo)
{
    (void)(GPUIndex); // unused
    (void)(GPUInfo); // unused
    bool retVal = false;

    // retVal = stat_thePerformanceCountersMgr.getLocalMachineGPUInformation(GPUIndex, GPUInfo);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaReadFile
// Description: Reads a file from the remote machine (where the spy is running)
//              through the API pipe into a memory buffer
// Arguments: remoteFilePath - the file's path on the remote machine
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaReadFile(const osFilePath& remoteFilePath, osRawMemoryBuffer& memoryBuffer)
{
    bool retVal = false;

    // Verify the spy connection is active:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // Get the Spy connection socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the command ID:
        spyConnectionSocket << (gtInt32)GA_FID_gaReadFile;

        // Send the file path:
        bool rcPath = remoteFilePath.writeSelfIntoChannel(spyConnectionSocket);
        GT_ASSERT(rcPath);

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Read the file contents:
            bool rcBuffer = memoryBuffer.readSelfFromChannel(spyConnectionSocket);
            GT_ASSERT(rcBuffer);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaWriteFile
// Description: Writes a memory buffer through the API pipe into a file on
//              the remote machine (where the spy is running).
//              This function overwrites any existing file, and creates a new
//              file if needed.
// Arguments: remoteFilePath - the file's path on the remote machine
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaWriteFile(const osFilePath& remoteFilePath, const osRawMemoryBuffer& memoryBuffer)
{
    bool retVal = false;

    // Verify the spy connection is active:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // Get the Spy connection socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the command ID:
        spyConnectionSocket << (gtInt32)GA_FID_gaWriteFile;

        // Send the file path:
        bool rcPath = remoteFilePath.writeSelfIntoChannel(spyConnectionSocket);
        GT_ASSERT(rcPath);

        // Send the file contents:
        bool rcBuffer = memoryBuffer.writeSelfIntoChannel(spyConnectionSocket);
        GT_ASSERT(rcBuffer);

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaRemoteToLocalFile
// Description: Asks the process debugger to get a file from a (possibly) remote
//              machine, modifying io_filePath as needed.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        30/9/2013
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaRemoteToLocalFile(osFilePath& io_filePath, bool useCache)
{
    bool retVal = true;

    pdProcessDebugger::instance().remoteToLocalFilePath(io_filePath, useCache);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaRemoteToLocalCallStack
// Description: Performs gaRemoteToLocalFile on all frames of a call stack.
//              Note that this function is intentionally left outside
//              the gaGRApiFunctions class, and should not be overridden.
// Author:      Uri Shomroni
// Date:        30/9/2013
// ---------------------------------------------------------------------------
bool gaRemoteToLocalCallStack(osCallStack& io_callStack)
{
    bool retVal = true;

    int numberOfStackFrames = io_callStack.amountOfStackFrames();

    for (int i = 0; i < numberOfStackFrames; i++)
    {
        const osCallStackFrame* pCurrentFrame = io_callStack.stackFrame(i);

        if (NULL != pCurrentFrame)
        {
            osCallStackFrame mutableFrame = *pCurrentFrame;
            osFilePath sourceFilePath = mutableFrame.sourceCodeFilePath();
            bool rcMir = gaRemoteToLocalFile(sourceFilePath, true);

            if (rcMir && (mutableFrame.sourceCodeFilePath() != sourceFilePath))
            {
                mutableFrame.setSourceCodeFilePath(sourceFilePath);
                io_callStack.setStackFrame(mutableFrame, i);
            }

            retVal = retVal && rcMir;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaFindMultipleAPIsFunctionID
// Description:
// Arguments:   apAPIFunctionId originalFunctionId
//            apCounterType contextType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
apAPIFunctionId gaGRApiFunctions::gaFindMultipleAPIsFunctionID(apAPIFunctionId originalFunctionId, apContextType contextType, apAPIConnectionType& apiConnectionType)
{
    apAPIFunctionId retVal = originalFunctionId;

    // Find the right API connection for this context type:
    apiConnectionType = AP_AMOUNT_OF_API_CONNECTION_TYPES;

    if (contextType == AP_NULL_CONTEXT)
    {
        // For the NULL context find the connected active API:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            apiConnectionType = AP_OPENGL_API_CONNECTION;
        }
        else if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENCL_API_CONNECTION))
        {
            apiConnectionType = AP_OPENCL_API_CONNECTION;
        }
    }
    else
    {
        apiConnectionType = apContextIDToAPIConnectionType(contextType);
    }

    // Get the function id by the original function id and the connection type:
    switch (originalFunctionId)
    {
        case GA_FID_gaGetAmountOfCurrentFrameFunctionCalls:
            if (apiConnectionType == AP_OPENCL_API_CONNECTION)
            {
                retVal = GA_FID_gaGetAmountOfOpenCLFunctionCalls;
            }

            break;

        case GA_FID_gaGetCurrentFrameFunctionCall:
            if (apiConnectionType == AP_OPENCL_API_CONNECTION)
            {
                retVal = GA_FID_gaGetOpenCLFunctionCall;
            }

            break;

        case GA_FID_gaGetLastFunctionCall:
            if (apiConnectionType == AP_OPENCL_API_CONNECTION)
            {
                retVal = GA_FID_gaGetLastOpenCLFunctionCall;
            }

            break;

        case GA_FID_gaGetContextLogFilePath:
            if (apiConnectionType == AP_OPENCL_API_CONNECTION)
            {
                retVal = GA_FID_gaGetCLContextLogFilePath;
            }

            break;

        case GA_FID_gaFindCurrentFrameFunctionCall:
            if (apiConnectionType == AP_OPENCL_API_CONNECTION)
            {
                retVal = GA_FID_gaFindOpenCLFunctionCall;
            }

            break;

        case GA_FID_gaGetCurrentStatistics:
            if (apiConnectionType == AP_OPENCL_API_CONNECTION)
            {
                retVal = GA_FID_gaGetCurrentOpenCLStatistics;
            }

            break;

        default:
        {
            GT_ASSERT(false);
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaSetHexDisplayMode
// Description: Set the hex mode
// Author:      Gilad Yarnitzky
// Date:        18/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaSetHexDisplayMode(bool hexDisplayMode)
{
    bool retVal = true;

    gaPersistentDataManager::instance().setHexDisplayMode(hexDisplayMode);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaIsHexDisplayMode
// Description: Set the hex mode
// Author:      Gilad Yarnitzky
// Date:        18/5/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaIsHexDisplayMode()
{
    bool retVal = true;

    retVal = gaPersistentDataManager::instance().isHexDisplayMode();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::deleteInstance
// Description: Deletes the single instance as a part of unloading cleanup.
//              This function SHOULD NOT be called while running, only when a
//              library that implements gaGRApiFunctions or a subclass of it
//              unloads.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
void gaGRApiFunctions::deleteInstance()
{
    delete _pMySingleInstance;
    _pMySingleInstance = NULL;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCrashReportAdditionalInformation
// Description: Get additional information to pass to the crash report
// Arguments:   bool& openCLEnglineLoaded
//              bool& openGLEnglineLoaded
//              bool &kernelDebuggingEnteredAtLeastOnce
// Author:      Gilad Yarnitzky
// Date:        7/7/2011
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetCrashReportAdditionalInformation(bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    bool retVal = true;

    gaPersistentDataManager::instance().getCrashReportAdditionalInformation(openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);

    return retVal;
}

bool gaGRApiFunctions::gaIsInHSAKernelBreakpoint()
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaIsInHSAKernelBreakpoint;

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSAGetCurrentLine(gtUInt64& line, gtUInt64& addr)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSAGetCurrentLine;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive output values:
            spyConnectionSocket >> line;
            spyConnectionSocket >> addr;
        }
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSAGetSourceFilePath(osFilePath& srcPath, gtString& kernelName)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSAGetSourceFilePath;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive output value:
            retVal = srcPath.readSelfFromChannel(spyConnectionSocket);

            spyConnectionSocket >> kernelName;
        }
    }

    return retVal;
}
bool gaGRApiFunctions::gaHSAGetCallStack(osCallStack& stack)
{
    // For now, construct the stack from the other two functions:
    bool retVal = false;

    osFilePath hsaSrc;
    gtString hsaKer;
    bool rcSrc = gaHSAGetSourceFilePath(hsaSrc, hsaKer);
    gtUInt64 hsaLn = 0;
    gtUInt64 hsaAddr = 0;
    bool rcLn = gaHSAGetCurrentLine(hsaLn, hsaAddr);
    retVal = rcSrc && rcLn;

    if (retVal)
    {
        osCallStackFrame hsaStackFrame;
        hsaStackFrame.setSourceCodeFilePath(hsaSrc);
        hsaStackFrame.setSourceCodeFileLineNumber((int)hsaLn);
        hsaStackFrame.setInstructionCounterAddress((osInstructionPointer)hsaAddr);
        hsaStackFrame.setFunctionName(hsaKer);
        stack.setAddressSpaceType(true);
        stack.addStackFrame(hsaStackFrame);
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSASetNextDebuggingCommand(apKernelDebuggingCommand cmd)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSASetNextDebuggingCommand;

        // Send the command Id:
        spyConnectionSocket << (gtUInt32)cmd;

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSASetBreakpoint(const gtString& kernelName, const gtUInt64& line)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSASetBreakpoint;

        // Send the kernel name:
        spyConnectionSocket << kernelName;

        // Send the line number:
        spyConnectionSocket << line;

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSAListVariables(gtVector<gtString>& variables)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSAListVariables;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive output values:
            gtUInt32 varCount = 0;
            spyConnectionSocket >> varCount;

            for (gtUInt32 i = 0; varCount > i; ++i)
            {
                gtString currVar;
                spyConnectionSocket >> currVar;
                variables.push_back(currVar);
            }
        }
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSAGetVariableValue(const gtString& varName, gtString& varValue, gtString& varValueHex, gtString& varType)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSAGetVariableValue;

        // Send the variable name:
        spyConnectionSocket << varName;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive output values:
            spyConnectionSocket >> varValue;
            spyConnectionSocket >> varValueHex;
            spyConnectionSocket >> varType;
        }
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSAListWorkItems(gtVector<gtUInt32>& o_gidLidWgid)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSAListWorkItems;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive output value:
            gtUInt32 wiCount = 0;
            spyConnectionSocket >> wiCount;

            // 3 Coordinates x 3 coordinate sets, per work item:
            o_gidLidWgid.resize(9 * wiCount);

            for (gtUInt32 i = 0; wiCount > i; ++i)
            {
                spyConnectionSocket >> o_gidLidWgid[9 * i    ]; // Global Id X
                spyConnectionSocket >> o_gidLidWgid[9 * i + 1]; // Global Id Y
                spyConnectionSocket >> o_gidLidWgid[9 * i + 2]; // Global Id Z
                spyConnectionSocket >> o_gidLidWgid[9 * i + 3]; // Local Id X
                spyConnectionSocket >> o_gidLidWgid[9 * i + 4]; // Local Id Y
                spyConnectionSocket >> o_gidLidWgid[9 * i + 5]; // Local Id Z
                spyConnectionSocket >> o_gidLidWgid[9 * i + 6]; // Workgroup Id X
                spyConnectionSocket >> o_gidLidWgid[9 * i + 7]; // Workgroup Id Y
                spyConnectionSocket >> o_gidLidWgid[9 * i + 8]; // Workgroup Id Z
            }
        }
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSASetActiveWorkItemIndex(gtUInt32 wiIndex)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSASetActiveWorkItemIndex;

        // Send the index:
        spyConnectionSocket << wiIndex;

        // Receive success value:
        spyConnectionSocket >> retVal;

        // Register a changed work item event:
        apKernelWorkItemChangedEvent workItemEvent(0, wiIndex);
        apEventsHandler::instance().registerPendingDebugEvent(workItemEvent);
    }

    return retVal;
}

bool gaGRApiFunctions::gaHSAGetWorkDims(gtUByte& dims)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_HSA_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaHSAGetWorkDims;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive output value:
            spyConnectionSocket >> dims;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfPipelineObjects
// Description: Returns the amount of the OpenGL Program Pipeline objects.
// Arguments:   int contextId - The relevant OpenGL context.
//              int& amountOfProgramPipelines - an output parameter to hold the result
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        24/6/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfPipelineObjects(int contextId, int& amountOfProgramPipelines)
{
    bool retVal = false;

    if (contextId == 0)
    {
        // The input context is the NULL context:
        amountOfProgramPipelines = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfPipelineObjects;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfPipelinesAsInt32 = 0;
                spyConnectionSocket >> amountOfPipelinesAsInt32;
                amountOfProgramPipelines = static_cast<int>(amountOfPipelinesAsInt32);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPipelineObjectDetails
// Description:
//   Returns a Program Pipeline's details.
// Arguments:   int contextId - The relevant OpenGL context id.
//              GLuint pipelineName - The OpenGL name of the relevant pipeline
//              apGLPipeline& programPipelineDetails - an output buffer to hold the
//                                                     requested details.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        24/6/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPipelineObjectDetails(int contextId, GLuint pipelineName, apGLPipeline& programPipelineDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetPipelineObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)pipelineName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = programPipelineDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetPipelineObjectName
// Description: Returns the OpenGL name of a Program Pipeline.
// Arguments:   contextId - The OpenGL context id of the Program Pipeline.
//              int pipelineIndex - the index of the program pipeline (storage
//                                  index of the pipeline in the monitor class,
//                                  not related to OpenGL).
//              GLuint& pipelineName - a buffer to hold the result.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        24/6/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetPipelineObjectName(int contextId, int pipelineIndex, GLuint& pipelineName)
{
    bool retVal = false;
    pipelineName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetPipelineObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)pipelineIndex;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 pipelineNameAsUInt32 = 0;
                spyConnectionSocket >> pipelineNameAsUInt32;
                pipelineName = (GLuint)pipelineNameAsUInt32;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetAmountOfSamplerObjects
// Description: Returns the amount of the OpenGL sampler objects.
// Arguments:   int contextId - The relevant OpenGL context.
//              int& amountOfSamplers - an output parameter to hold the result.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        24/6/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetAmountOfSamplerObjects(int contextId, int& amountOfSamplers)
{
    bool retVal = false;

    if (contextId == 0)
    {
        // The input context is the NULL context:
        amountOfSamplers = 0;
        retVal = true;
    }
    else if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetAmountOfSamplerObjects;

            // Send the context id:
            spyConnectionSocket << (gtInt32)contextId;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtInt32 amountOfSamplersAsInt32 = 0;
                spyConnectionSocket >> amountOfSamplersAsInt32;
                amountOfSamplers = static_cast<int>(amountOfSamplersAsInt32);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetSamplerObjectDetails
// Description:
//   Returns an OpenGL sampler object's details.
// Arguments:   int contextId - The relevant OpenGL context id.
//              GLuint samplerName - The OpenGL name of the relevant sampler
//              apGLSampler& samplerDetails - an output buffer to hold the
//                                            requested details.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        24/6/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetSamplerObjectDetails(int contextId, GLuint samplerName, apGLSampler& samplerDetails)
{
    bool retVal = false;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetSamplerObjectDetails;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtUInt32)samplerName;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                retVal = samplerDetails.readSelfFromChannel(spyConnectionSocket);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGRApiFunctions::gaGetSamplerObjectName
// Description: Returns the OpenGL name of an OpenGL sampler object.
// Arguments:   contextId - The OpenGL context id of the sampler.
//              int samplerIndex  - the index of the sampler (storage
//                                  index of the sampler in the monitor class,
//                                  not related to OpenGL).
//              GLuint& samplerName - a buffer to hold the result.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        24/6/2014
// ---------------------------------------------------------------------------
bool gaGRApiFunctions::gaGetSamplerObjectName(int contextId, int samplerIndex, GLuint& samplerName)
{
    bool retVal = false;
    samplerName = 0;

    // If the input context is a real context:
    if (contextId > 0)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetSamplerObjectName;

            // Send parameters:
            spyConnectionSocket << (gtInt32)contextId;
            spyConnectionSocket << (gtInt32)samplerIndex;

            // Receive success value:
            spyConnectionSocket >> retVal;

            if (retVal)
            {
                // Receive parameters:
                gtUInt32 samplerNameAsUInt32 = 0;
                spyConnectionSocket >> samplerNameAsUInt32;
                samplerName = (GLuint)samplerNameAsUInt32;
            }
        }
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
// Global function connection macros start here
//////////////////////////////////////////////////////////////////////////
//GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS
// API package:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIntializeAPIPackage, bool, (bool shouldInitPerfCounters), (shouldInitPerfCounters));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsAPIConnectionActive, bool, (apAPIConnectionType apiType), (apiType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsAPIConnectionActiveAndDebuggedProcessSuspended, bool, (apAPIConnectionType apiType), (apiType));

// Debugged process:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaDebuggedExecutableExists, bool, (const apDebugProjectSettings& processCreationData), (processCreationData));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSystemLibrariesExists, bool, (const apDebugProjectSettings& processCreationData, gtString& missingSystemLibraries), (processCreationData, missingSystemLibraries));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaLaunchDebuggedProcess, bool, (const apDebugProjectSettings& processCreationData, bool launchSuspended), (processCreationData, launchSuspended));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaContinueDebuggedProcessFromSuspendedCreation, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaDebuggedProcessExists, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaTerminateDebuggedProcess, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSuspendDebuggedProcess, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaResumeDebuggedProcess, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaLockDriverThreads, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUnLockDriverThreads, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSuspendThreads, bool, (const std::vector<osThreadId>& thrds), (thrds));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaResumeThreads, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsDebuggedProcessSuspended, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsDuringDebuggedProcessTermination, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsDebugging64BitApplication, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCurrentDebugSessionLogFilesSubDirectory, bool, (osFilePath& logFilesPath), (logFilesPath));

// Debugged process threads:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfDebuggedProcessThreads, bool, (int& threadsAmount), (threadsAmount));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetThreadId, bool, (int threadIndex, osThreadId& threadId), (threadIndex, threadId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetBreakpointTriggeringThreadIndex, bool, (int& threadIndex), (threadIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetThreadCurrentRenderContext, bool, (const osThreadId& threadId, int& contextId), (threadId, contextId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetBreakpointTriggeringContextId, bool, (apContextID& contextId), (contextId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderContextCurrentThread, bool, (int contextId, osThreadId& threadId), (contextId, threadId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetThreadCallStack, bool, (const osThreadId& threadId, osCallStack& callStack, bool hideSpyDLLsFunctions, bool fetchRemoteSourceFiles), (threadId, callStack, hideSpyDLLsFunctions, fetchRemoteSourceFiles));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaCanGetHostVariables, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaCanGetHostDebugging, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHostDebuggerStepIn, bool, (osThreadId threadId), (threadId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHostDebuggerStepOut, bool, (osThreadId threadId), (threadId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHostDebuggerStepOver, bool, (osThreadId threadId), (threadId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetThreadLocals, bool, (const osThreadId& threadId, int frameIndex, gtVector<gtString>& o_locals), (threadId, frameIndex, o_locals));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetThreadVariableValue, bool, (const osThreadId& threadId, int frameIndex, const gtString& variableName, gtString& o_variableValue, gtString& o_variableValueHex, gtString& o_varType), (threadId, frameIndex, variableName, o_variableValue, o_variableValueHex, o_varType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsHostBreakPoint, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetHostBreakpoint, bool, (const osFilePath& filePath, int lineNumber), (filePath, lineNumber));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetHostBreakpointLocation, bool, (osFilePath& bpFile, int& bpLine), (bpFile, bpLine));

// Render contexts:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfRenderContexts, bool, (int& contextsAmount), (contextsAmount));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderContextDetails, bool, (int contextId, apGLRenderContextInfo& renderContextInfo), (contextId, renderContextInfo));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderContextGraphicDetails, bool, (int contextId, apGLRenderContextGraphicsInfo& renderContextGraphicsInfo), (contextId, renderContextGraphicsInfo));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderContextLivingShareListsHolder, bool, (int contextID, int& livingContextID), (contextID, livingContextID));

// OpenCL contexts:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLContextDetails, bool, (int contextId, apCLContext& renderContextInfo), (contextId, renderContextInfo));

// OpenCL & OpenGL contexts:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaWasContextDeleted, bool, (const apContextID& contextId), (contextId));

// Monitored state variables:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenGLStateVariables, bool, (int& amountOfStateVariables), (amountOfStateVariables));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenGLStateVariableName, bool, (int stateVariableId, gtString& stateVariableName), (stateVariableId, stateVariableName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenGLStateVariableGlobalType, bool, (int stateVariableId, unsigned int& stateVariableGlobalType), (stateVariableId, stateVariableGlobalType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenGLStateVariableId, bool, (const gtString& stateVariableName, int& stateVariableId), (stateVariableName, stateVariableId));

// Monitored functions:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfMonitoredFunctions, bool, (int& amountOfFunctions), (amountOfFunctions));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetMonitoredFunctionName, bool, (apMonitoredFunctionId functionId, gtString& functionName), (functionId, functionName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetMonitoredFunctionId, bool, (const gtString& functionName, apMonitoredFunctionId& functionId), (functionName, functionId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetMonitoredFunctionType, bool, (apMonitoredFunctionId functionId, unsigned int& functionType), (functionId, functionType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetMonitoredFunctionAPIType, bool, (apMonitoredFunctionId functionId, unsigned int& functionAPIType), (functionId, functionAPIType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetMonitoredFunctionDeprecationVersion, bool, (apMonitoredFunctionId functionId, apAPIVersion& deprectedAtVersion, apAPIVersion& removedAtVersion), (functionId, deprectedAtVersion, removedAtVersion));

// State variable values:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetDefaultOpenGLStateVariableValue, bool, (int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue), (contextId, stateVariableId, aptrStateVariableValue));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenGLStateVariableValue, bool, (int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue), (contextId, stateVariableId, aptrStateVariableValue));

// Textures:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaEnableImagesDataLogging, bool, (bool isImagesDataLogged), (isImagesDataLogged));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsImagesDataLogged, bool, (bool& isImagesDataLogged), (isImagesDataLogged));

GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfTextureUnits, bool, (int contextId, int& amountOfTextureUnits), (contextId, amountOfTextureUnits));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetActiveTextureUnit, bool, (int contextId, int& activeTextureUnitId), (contextId, activeTextureUnitId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureUnitName, bool, (int contextId, int textureUnitId, GLenum& textureUnitName), (contextId, textureUnitId, textureUnitName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetEnabledTexturingMode, bool, (int contextId, int textureUnitId, bool& isTexturingEnabled, apTextureType& enabledTexturingMode), (contextId, textureUnitId, isTexturingEnabled, enabledTexturingMode));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetBoundTexture, bool, (int contextId, int textureUnitId, apTextureType bindTarget, GLuint& textureName), (contextId, textureUnitId, bindTarget, textureName));

GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfTextureObjects, bool, (int contextId, int& amountOfTextures), (contextId, amountOfTextures));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureObjectName, bool, (int contextId, int textureId, GLuint& textureName), (contextId, textureId, textureName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureObjectType, bool, (int contextId, int textureId, apTextureType& textureType), (contextId, textureId, textureType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureObjectDetails, bool, (int contextId, GLuint textureName, apGLTexture& textureDetails), (contextId, textureName, textureDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureObjectThumbnailData, bool, (int contextId, GLuint textureName, apGLTextureMiplevelData& textureThumbDetails), (contextId, textureName, textureThumbDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureDataObjectDetails, bool, (int contextId, GLuint textureName, apGLTextureData& textureData), (contextId, textureName, textureData));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureMemoryDataObjectDetails, bool, (int contextId, GLuint textureName, apGLTextureMemoryData& textureData), (contextId, textureName, textureData));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateTextureRawData, bool, (int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector), (contextId, texturesVector));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetTextureMiplevelDataFilePath, bool, (int contextId, apGLTextureMipLevelID textureMiplevelId, int faceIndex, osFilePath& filePath), (contextId, textureMiplevelId, faceIndex, filePath));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateTextureParameters, bool, (int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams), (contextId, texturesVector, shouldUpdateOnlyMemoryParams));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsTextureImageDirty, bool, (int contextId, apGLTextureMipLevelID textureMiplevelId, bool& dirtyImageExists, bool& dirtyRawDataExists), (contextId, textureMiplevelId, dirtyImageExists, dirtyRawDataExists));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaMarkAllTextureImagesAsUpdated, bool, (int contextId, int textureId), (contextId, textureId));

// Render buffers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfRenderBufferObjects, bool, (int contextId, int& amountOfRenderBuffers), (contextId, amountOfRenderBuffers));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderBufferObjectName, bool, (int contextId, int renderBufferId, GLuint& renderBufferName), (contextId, renderBufferId, renderBufferName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderBufferObjectDetails, bool, (int contextId, GLuint renderBufferName, apGLRenderBuffer& renderBufferDetails), (contextId, renderBufferName, renderBufferDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateRenderBufferRawData, bool, (int contextId, const gtVector<GLuint>& renderBuffersVector), (contextId, renderBuffersVector));

// Program Pipelines:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfPipelineObjects, bool, (int contextId, int& amountOfProgramPipelines), (contextId, amountOfProgramPipelines));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPipelineObjectDetails, bool, (int contextId, GLuint pipelineName, apGLPipeline& programPipelineDetails), (contextId, pipelineName, programPipelineDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPipelineObjectName, bool, (int contextId, int pipelineIndex, GLuint& pipelineName), (contextId, pipelineIndex, pipelineName));

// Samplers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfSamplerObjects, bool, (int contextId, int& amountOfSamplers), (contextId, amountOfSamplers));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetSamplerObjectDetails, bool, (int contextId, GLuint samplerName, apGLSampler& samplerDetails), (contextId, samplerName, samplerDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetSamplerObjectName, bool, (int contextId, int samplerIndex, GLuint& samplerName), (contextId, samplerIndex, samplerName));

// FBOs
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfFBOs, bool, (int contextId, int& amountOfFBOs), (contextId, amountOfFBOs));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetFBOName, bool, (int contextId, int fboId, GLuint& fboName), (contextId, fboId, fboName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetFBODetails, bool, (int contextId, GLuint fboName, apGLFBO& fboDetails), (contextId, fboName, fboDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetActiveFBO, bool, (int contextId, GLuint& fboName), (contextId, fboName));

// VBOs
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfVBOs, bool, (int contextId, int& amountOfVBOs), (contextId, amountOfVBOs));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetVBOName, bool, (int contextId, int vboId, GLuint& vboName), (contextId, vboId, vboName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetVBODetails, bool, (int contextId, GLuint vboName, apGLVBO& vboDetails), (contextId, vboName, vboDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetVBOAttachment, bool, (int contextId, GLuint vboName, GLenum& vboLastTarget, gtVector<GLenum>& vboCurrentTargets), (contextId, vboName, vboLastTarget, vboCurrentTargets));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateVBORawData, bool, (int contextId, const gtVector<GLuint>& vboNamesVector), (contextId, vboNamesVector));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetVBODisplayProperties, bool, (int contextId, GLuint vboName, oaTexelDataFormat displayFormat, int offset, GLsizei stride), (contextId, vboName, displayFormat, offset, stride));

// Static Buffers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfStaticBuffersObjects, bool, (int contextId, int& amountOfStaticBuffers), (contextId, amountOfStaticBuffers));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetStaticBufferType, bool, (int contextId, int staticBufferId, apDisplayBuffer& bufferType), (contextId, staticBufferId, bufferType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetStaticBufferObjectDetails, bool, (int contextId, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails), (contextId, bufferType, staticBufferDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateStaticBufferRawData, bool, (int contextId, apDisplayBuffer bufferType), (contextId, bufferType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateStaticBuffersDimension, bool, (int contextId), (contextId));

// PBuffers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfPBuffersObjects, bool, (int& amountOfPBuffers), (amountOfPBuffers));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPBufferObjectDetails, bool, (int pbufferID, apPBuffer& pbufferDetails), (pbufferID, pbufferDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfPBufferContentObjects, bool, (int pbufferID, int& amountOfPBufferContentObjects), (pbufferID, amountOfPBufferContentObjects));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPBufferStaticBufferType, bool, (int pbufferID, int staticBufferIter, apDisplayBuffer& bufferType), (pbufferID, staticBufferIter, bufferType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPBufferStaticBufferObjectDetails, bool, (int pbufferID, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails), (pbufferID, bufferType, staticBufferDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdatePBufferStaticBufferRawData, bool, (int pbufferContextId, int pbufferID, apDisplayBuffer bufferType), (pbufferContextId, pbufferID, bufferType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdatePBuffersDimension, bool, (int contextId), (contextId));

// Sync objects:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfSyncObjects, bool, (int& amountOfSyncObjects), (amountOfSyncObjects));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetSyncObjectDetails, bool, (int syncId, apGLSync& syncObjectDetails), (syncId, syncObjectDetails));

// Programs and Shaders:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfProgramObjects, bool, (int contextId, int& amountOfPrograms), (contextId, amountOfPrograms));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetProgramObjectName, bool, (int contextId, int programId, GLuint& programName), (contextId, programId, programName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetActiveProgramObjectName, bool, (int contextId, GLuint& activeProgramName), (contextId, activeProgramName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetProgramObjectDetails, bool, (int contextId, GLuint programName, apGLProgram& programDetails), (contextId, programName, programDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetProgramActiveUniforms, bool, (int contextId, GLuint programName, apGLItemsCollection& activeUniforms), (contextId, programName, activeUniforms));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaLinkProgramObject, bool, (int contextId, GLuint programName, bool& wasLinkSuccessful, gtString& linkLog), (contextId, programName, wasLinkSuccessful, linkLog));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaValidateProgramObject, bool, (int contextId, GLuint programName, bool& wasValidationSuccessful, gtString& validationLog), (contextId, programName, wasValidationSuccessful, validationLog));

GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfShaderObjects, bool, (int contextId, int& amountOfShaders), (contextId, amountOfShaders));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetShaderObjectName, bool, (int contextId, int shaderId, GLuint& shaderName), (contextId, shaderId, shaderName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetShaderObjectDetails, bool, (int contextId, GLuint shaderName, gtAutoPtr<apGLShaderObject>& aptrShaderDetails), (contextId, shaderName, aptrShaderDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaMarkShaderObjectSourceCodeAsForced, bool, (int contextId, GLuint shaderName, bool isSourceCodeForced), (contextId, shaderName, isSourceCodeForced));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetShaderObjectSourceCode, bool, (int contextId, GLuint shaderName, const osFilePath& sourceCodeFile), (contextId, shaderName, sourceCodeFile));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaCompileShaderObject, bool, (int contextId, GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog), (contextId, shaderName, wasCompilationSuccessful, compilationLog));

// Display Lists:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfDisplayLists, bool, (int contextID, int& amountOfDisplayLists), (contextID, amountOfDisplayLists));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetDisplayListObjectName, bool, (int contextID, int displayListIndex, GLuint& displayListName), (contextID, displayListIndex, displayListName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetDisplayListObjectDetails, bool, (int contextID, GLuint displayListName, apGLDisplayList& pDisplayListDetails), (contextID, displayListName, pDisplayListDetails));

// OpenGL / OpenCL Function calls:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfCurrentFrameFunctionCalls, bool, (const apContextID& contextID, int& amountOfFunctionCalls), (contextID, amountOfFunctionCalls));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCurrentFrameFunctionCall, bool, (const apContextID& contextID, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall), (contextID, callIndex, aptrFunctionCall));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetLastFunctionCall, bool, (const apContextID& contextID, gtAutoPtr<apFunctionCall>& aptrFunctionCall), (contextID, aptrFunctionCall));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaFindCurrentFrameFunctionCall, bool, (const apContextID& contextID, apSearchDirection searchDirection, int searchStartIndex, const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex), (contextID, searchDirection, searchStartIndex, searchedString, isCaseSensitiveSearch, foundIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaClearFunctionCallsStatistics, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCurrentStatistics, bool, (const apContextID& contextID, apStatistics* pStatistics), (contextID, pStatistics));

// OpenGL function calls:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCurrentFrameFunctionCallDeprecationDetails, bool, (int contextId, int callIndex, apFunctionDeprecation& functionDeprecationDetails), (contextId, callIndex, functionDeprecationDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsInOpenGLBeginEndBlock, bool, (int contextId), (contextId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetRenderPrimitivesStatistics, bool, (int contextId, apRenderPrimitivesStatistics& renderPrimitivesStatistics), (contextId, renderPrimitivesStatistics));

// OpenCL handles:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLHandleObjectDetails, bool, (oaCLHandle openCLHandlePtr, apCLObjectID& clObjectIdDetails), (openCLHandlePtr, clObjectIdDetails));

// OpenGL Contexts:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLContexts, bool, (int& amountOfContexts), (amountOfContexts));

// OpenCL programs:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLProgramObjects, bool, (int contextId, int& amountOfPrograms), (contextId, amountOfPrograms));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLProgramObjectDetails, bool, (int contextId, int programIndex, apCLProgram& programDetails), (contextId, programIndex, programDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetOpenCLProgramCode, bool, (oaCLProgramHandle programHandle, const osFilePath& newSourcePath), (programHandle, newSourcePath));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaBuildOpenCLProgram, bool, (oaCLProgramHandle programHandle, apCLProgram*& pFailedProgramData), (programHandle, pFailedProgramData));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLProgramHandleFromSourceFilePath, bool, (const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath, oaCLProgramHandle& programHandle), (sourceFilePath, newTempSourceFilePath, programHandle));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetKernelSourceFilePath, bool, (gtVector<osFilePath>& programsFilePath), (programsFilePath));

// OpenCL kernels:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLKernelObjectDetails, bool, (int contextId, oaCLKernelHandle kernelHandle, apCLKernel& kernelDetails), (contextId, kernelHandle, kernelDetails));

// OpenCL kernel debugging:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingLocation, bool, (oaCLProgramHandle& debuggedProgramHandle, int& currentLineNumber), (debuggedProgramHandle, currentLineNumber));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCurrentlyDebuggedKernelDetails, bool, (apCLKernel& kernelDetails), (kernelDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCurrentlyDebuggedKernelCallStack, bool, (osCallStack& kernelStack), (kernelStack));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetKernelDebuggingCommand, bool, (apKernelDebuggingCommand command), (command));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingGlobalWorkOffset, bool, (int coordinate, int& dimension), (coordinate, dimension));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingGlobalWorkOffset, bool, (int& xOffset, int& yOffset, int& zOffset), (xOffset, yOffset, zOffset));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingGlobalWorkSize, bool, (int coordinate, int& dimension), (coordinate, dimension));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingGlobalWorkSize, bool, (int& xDimension, int& yDimension, int& zDimension), (xDimension, yDimension, zDimension));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingLocalWorkSize, bool, (int coordinate, int& dimension), (coordinate, dimension));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingLocalWorkSize, bool, (int& xDimension, int& yDimension, int& zDimension, int& amountOfDimensions), (xDimension, yDimension, zDimension, amountOfDimensions));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetKernelDebuggingCurrentWorkItemCoordinate, bool, (int coordinate, int value), (coordinate, value));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingCurrentWorkItemCoordinate, bool, (int coordinate, int& value), (coordinate, value));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingCurrentWorkItem, bool, (int& xValue, int& yValue, int& zValue), (xValue, yValue, zValue));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsInKernelDebugging, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetKernelSteppingWorkItem, bool, (const int coordinate[3]), (coordinate));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateKernelSteppingWorkItemToCurrentCoordinate, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsWorkItemValid, bool, (const int coordinate[3]), (coordinate));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetFirstValidWorkItem, bool, (int wavefrontIndex, int coordinate[3]), (wavefrontIndex, coordinate));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaCanGetKernelVariableValue, bool, (const gtString& variableName), (variableName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingVariableValueString, bool, (const gtString& variableName, const int workItem[3], gtString& variableValue, gtString& variableValueHex, gtString& variableType), (variableName, workItem, variableValue, variableValueHex, variableType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingVariableMembers, bool, (const gtString& variableName, gtVector<gtString>& memberNames), (variableName, memberNames));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingAvailableVariables, bool, (gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth), (variableNames, getLeaves, stackFrameDepth));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingAmountOfActiveWavefronts, bool, (int& amountOfWavefronts), (amountOfWavefronts));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingActiveWavefrontID, bool, (int wavefrontIndex, int& wavefrontId), (wavefrontIndex, wavefrontId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelDebuggingWavefrontIndex, bool, (const int coordinate[3], int& wavefrontIndex), (coordinate, wavefrontIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateKernelVariableValueRawData, bool, (const gtString& variableName, bool& variableTypeSupported, osFilePath& variableRawData), (variableName, variableTypeSupported, variableRawData));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetKernelSourceCodeBreakpointResolution, bool, (oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber), (programHandle, requestedLineNumber, resolvedLineNumber));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetKernelDebuggingEnable, bool, (bool kernelEnable), (kernelEnable));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetMultipleKernelDebugDispatchMode, bool, (apMultipleKernelDebuggingDispatchMode mode), (mode));

// OpenCL device:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLDeviceObjectDetails, bool, (int deviceId, apCLDevice& deviceDetails), (deviceId, deviceDetails));

// OpenCL platforms:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLPlatformAPIID, bool, (gtUInt64 platformId, int& platformName), (platformId, platformName));

// OpenCL buffers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLBufferObjects, bool, (int contextId, int& amountOfBuffers), (contextId, amountOfBuffers));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLBufferObjectDetails, bool, (int contextId, int bufferIndex, apCLBuffer& bufferDetails), (contextId, bufferIndex, bufferDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateOpenCLBufferRawData, bool, (int contextId, const gtVector<int>& bufferIdsVector), (contextId, bufferIdsVector));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetCLBufferDisplayProperties, bool, (int contextId, int bufferId, oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride), (contextId, bufferId, displayFormat, displayOffset, displayStride));

// OpenCL Sub Buffers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLSubBufferObjectDetails, bool, (int contextId, int subBufferName, apCLSubBuffer& subBufferDetails), (contextId, subBufferName, subBufferDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateOpenCLSubBufferRawData, bool, (int contextId, const gtVector<int>& subBufferIdsVector), (contextId, subBufferIdsVector));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetCLSubBufferDisplayProperties, bool, (int contextId, int subBufferId, oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride), (contextId, subBufferId, displayFormat, displayOffset, displayStride));

// OpenCL images:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLImageObjects, bool, (int contextId, int& amountOfImages), (contextId, amountOfImages));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLImageObjectDetails, bool, (int contextId, int imageIndex, apCLImage& imageDetails), (contextId, imageIndex, imageDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdateOpenCLImageRawData, bool, (int contextId, const gtVector<int>& imageIdsVector), (contextId, imageIdsVector));

// OpenCL pipes:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLPipeObjects, bool, (int contextId, int& amountOfPipes), (contextId, amountOfPipes));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLPipeObjectDetails, bool, (int contextId, int pipeIndex, apCLPipe& pipeDetails), (contextId, pipeIndex, pipeDetails));

// OpenCL command queues:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfCommandQueues, bool, (int contextId, int& amountOfQueues), (contextId, amountOfQueues));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCommandQueueDetails, bool, (int contextId, int queueIndex, apCLCommandQueue& commandQueueDetails), (contextId, queueIndex, commandQueueDetails));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfCommandsInQueue, bool, (oaCLCommandQueueHandle hQueue, int& amountOfCommands), (hQueue, amountOfCommands));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfEventsInQueue, bool, (oaCLCommandQueueHandle hQueue, int& amountOfEvents), (hQueue, amountOfEvents));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetEnqueuedCommandDetails, bool, (oaCLCommandQueueHandle hQueue, int commandIndex, gtAutoPtr<apCLEnqueuedCommand>& aptrCommand), (hQueue, commandIndex, aptrCommand));

// OpenCL samplers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLSamplers, bool, (int contextId, int& amountOfSamplers), (contextId, amountOfSamplers));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLSamplerObjectDetails, bool, (int contextId, int samplerIndex, apCLSampler& samplerDetails), (contextId, samplerIndex, samplerDetails));

// OpenCL events:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfOpenCLEvents, bool, (int contextId, int& amountOfEvents), (contextId, amountOfEvents));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetOpenCLEventObjectDetails, bool, (int contextId, int eventIndex, apCLEvent& eventDetails), (contextId, eventIndex, eventDetails));
// Allocated Objects
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfRegisteredAllocatedObjects, bool, (unsigned int& numberOfObjects), (numberOfObjects));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAllocatedObjectCreationStack, bool, (int allocatedObjectId, osCallStack& callStack), (allocatedObjectId, callStack));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaCollectAllocatedObjectsCreationCallsStacks, bool, (bool collectCreationStacks), (collectCreationStacks));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCollectAllocatedObjectCreationCallsStacks, bool, (bool& collectCreationStacks), (collectCreationStacks));

// String markers:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaFindStringMarker, bool, (int contextId, apSearchDirection searchDirection, int searchStartIndex, int& foundIndex), (contextId, searchDirection, searchStartIndex, foundIndex));

// Breakpoints:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetBreakpoint, bool, (const apBreakPoint& breakpoint), (breakpoint));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoveBreakpoint, bool, (int breakpointId), (breakpointId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoveGenericBreakpoint, bool, (apGenericBreakpointType breakpointType), (breakpointType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfBreakpoints, bool, (int& amountOfBreakpoints), (amountOfBreakpoints));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetBreakpoint, bool, (int breakPointId, gtAutoPtr<apBreakPoint>& aptrBreakpoint), (breakPointId, aptrBreakpoint));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetBreakpointIndex, bool, (const apBreakPoint& breakpoint, int& breakpointId), (breakpoint, breakpointId));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoveAllBreakpoints, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoveAllBreakpointsByType, bool, (osTransferableObjectType breakpointType), (breakpointType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetEnableAllBreakpointsStatus, bool, (bool& isEnableAllBreakpointsChecked, bool& isEnableAllBreakpointsEnabled), (isEnableAllBreakpointsChecked, isEnableAllBreakpointsEnabled));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaBreakOnNextMonitoredFunctionCall, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaBreakOnNextDrawFunctionCall, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaBreakOnNextFrame, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaBreakInMonitoredFunctionCall, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaClearAllStepFlags, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetGenericBreakpointStatus, bool, (apGenericBreakpointType breakpointType, bool& doesExist, bool& isEnabled), (breakpointType, doesExist, isEnabled));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetKernelBreakpointProgramHandle, bool, (int breakpointIndex, oaCLProgramHandle programHandle), (breakpointIndex, programHandle));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoveAllBreakpointsByState, bool, (const apBreakPoint::State state), (state));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaTemporarilyDisableAllBreakpoints, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaEnableAllBreakpointsByState, bool, (const apBreakPoint::State state), (state));

// Log file recording:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaDeleteLogFilesWhenDebuggedProcessTerminates, bool, (bool deleteLogFiles), (deleteLogFiles));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaStartMonitoredFunctionsCallsLogFileRecording, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaStopMonitoredFunctionsCallsLogFileRecording, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsMonitoredFunctionsCallsLogFileRecordingActive, bool, (bool& isActive), (isActive));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetContextLogFilePath, bool, (apContextID contextID, bool& logFileExists, osFilePath& filePath), (contextID, logFileExists, filePath));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaFlushLogFileAfterEachFunctionCall, bool, (bool flushAfterEachFunctionCall), (flushAfterEachFunctionCall));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsLogFileFlushedAfterEachFunctionCall, bool, (bool& isLogFileFlushedAfterEachFunctionCall), (isLogFileFlushedAfterEachFunctionCall));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaWasOpenGLDataRecordedInDebugSession, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaResetRecordingWasDoneFlag, bool, (bool isEnabled), (isEnabled));

// Slow motion:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetSlowMotionDelay, bool, (int delayTimeUnits), (delayTimeUnits));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetSlowMotionDelay, bool, (int& delayTimeUnits), (delayTimeUnits));

// Force OpenGL flush:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaForceOpenGLFlush, bool, (bool isOpenGLFlushForced), (isOpenGLFlushForced));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsOpenGLFlushForced, bool, (bool& isOpenGLFlushForced), (isOpenGLFlushForced));

// "Interactive" break mode:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetInteractiveBreakMode, bool, (bool isInteractiveBreakOn), (isInteractiveBreakOn));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsInteractiveBreakOn, bool, (), ());

GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetDebuggedProcessExecutionMode, bool, (apExecutionMode executionMode), (executionMode));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetDebuggedProcessExecutionMode, bool, (apExecutionMode& executionMode), (executionMode));

// Forces OpenGL render modes:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaForceOpenGLPolygonRasterMode, bool, (apRasterMode rasterMode), (rasterMode));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaCancelOpenGLPolygonRasterModeForcing, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsOpenGLPolygonRasterModeForced, bool, (bool& isForced), (isForced));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetForceOpenGLPolygonRasterMode, bool, (apRasterMode& rasterMode), (rasterMode));

// Turn off graphic pipeline stages:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSwitchToNULLOpenGLImplementation, bool, (bool isNULLOpenGLImplOn), (isNULLOpenGLImplOn));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsUnderNULLOpenGLImplementation, bool, (bool& isUnderNULLOpenGLImplementation), (isUnderNULLOpenGLImplementation));

// OpenGL Force stub operations:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaForceOpenGLStub, bool, (apOpenGLForcedModeType openGLStubType, bool isStubForced), (openGLStubType, isStubForced));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsOpenGLStubForced, bool, (apOpenGLForcedModeType openGLStubType, bool& isStubForced), (openGLStubType, isStubForced));

// OpenCL cancel operations:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsOpenCLExecutionOn, bool, (apOpenCLExecutionType executionType, bool& isExecutionOn), (executionType, isExecutionOn));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetOpenCLExecution, bool, (apOpenCLExecutionType executionType, bool isExecutionOn), (executionType, isExecutionOn));

// Performance counters:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfPerformanceCounters, bool, (int& countersAmount), (countersAmount));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfPerformanceCountersByType, bool, (apCounterType counterType, int& countersAmount), (counterType, countersAmount));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaAddOSCounter, bool, (const apCounterInfo& newCounterInfo, int& newCounterIndex, bool& isNewCounter), (newCounterInfo, newCounterIndex, isNewCounter));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoveOSCounter, bool, (int counterIndex), (counterIndex));

GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPerformanceCounterInfo, const apCounterInfo*, (int counterIndex), (counterIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPerformanceCounterLocalIndex, bool, (int counterIndex, int& counterLocalIndex), (counterIndex, counterLocalIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPerformanceCounterType, bool, (int counterIndex, apCounterType& counterType), (counterIndex, counterType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPerformanceCounterIndex, bool, (const apCounterID& counterId, int& counterIndex), (counterId, counterIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaActivatePerformanceCounter, bool, (const apCounterActivationInfo& counterActivationInfo), (counterActivationInfo));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaUpdatePerformanceCountersValues, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetPerformanceCounterValue, double, (const apCounterID& counterId), (counterId));

// OpenCL queues:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS_VOID_RETVAL(gaOnQueueCreatedEvent, (const apOpenCLQueueCreatedEvent& eve), (eve));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS_VOID_RETVAL(gaOnQueueDeletedEvent, (const apOpenCLQueueDeletedEvent& eve), (eve));

// OpenGL debug output messages:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsGLDebugOutputSupported, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaEnableGLDebugOutputLogging, bool, (bool isGLDebugOutputLoggingEnabled), (isGLDebugOutputLoggingEnabled));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetGLDebugOutputLoggingEnabledStatus, bool, (bool& isGLDebugOutputLoggingEnabled), (isGLDebugOutputLoggingEnabled));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetGLDebugOutputSeverityEnabled, bool, (apGLDebugOutputSeverity severity, bool& enabled), (severity, enabled));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetGLDebugOutputSeverityEnabled, bool, (apGLDebugOutputSeverity severity, bool enabled), (severity, enabled));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetGLDebugOutputKindMask, bool, (const gtUInt64& debugReportMask), (debugReportMask));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetGLDebugOutputKindMask, bool, (gtUInt64& debugReportMask), (debugReportMask));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaDoesDebugForcedContextExist, bool, (bool isDebugContext, bool& isDebugContextForced), (isDebugContext, isDebugContextForced));

// GPUs:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetAmountOfLocalMachineGPUs, bool, (int& GPUsAmount), (GPUsAmount));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetLocalMachineGPUInformation, bool, (int GPUIndex, apGPUInfo& GPUInfo), (GPUIndex, GPUInfo));

// Sending files through the API pipe:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaReadFile, bool, (const osFilePath& remoteFilePath, osRawMemoryBuffer& memoryBuffer), (remoteFilePath, memoryBuffer));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaWriteFile, bool, (const osFilePath& remoteFilePath, const osRawMemoryBuffer& memoryBuffer), (remoteFilePath, memoryBuffer));

// Sending files with the process debugger:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaRemoteToLocalFile, bool, (osFilePath& io_filePath, bool useCache), (io_filePath, useCache));

// AID function for multiple APIs functions:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaFindMultipleAPIsFunctionID, apAPIFunctionId, (apAPIFunctionId originalFunctionId, apContextType contextType, apAPIConnectionType& apiConnectionType), (originalFunctionId, contextType, apiConnectionType));

// Deleted context:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaMarkContextAsDeleted, bool, (const apContextID& deletedContextID), (deletedContextID));

// VS information:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaSetHexDisplayMode, bool, (bool hexMode), (hexMode));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsHexDisplayMode, bool, (), ());

// Extra crash information:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaGetCrashReportAdditionalInformation, bool, (bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce), (openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce));

// HSA Debugging:
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaIsInHSAKernelBreakpoint, bool, (), ());
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAGetCurrentLine, bool, (gtUInt64& line, gtUInt64& addr), (line, addr));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAGetSourceFilePath, bool, (osFilePath& srcPath, gtString& kernelName), (srcPath, kernelName));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAGetCallStack, bool, (osCallStack& stack), (stack));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSASetNextDebuggingCommand, bool, (apKernelDebuggingCommand cmd), (cmd));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSASetBreakpoint, bool, (const gtString& kernelName, const gtUInt64& line), (kernelName, line));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAListVariables, bool, (gtVector<gtString>& variables), (variables));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAGetVariableValue, bool, (const gtString& varName, gtString& varValue, gtString& varValueHex, gtString& varType), (varName, varValue, varValueHex, varType));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAListWorkItems, bool, (gtVector<gtUInt32>& o_gidLidWgid), (o_gidLidWgid));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSASetActiveWorkItemIndex, bool, (gtUInt32 wiIndex), (wiIndex));
GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS(gaHSAGetWorkDims, bool, (gtUByte& dims), (dims));

