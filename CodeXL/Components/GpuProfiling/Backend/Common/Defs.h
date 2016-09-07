//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Some common definitions.
//==============================================================================

#ifndef _DEFS_H_
#define _DEFS_H_

#include <string>
#include <cstring>
#include <map>
#include <unordered_set>
#include <assert.h>
#include "Version.h"
#include "../Common/Logger.h"
#include "AMDTBaseTools/Include/gtString.h"

using namespace GPULogger;


/// \defgroup Common Common
/// This module consists of the common functionalities used by other modules.
///
/// \ingroup Backend
// @{

#ifdef WIN32
   #if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        #define BITNESS "-x64"
        #define NUMBITS "64"
    #else
        #define BITNESS ""
        #define NUMBITS "32"
    #endif

    #define LIB_PREFIX ""
    #define LIB_SUFFIX ".dll"
#else
    #ifdef X86
        #define BITNESS "32"
        #define NUMBITS "32"
    #else
        #define BITNESS ""
        #define NUMBITS "64"
    #endif

    #define LIB_PREFIX "lib"
    #define LIB_SUFFIX ".so"
#endif

#ifdef _DEBUG
    #define DEBUG_PREFIX "-d"
#else
    #define DEBUG_PREFIX
#endif

#define GPU_PROFILER_LIBRARY_NAME_PREFIX "CXLGpuProfiler"
#define CL_TRACE_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "CLTraceAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#define CL_SUB_KERNEL_PROFILE_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "CLSubKernelProfileAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#define CL_PROFILE_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "CLProfileAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#define CL_OCCUPANCY_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "CLOccupancyAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#define CL_THREAD_TRACE_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "CLThreadTraceAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX

#define HSA_TRACE_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "HSATraceAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#define HSA_PROFILE_AGENT_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "HSAProfileAgent" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX

#ifdef WIN32
   #if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        #define HSA_RUNTIME_TOOLS_LIB "hsa-runtime-tools64.dll"
    #else
        #define HSA_RUNTIME_TOOLS_LIB "hsa-runtime-tools.dll"
    #endif
#else
    #ifdef X86
        #define HSA_RUNTIME_TOOLS_LIB "libhsa-runtime-tools.so.1"
    #else
        #define HSA_RUNTIME_TOOLS_LIB "libhsa-runtime-tools64.so.1"
    #endif
#endif

#ifdef WIN32
    #define DC_SERVER_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "DCServer" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
    #define MICRO_DLL LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "MicroDll" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#elif defined (__linux__) || defined (_LINUX) || defined(LINUX)
    #define PRELOAD_XINITTHREADS_LIB LIB_PREFIX GPU_PROFILER_LIBRARY_NAME_PREFIX "PreloadXInitThreads" BITNESS DEBUG_PREFIX GDT_BUILD_SUFFIX LIB_SUFFIX
#endif

#ifdef SUPPORT_HSA_GRANITE
    #define HSA_ENABLE_PROFILING_ENV_VAR "HSA_AGENT"
#else
    #define HSA_ENABLE_PROFILING_ENV_VAR "HSA_TOOLS_LIB"
#endif

#define OCL_ENABLE_PROFILING_ENV_VAR "CL_AGENT"

#define STRLEN 100

#ifndef ULONGLONG
    typedef unsigned long long ULONGLONG;
#endif

#ifdef WIN32
    #define SP_MAX_PATH MAX_PATH
#else
    #define SP_MAX_PATH 4096
#endif

/// max number of command line arguments we support in Linux build
#define SP_MAX_ARG 128
/// max number of environment variables we support in Linux build
#define SP_MAX_ENVVARS 256

#define SP_MAX_ENVVAR_SIZE 8191 // max length for Windows, seems reasonably long for Linux

/// max number of properties to show when dereferencing the properties parameter for clCreateContext, clCreateContextFromType and clGetGLContextInfoKHR
#define SP_MAX_NUM_CONTEXT_PROPERTIES 64

#ifndef _DEBUG
    #define DISABLE_LOG
#endif

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_FREE
    #define SAFE_FREE(p) { if (p) { free(p); p=NULL; } }
#endif

