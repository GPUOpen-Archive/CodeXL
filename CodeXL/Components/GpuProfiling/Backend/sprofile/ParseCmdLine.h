//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the function to parse the command line using Boost
//==============================================================================

#ifndef _PARSE_CMD_LINE_H_
#define _PARSE_CMD_LINE_H_

#include <map>
#include <string>
#include "../Common/Defs.h"

#include "AMDTBaseTools/Include/gtString.h"

/// \addtogroup sprofile
// @{

typedef std::vector<std::string> CounterFileList;

typedef struct
{
    gtString            strInjectedApp;                     ///< the injected app name
    gtString            strInjectedAppArgs;                 ///< the injected app argument list
    std::string         strOutputFile;                      ///< the output filename specified in the command line
    std::string         strSessionName;                     ///< the session name specified in the command line
    CounterFileList     counterFileList;                    ///< the counter filename specified in the command line
    std::string         strKernelFile;                      ///< the kernel list filename specified in the command line
    std::string         strAPIFilterFile;                   ///< the API filter file specified in the command line
    gtString            strWorkingDirectory;                ///< the injected app working directory
    std::string         strRulesConfigFile;                 ///< the OpenCL API Analyzer rules config file
    std::string         strTimerDLLFile;                    ///< the user timer DLL file, including path
    std::string         strUserTimerFn;                     ///< the name of the function passed from the user-supplied dll
    std::string         strUserTimerInitFn;                 ///< the name of the function to initialize the user timer function
    std::string         strUserTimerDestroyFn;              ///< the name of the function to destroy the user timer function
    std::string         strUserPMCLibPath;                  ///< the user PMC sampler module path
    std::string         strOSInfo;                          ///< the OS version being used
    std::string         strOccupancyParamsFile;             ///< the file storing the occupancy parameters for generating the HTML output
#if defined (_LINUX) || defined (LINUX)
    std::string         strPreloadLib;                      ///< list of libs to preload in the application being profiled
#endif
    bool                bAnalyze;                           ///< flag indicating whether or not post-process analyze is enabled
    bool                bAnalyzeOnly;                       ///< flag indicating whether or not only analyze module is enabled
    bool                bVerbose;                           ///< flag indicating whether or not verbose is enabled
    bool                bOutputIL;                          ///< flag indicating whether or not OpenCL kernel IL files are written out
    bool                bOutputHSAIL;                       ///< flag indicating whether or not Kernel HSAIL are written out
    bool                bOutputISA;                         ///< flag indicating whether or not OpenCL kernel ISA files are written out
    bool                bOutputCL;                          ///< flag indicating whether or not OpenCL kernel CL files are written out
    bool                bOutputASM;                         ///< flag indicating whether or not DirectCompute shader ASM files are written out
    char                cOutputSeparator;                   ///< the character used to separate fields in the output file
    bool                bTrace;                             ///< flag indicating which module to use, performance counter, API tracer or sub-kernel profiler.
    bool                bPerfCounter;                       ///< flag indicating the CL/DirectCompute performance counter agent to be used
    bool                bTimeOut;                           ///< flag indicating which mode to use
    bool                bQueryRetStat;                      ///< flag indicating whether to always query return status
    bool                bCollapseClGetEventInfo;            ///< flag indicating whether consecutive identical calls to clGetEventInfo should be collapsed
    bool                bSubKernelProfile;                  ///< flag indicating which module to use, performance counter, API tracer or sub-kernel profiler.
    bool                bGMTrace;                           ///< flag indicating whether or not global memory trace is enabled
    bool                bTestMode;                          ///< flag indicating that we're running automated tests. [Hidden option, INTERNAL]
    bool                bMergeMode;                         ///< flag indicating that we're doing merge of temp files. [Hidden option, INTERNAL]
    bool                bUserTimer;                         ///< flag indicating that we want to use the TSC time [Hidden option, INTERNAL]
    bool                bUserPMCSampler;                    ///< flag indicating whether or not user PMC sampler callbacks are invoked during CPU timestamp read. [Hidden option, INTERNAL]
    unsigned int        uiPID;                              ///< process id, tmp file prefix (Used in merge mode only)
    unsigned int        uiTimeOutInterval;                  ///< Timeout interval
    EnvVarMap           mapEnvVars;                         ///< an environment block for the profiled app (zero separated, double-zero terminated)
    bool                bFullEnvBlock;                      ///< flag indicating whether or not the strEnvBlock represents a full environment block
    bool                bSym;                               ///< flag indicating whether or not symbol information will be generated
    AnalyzeOps          analyzeOps;                         ///< switches for sanalyze
    unsigned int        uiMaxNumOfAPICalls;                 ///< maximum number of API calls.
    unsigned int        uiMaxKernels;                       ///< maximum number of kernels to profile.
    bool                bOccupancy;                         ///< flag to indicate whether or not occupancy information file will be generated
    bool                bOccupancyDisplay;                  ///< flag indicating that CodeXLGpuProfiler will generate an occupancy HTML output file
    bool                bThreadTrace;                       ///< flag indicating whether or not thread trace is enabled
    bool                bHSATrace;                          ///< flag indicating whether or not HSA trace is enabled
    bool                bHSAPMC;                            ///< flag indicating whether or not HSA performance counter is enabled
    bool                bCompatibilityMode;                 ///< flag indicating whether or not compatibility mode is enabled
    bool                bNoDetours;                         ///< flag indicating that application should not be launched using detours
    bool                bForceSinglePassPMC;                ///< flag indicating that only a single pass should be allowed when collecting performance counters
    bool                bGPUTimePMC;                        ///< flag indicating whether or not the profiler should collect gpu time when collecting perf counters
    bool                bStartDisabled;                     ///< flag indicating whether or not to start with profiling disabled
    unsigned int        m_delayInMilliseconds;              ///< delay for profiler in milliseconds
    unsigned int        m_durationInMilliseconds;           ///< duration for profiler in milliseconds for which profiler should run
    unsigned int        m_maxPassPerFile;                   ///< maximum pass for generating counter files
} Config;

/// Parse the command line arguments
/// \param[in]  argc      the number of arguments
/// \param[in]  argv      the array of argument strings
/// \param[out] configOut the output config structure
/// \return true if successful, false otherwise
bool ParseCmdLine(int argc, wchar_t* argv[], Config& configOut);
std::pair<std::string, std::string> Parser(const std::string& s);
// @}

#ifdef CL_TRACE_TEST
    bool get_s_bEncounteredPositional();
    void set_s_bEncounteredPositional(bool val);
    std::string get_s_strInjectedApp();
    void set_s_strInjectedApp(std::string val);
    std::string get_s_strInjectedAppArgs();
    void set_s_strInjectedAppArgs(std::string val);
#endif

#endif