#ifndef SAFE_DELETE
    #define SAFE_DELETE(p) { if (p) { delete p; p=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
    #define SAFE_DELETE_ARRAY(p) { if (p) { delete [] p; p=NULL; } }
#endif

#if defined (_WIN32)
    #define SP_UNREFERENCED_PARAMETER( x ) ( x )
#elif defined (__linux__) || defined (__CYGWIN__) || defined (_LINUX) || defined(LINUX)
    #define SP_UNREFERENCED_PARAMETER( x ) (void)(x)
#endif

#ifdef _WIN32
    //#define USE_TEXT_WRITER
#endif

#define hidden_quote( s ) #s
#define hidden_numquote( n ) hidden_quote( n )

#ifndef SUPPRESS_TODO_MSG
    #if defined (_WIN32)

        #define SP_TODO(x)  __pragma( message( __FILE__ "(" hidden_numquote( __LINE__ ) "): TODO: " x ) )

    #elif defined (__linux__) || defined (_LINUX) || defined(LINUX)

        // Macros do not seem to directly expand on Linux in #pragma statements
        #define DO_PRAGMA(x)    _Pragma(#x)
        #define SP_TODO(x)  DO_PRAGMA( message( __FILE__ "(" hidden_numquote( __LINE__ ) "): TODO: " x ) )

    #elif defined (__CYGWIN__)

        #define SP_TODO(x)

    #endif
#else
    #define SP_TODO(x)
#endif

#ifndef DLL_PUBLIC
    #if defined _WIN32 || defined __CYGWIN__
        #ifdef __GNUC__
            #define DLL_PUBLIC __attribute__ ((dllexport))
        #else
            #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
        #endif
        #ifndef DLL_LOCAL
            #define DLL_LOCAL
        #endif
    #else
        #if __GNUC__ >= 4
            #define DLL_PUBLIC __attribute__ ((visibility ("default")))
            #ifndef DLL_LOCAL
                #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
            #endif
        #else
            #define DLL_PUBLIC
            #ifndef DLL_LOCAL
                #define DLL_LOCAL
            #endif
        #endif
    #endif
#endif //DLL_PUBLIC

#define SPACE "&nbsp;"

#define TMP_TIME_STAMP_EXT ".tstamp"
#define TMP_GPU_TIME_STAMP_RAW_EXT ".ocltstampraw"
#define TMP_KERNEL_TIME_STAMP_EXT ".kerneltstamp"
#define TMP_TRACE_EXT ".apitrace"
#define TMP_TRACE_STACK_EXT ".stfrag"
#define TMP_OCCUPANCY_EXT ".occupancyfrag"
#define TRACE_STACK_EXT ".st"
#define PERFMARKER_EXT ".amdtperfmarker"
#define OCCUPANCY_EXT "occupancy"
#define TRACE_EXT "atp"
#define OBJECT_EXT "aop"
#define PERF_COUNTER_EXT "csv"
#define DEFAULT_OUTPUT_FILE "session1"
#define MEM_TRACE_RAWDATA_EXT ".mtr"
#define IL_PATH_ANALYZER_EXT ".ilpath"
#define KERNEL_ASSEMBLY_FILE_PREFIX "sp_tmp."

#define DEFAULT_TIMEOUT_INTERVAL 100

#define DEFAULT_MAX_NUM_OF_API_CALLS 1000000
#define DEFAULT_MAX_KERNELS 100000

#define TEST_EXCEPTION_EXIT_CODE 0xBAD99999

// Init extension function pointers
#define INIT_CL_EXT_FCN_PTR(name) \
    if(m_instance.name == NULL) { \
        if (g_realDispatchTable.GetExtensionFunctionAddressForPlatform != NULL) { \
            m_instance.name = (cl##name##Proc) g_realDispatchTable.GetExtensionFunctionAddressForPlatform(CLUtils::GetDefaultPlatform(), "cl"#name); \
        } \
        else if (g_realDispatchTable.GetExtensionFunctionAddress != NULL) \
        { \
            m_instance.name = (cl##name##Proc) g_realDispatchTable.GetExtensionFunctionAddress("cl"#name); \
        } \
        else \
        { \
            assert(!"Unable to call GetExtensionFunctionAddressForPlatform or GetExtensionFunctionAddress"); \
        } \
    }

///
#define CHECK_SS_ERROR( ss )  if( ss.fail() )   \
    {                 \
        Log( GPULogger::logWARNING, "Failed to parse string.\n" ); \
        return false;  \
    }

#define TRY_READ( fin, buf ) memset( buf, 0, BUFSIZE );       \
    fin.getline ( buf, BUFSIZE );    \
    if (fin.fail())                  \
    {                                \
        if(fin.eof())                 \
        {                             \
            return true;               \
        }                             \
        else                          \
        {                             \
            GPULogger::Log( logERROR, "Error bit set in istream.\n" );   \
            return false;              \
        }                             \
    }

#define CASE(x) case x: return #x;
#define CASESTR(x) case x: ss << #x; break;

// Call clGetEventInfo to retrieve ref count in Trace_func
// Create overhead, debug use only
//#define _DEBUG_REF_COUNT_

/// Map of the current environment variables
typedef std::map<gtString, gtString> EnvVarMap;

/// List of kernels to be profiled
typedef std::unordered_set<std::string> KernelFilterList;

/// Parameters passed from CodeXLGpuProfiler to servers
struct Parameters
{
    /// Default Constructor
    Parameters()
    {
        m_strOutputFile.clear();
        m_strSessionName.clear();
        m_strDLLPath = L"";
        m_strCounterFile.clear();
        m_strKernelFile.clear();
        m_strCmdArgs = L"";
        m_strWorkingDir = L"";
        m_strTimerDLLFile.clear();
        m_strUserTimerFn.clear();
        m_strUserTimerInitFn.clear();
        m_strUserTimerDestroyFn.clear();
        m_strUserPMCLibPath.clear();
        m_bVerbose = false;
        m_bOutputIL = false;
        m_bOutputISA = false;
        m_bOutputCL = false;
        m_bOutputASM = false;
        m_bOutputHSAIL = false;
        m_bPerfCounter = false;
        m_bTrace = false;
        m_bHSATrace = false;
        m_bHSAPMC = false;
        m_bTimeOutBasedOutput = true;
        m_uiTimeOutInterval = DEFAULT_TIMEOUT_INTERVAL;
        m_bFullEnvBlock = false;
        m_cOutputSeparator = '\0';
        m_bTestMode = false;
        m_bUserTimer = false;
        m_bQueryRetStat = false;
        m_bCollapseClGetEventInfo = true;
        m_bStackTrace = false;
        m_bKernelOccupancy = false;
        m_uiVersionMajor = GPUPROFILER_BACKEND_MAJOR_VERSION;
        m_uiVersionMinor = GPUPROFILER_BACKEND_MINOR_VERSION;
        m_uiMaxNumOfAPICalls = DEFAULT_MAX_NUM_OF_API_CALLS;
        m_uiMaxKernels = DEFAULT_MAX_KERNELS;
        m_bUserPMC = false;
        m_bCompatibilityMode = false;
        m_bGMTrace = false;
        m_bForceSinglePassPMC = false;
        m_bGPUTimePMC = false;
        m_bStartDisabled = false;
        m_bForceSingleGPU = false;
        m_uiForcedGpuIndex = 0;
    }

    unsigned int m_uiVersionMajor;      ///< Version major
    unsigned int m_uiVersionMinor;      ///< Version minor
    gtString m_strCmdArgs;              ///< command line arguments
    gtString m_strWorkingDir;           ///< working directory
    std::string m_strOutputFile;        ///< output file path string
    std::string m_strSessionName;       ///< session name string
    gtString m_strDLLPath;              ///< GPUPerfAPIDLL path
    std::string m_strCounterFile;       ///< Counter file
    std::string m_strKernelFile;        ///< Kernel list file
    std::string m_strAPIFilterFile;     ///< API Filter file
    std::string m_strTimerDLLFile;      ///< User timer DLL file (including path)
    std::string m_strUserTimerFn;       ///< User timer function name
    std::string m_strUserTimerInitFn;   ///< User timer initialization function (name)
    std::string m_strUserTimerDestroyFn;///< User timer destroy function (name)
    std::string m_strUserPMCLibPath;    ///< User PMC sampler module path
    bool m_bVerbose;                    ///< verbose option
    bool m_bOutputIL;                   ///< output OpenCL kernel IL file option
    bool m_bOutputHSAIL;                ///< output Kernel HSAIL file option
    bool m_bOutputISA;                  ///< output OpenCL kernel ISA file option
    bool m_bOutputCL;                   ///< output OpenCL kernel CL file option
    bool m_bOutputASM;                  ///< output DirectCompute shader ASM file option
    bool m_bPerfCounter;                ///< enable CL/DirectCompute performance counter mode option
    bool m_bTrace;                      ///< enable CL trace mode option
    bool m_bTimeOutBasedOutput;         ///< timeOut-based output model
    bool m_bHSATrace;                   ///< enable HSA trace mode option
    bool m_bHSAPMC;                     ///< enable HSA performance counter mode option
    unsigned int m_uiTimeOutInterval;   ///< Timeout interval
    EnvVarMap m_mapEnvVars;             ///< an environment block for the profiled app
    bool m_bFullEnvBlock;               ///< flag indicating whether or not the strEnvBlock represents a full environment block
    char m_cOutputSeparator;            ///< output file separator character
    bool m_bTestMode;                   ///< internal test mode flag
    bool m_bUserTimer;                  ///< internal mode to use the user timer rather than the default Win32 timers
    bool m_bQueryRetStat;               ///< Always query cl function status
    bool m_bCollapseClGetEventInfo;     ///< Collapse consecutive, identical clGetEventInfo calls into a single call
    bool m_bStackTrace;                 ///< Stack trace
    bool m_bKernelOccupancy;            ///< Flag to signal whether to record kernel occupancy
    bool m_bUserPMC;                    ///< flag indicating whether or not user PMC sampler callbacks are invoked during CPU timestamp read.
    bool m_bCompatibilityMode;          ///< flag indicating whether or not compatibility mode is enabled
    unsigned int m_uiMaxNumOfAPICalls;  ///< Maximum number of API calls
    unsigned int m_uiMaxKernels;        ///< maximum number of kernels to profile.
    bool m_bGMTrace;                    ///< Flag indicating whether or not global memory trace is enabled
    bool m_bForceSinglePassPMC;         ///< Flag indicating that only a single pass should be allowed when collecting performance counters
    bool m_bGPUTimePMC;                 ///< Flag indicating whether or not the profiler should collect gpu time when collecting perf counters
    bool m_bStartDisabled;              ///< Flag indicating whether or not to start with profiling disabled
    KernelFilterList m_kernelFilterList;///< List of kernels to filter for perf counter profiling and subkernel profiling
    bool m_bForceSingleGPU;             ///< Flag indicating whether or not to force a single GPU
    unsigned int m_uiForcedGpuIndex;    ///< Forced GPU index
};

typedef std::map<std::string, bool> AnalyzerMap;

/// Templated Less<T> for comparing member variables
template <class Class, typename Type, Type Class::*PtrToMem>
struct MemberCmp
{
    bool operator()(const Class* left, const Class* right)
    {
        return left->*PtrToMem < right->*PtrToMem;
    }
};

struct AnalyzeOps
{
    std::string strAtpFile;             ///< Input Atp file
    std::string strCsvFile;             ///< Input csv file
    bool bContextSummary;               ///< Indicating whether we generate summary page for context
    bool bTop10KernelSummary;           ///< Indicating whether we generate summary page for top 10 kernel
    bool bTop10DataTransferSummary;     ///< Indicating whether we generate summary page for top 10 data transfer
    bool bAPISummary;                   ///< Indicating whether we generate summary page for API
    bool bKernelSummary;                ///< Indicating whether we generate summary page for kernel summary
    bool bBestPractices;                ///< Indicating whether we generate best practices page
    AnalyzerMap analyzerMap;            ///< Analyzer map

    AnalyzeOps()
    {
        bContextSummary = bTop10DataTransferSummary = bTop10KernelSummary = bAPISummary = bKernelSummary = bBestPractices = true;
        strAtpFile.clear();
        strCsvFile.clear();
    }
};

// @}

#endif
