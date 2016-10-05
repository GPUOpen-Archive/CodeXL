//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the function to parse the command line using Boost
//==============================================================================

#pragma warning ( push )
#pragma warning(disable:4996)
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#pragma warning ( pop )

#include "AMDTOSWrappers/Include/osFilePath.h"

#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include "../Common/Version.h"
#include "ParseCmdLine.h"
#include "../Common/StringUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/LocaleSetting.h"
#include "../Common/Defs.h"
#include "../Common/GPAUtils.h"
#include "../CLCommon/CLUtils.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "DeviceInfoUtils.h"
#include <ADLUtil/ADLUtil.h>
#include "OpenCLModule.h"

#if defined (_LINUX) || defined (LINUX)
    #include "HSAUtils.h"
#endif

namespace po = boost::program_options;
using namespace std;

struct DeviceInfo
{
    unsigned int m_vendorId;            ///< vendor Id of the device
    unsigned int m_deviceId;            ///< device Id
    unsigned int m_revId;               ///< revision Id of the device
    std::string m_deviceCALName;        ///< CAL device name
    GDT_HW_GENERATION m_generation;     ///< generation of the device

    DeviceInfo()
    {
        m_vendorId = 0x1002; //AMD_VENDOR_ID
        m_deviceId = 0;
        m_revId = REVISION_ID_ANY;
    }
};

struct CounterPassInfo
{
    DeviceInfo m_deviceInfo;            ///< device Info
    CounterList m_counterList;          ///< list of the counter for the device
    unsigned int m_numberOfPass;        ///< number of pass for the counters and selected deviceId

    CounterPassInfo()
    {
        m_deviceInfo = DeviceInfo();
        m_numberOfPass = 0;
    }
};


void PrintCounters(const std::string& strOutputFile, const bool shouldIncludeCounterDescriptions = false);
void PrintCounters(CounterList& counterList, const string& strGenerationName, const std::string& outputFile, const bool shouldIncludeCounterDescriptions = false);
std::string GetCounterListOutputFileName(const std::string& strOutputFile, const std::string& strAPI, const std::string& strGeneration);
std::string RemoveGroupFromCounterDescriptionIfPresent(std::string counterDescription);
inline void ShowExamples();
void PrintNumberOfPass(const std::string counterFile, const bool& gpuTimePMCEnabled);
std::vector<DeviceInfo> RemoveDuplicateDevice(std::vector<DeviceInfo> deviceInfoList);
inline bool CompareDeviceInfo(DeviceInfo first, DeviceInfo second);
void PrintCounterList(CounterList counterList);
std::vector<DeviceInfo> GetDeviceInfoList(GPA_API_Type);
std::vector<CounterPassInfo> GetNumberOfPassForAPI(GPA_API_Type apiType, CounterList counterList);
std::vector<CounterPassInfo> GetNumberOfPassFromGPUPerfAPI(GPA_API_Type apiType, CounterList counterList, std::vector<DeviceInfo> deviceInfoList);
std::vector<CounterList> GetCounterListsByMaxPassForEachDevice(GPA_API_Type apiType, CounterPassInfo counterPassInfo, unsigned int maxPass, CounterList& leftCounterList);
void ListCounterToFileForMaxPass(std::string counterFile, unsigned int maxPass);

pair<string, string> Parser(const string& strOptionToParse);

// globals to track command line alongside boost
static bool s_bEncounteredPositional;
static string s_strInjectedApp, s_strInjectedAppArgs;
static po::options_description* s_pCmdline_options = NULL;
static string s_profilerTitleVersion = "CodeXL GPU Profiler (" GPUPROFILER_BACKEND_EXE_NAME ") " NUMBITS " bits - " GPUPROFILER_BACKEND_VERSION_STRING;

bool ParseCmdLine(int argc, wchar_t* argv[], Config& configOut)
{
    //----- Command Line Options Parsing
    try
    {
        // Parse command line options using Boost library
        po::options_description genericOpt("General options");
        genericOpt.add_options()
        ("startdisabled", "Start the application with profiling disabled. This is useful for applications that call amdtStopProfiling and amdtResumeProfiling from the AMDTActivityLogger library.")
        ("startdelay,d", po::value<unsigned int>()->default_value(0), "Profiling will be enabled after the specified delay (in milliseconds). This delay is relative to the first API call (either OpenCL or HSA) made by the application.")
        ("profileduration,D", po::value<unsigned int>()->default_value(0), "Profile duration in milliseconds.")
        ("envvar,e", po::value< std::vector<string> >()->multitoken(), "Environment variable that should be defined when running the profiled application. Argument should be in the format NAME=VALUE.")
        ("envvarfile,E", po::value<string>(), "Path to a file containing a list of environment variables that should be defined when running the profiled application. The file should contain one line for each variable in the format NAME=VALUE.")
        ("fullenv,f", "The environment variables specified with the envvar or envvarfile switch represent the full environment block.  If not specified, then the environment variables represent additions or changes to the system environment block.")
        ("list,l", "Print a list of valid counter names.")
        ("listdetailed,L", "Print a list of valid counter names with descriptions")
        ("sessionname,N", po::value<string>(), "Name of the generated session.")
        ("maxpassperfile", po::value<unsigned int>(), "Limits the set of counters in the generated counter file to a set that can be collected in the the specified number of passes. If the full set of counters do not fit in the specified number of passes, then multiple counter files will be generated. Ignored if --list and --outputfile are not specified.")
        ("numberofpass", "Print the number of passes required for the specified counter set (or the default set if no counter file is specified)")
#ifdef _WIN32
        ("outputfile,o", po::value<string>(), "Path to OutputFile (the default is Session1.csv in an \"CodeXL\" directory under the current user's Documents directory; when performing an API trace, the default is apitrace.atp in the same location).")
#else
        ("outputfile,o", po::value<string>(), "Path to OutputFile (the default is ~/Session1.csv when collecting performance counters; the default is ~/apitrace.atp when performing an API trace).")
#endif
        ("version,v", "Print the " GPUPROFILER_BACKEND_EXE_NAME " version number.")
        ("workingdirectory,w", po::value<string>(), "Set the working directory (the default is the app binary's path).")
        ("help,h", "Print this help message.");

        po::options_description profileModeOpt("Profile mode options");
        profileModeOpt.add_options()
        ("apitrace,t", "Trace OpenCL application and generate CPU and GPU time stamps and detailed API call traces.")
        ("perfcounter,p", "Get the performance counters for each OpenCL or DirectCompute kernel dispatched by the application.")
#if defined (_LINUX) || defined (LINUX)
        ("hsatrace,A", "Trace HSA application and generate CPU and GPU time stamps and detailed API call traces.")
        ("hsapmc,C", "Get the performance counters for each HSA kernel dispatched by the application.")
#endif
        ("occupancy,O", "Generate kernel occupancy information for each OpenCL kernel dispatched by the application.")
        ("occupancydisplay,P", po::value<string>(), "Path to configuration file to use to generate an occupancy display file.  The occupancy display file to generate should be specified with --outputfile.")
        ("tracesummary,T", "Generate summary pages from an input .atp file.")
#ifdef GDT_INTERNAL
        ("subkernel,S", "Sub-kernel profiling.")
        ("gmtrace", "Global memory access trace.")
#endif
#ifdef GDT_INTERNAL
        ("threadtrace,H", "Generate thread trace.")
#endif
        ;

        po::options_description apiTraceOpt("Application Trace mode options (for --apitrace or --hsatrace)");
        apiTraceOpt.add_options()
        ("apifilterfile,F", po::value<string>(), "Path to the API filter file which contains a list of OpenCL or HSA APIs to be filtered out when performing an API trace.")
#ifdef _WIN32
        ("interval,i", po::value<unsigned int>()->default_value(DEFAULT_TIMEOUT_INTERVAL), "Timeout interval in milliseconds. Ignored when not using timeout mode.")
#else
        ("interval,i", po::value<unsigned int>()->default_value(DEFAULT_TIMEOUT_INTERVAL), "Timeout interval in milliseconds.")
#endif
#ifdef _WIN32
        ("timeout,m", "Flush Trace data periodically, default timeout interval is 100 milliseconds (can be changed with -i option).")
#endif
        ("maxapicalls,M", po::value<unsigned int>()->default_value(1000000), "Maximum number of API calls.")
        ("nocollapse,n", "Do not collapse consecutive identical clGetEventInfo calls into a single call in the trace output.")
        ("ret,r", "Always include the OpenCL API return code in API trace even if client application doesn't query it.")
        ("sym,y", "Include symbol information for each API in the .atp file.");

        po::options_description perfCounterOpt("Performance Counter mode options (for --perfcounter or --hsapmc)");
        perfCounterOpt.add_options()
        ("counterfile,c", po::value< std::vector<string> >()->multitoken(), "Path to the counter file to enable selected counters (case-sensitive). If not provided, all counters will be used.")
        ("singlepass,g", "Only allow a single pass when collecting performance counters. Any counters that cannot fit into a single pass will be ignored. If specified, the GPUTime will not be collected, as a separate pass is required to query the GPUTime.")
        ("nogputime,G", "Skip collection of GPUTime when profiling a kernel (GPUTime requires a separate pass).")
        ("kerneloutput,k", po::value< std::vector<string> >()->multitoken(), "Output the specified kernel file. Valid argument values are: \n"
         "  il:    output kernel IL files\n"
         "  isa:   output kernel ISA files\n"
         "  cl:    output kernel CL files\n"
         "  hsail: output kernel HSAIL files\n"
#ifdef _WIN32
         "  asm:   output DirectCompute shader ASM files\n"
#endif
         "  all:   output all files")
#ifndef GDT_INTERNAL
        ("kernellistfile,K", po::value<string>(), "Path to the kernel list file which contains a case-sensitive list of kernels to profile. If not provided, all kernels will be profiled.")
#endif
        ("outputseparator,s", po::value<char>(), "Character used to separate fields in the OutputFile.")
        ("maxkernels,x", po::value<unsigned int>()->default_value(100000), "Maximum number of kernels to profile.")
#if defined (_LINUX) || defined (LINUX)
        ("xinitthreads", "Call XInitThreads at application startup. This can be a workaround for an assert that occurs when collecting performance counters")
#endif
        ;

        po::options_description traceSummaryOpt("Trace Summary mode options (for --tracesummary)");
        traceSummaryOpt.add_options()
        ("atpfile,a", po::value<string>(), "Path to the .atp file from which to generate summary pages. Optional when performing an API trace. Required if --tracesummary is specified when not performing an API trace.")
        ("apirulesfile,R", po::value<string>(), "Path to OpenCL API analyzer configuration file. If not specified, all rules are enabled.");

#ifdef GDT_INTERNAL
        po::options_description subkernelOrPerfCounterProfilingOpt("Performance Counter mode or Sub-kernel Profiling mode options (for --subkernel or --gmtrace)");
        subkernelOrPerfCounterProfilingOpt.add_options()
        ("kernellistfile,K", po::value<string>(), "Path to the kernel list file which contains a case-sensitive list of kernels to profile. If not provided, all kernels will be profiled.");
#endif

        po::options_description hiddenOpt("Internal (hidden) options");
        hiddenOpt.add_options()
        ("__testmode__", "Run in internal testing mode (for automated tests).")
        ("__mergemode__", po::value<unsigned int>(), "temp files prefix (process id).")
        ("__USER_TIMER__", po::value<string>(), "user supplied timer.")
        ("__user_pmc__", po::value<string>(), "user PMC sampler dynamic library path.")
        ("__compatibilitymode__", "Use compatibility mode. Stack trace and perfmarker files are output separately.")
#if defined (_LINUX) || defined (LINUX)
        ("__preload__", po::value<string>(), "Name of an optional library or libraries to preload in the application being profiled")
#endif
        ("__nodetours__", "Don't launch application using detours (will prevent DirectCompute from being profiled).")
        ("__hsaforcesinglegpu__", po::value<unsigned int>(), "Override HSA agent discovery to only expose a single GPU to the application. The argument is the device index.");

        // all options available from command line
        po::options_description cmdline_options;
        cmdline_options.add(genericOpt).add(profileModeOpt).add(apiTraceOpt).add(perfCounterOpt).add(traceSummaryOpt).add(hiddenOpt);

#ifdef GDT_INTERNAL
        cmdline_options.add(subkernelOrPerfCounterProfilingOpt);
#endif

        po::variables_map vm;

        // init variables to keep track of what is passed in as application command line
        s_bEncounteredPositional = false;
        s_strInjectedApp.clear();
        s_strInjectedAppArgs.clear();
        s_pCmdline_options = &cmdline_options;

        // Parse command line
        po::basic_parsed_options<wchar_t> programOptions = po::wcommand_line_parser(argc, argv).options(cmdline_options).allow_unregistered().extra_parser(Parser).run();
        std::map<string, wstring> unicodeOptionsMap;

        // Add all items to the map:
        std::vector< po::basic_option<wchar_t> >::iterator optionIterator = programOptions.options.begin();

        for (; optionIterator != programOptions.options.end(); optionIterator++)
        {
            if (0 != (*optionIterator).value.size())
            {
                if (unicodeOptionsMap.count((*optionIterator).string_key) == 0)
                {
                    unicodeOptionsMap[(*optionIterator).string_key] = (*optionIterator).value[0];
                }
                else
                {
                    unicodeOptionsMap[(*optionIterator).string_key] = unicodeOptionsMap[(*optionIterator).string_key] + L"," + (*optionIterator).value[0];
                }
            }
            else
            {
                // If there is no value add it otherwise do nothing
                if (unicodeOptionsMap.count((*optionIterator).string_key) == 0)
                {
                    unicodeOptionsMap[(*optionIterator).string_key] = L"";
                }
            }
        }

        SP_TODO("unicode parameters : --outputfile, --atpfile, --counterfile, --envvarfile, --apifilterfile, --apirulesfile, , --kernellistfile, --__USER_TIMER__ and --__user_pmc__, --occupancydisplay")
        SP_TODO("Pass the options to vm in newer boost that might support unicode parameters")
        // store(programOptions, vm);

        s_pCmdline_options = NULL;

        // Handle Options
        SP_TODO("Restore notify(vm) when boost version enables store(programOptions,vm) with a unicode version")
        // notify(vm);

        if (unicodeOptionsMap.count("version") > 0)
        {
            cout << s_profilerTitleVersion << endl;
            return false;
        }

        // get the counter files
        boost::filesystem::wpath counterFile;

        if (unicodeOptionsMap.count("counterfile") > 0)
        {
            std::wstring valueStr;
            valueStr = unicodeOptionsMap["counterfile"];
            boost::split(configOut.counterFileList, valueStr, boost::is_any_of(","));
        }

        // get the kernel file
        boost::filesystem::wpath kernelFile;

        if (unicodeOptionsMap.count("kernellistfile") > 0)
        {
            kernelFile = boost::filesystem::wpath(unicodeOptionsMap["kernellistfile"]);
            configOut.strKernelFile = kernelFile.string();
        }
        else
        {
            configOut.strKernelFile.clear();
        }

        // get the api filter file
        boost::filesystem::wpath filterFile;

        if (unicodeOptionsMap.count("apifilterfile") > 0)
        {
            filterFile = boost::filesystem::wpath(unicodeOptionsMap["apifilterfile"]);
            configOut.strAPIFilterFile = filterFile.string();
        }
        else
        {
            configOut.strAPIFilterFile.clear();
        }

        // check profile modes
        configOut.bTrace = unicodeOptionsMap.count("apitrace") > 0;
        configOut.bPerfCounter = unicodeOptionsMap.count("perfcounter") > 0;
        configOut.bHSATrace = unicodeOptionsMap.count("hsatrace") > 0;
        configOut.bHSAPMC = unicodeOptionsMap.count("hsapmc") > 0;
        configOut.bOccupancy = unicodeOptionsMap.count("occupancy") > 0;
        configOut.bSubKernelProfile = unicodeOptionsMap.count("subkernel") > 0;
        configOut.bGMTrace = unicodeOptionsMap.count("gmtrace") > 0;
        configOut.bThreadTrace = unicodeOptionsMap.count("threadtrace") > 0;

        configOut.bStartDisabled = unicodeOptionsMap.count("startdisabled") > 0;

        if (unicodeOptionsMap.count("maxapicalls") > 0)
        {
            wstring valueStr = unicodeOptionsMap["maxapicalls"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.uiMaxNumOfAPICalls = boost::lexical_cast<unsigned int>(valueStrConverted);
        }
        else
        {
            configOut.uiMaxNumOfAPICalls = DEFAULT_MAX_NUM_OF_API_CALLS;
        }

        if (unicodeOptionsMap.count("maxkernels") > 0)
        {
            wstring valueStr = unicodeOptionsMap["maxkernels"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.uiMaxKernels = boost::lexical_cast<unsigned int>(valueStrConverted);
        }
        else
        {
            configOut.uiMaxKernels = DEFAULT_MAX_KERNELS;
        }

        configOut.bQueryRetStat = unicodeOptionsMap.count("ret") > 0;
        configOut.bSym = unicodeOptionsMap.count("sym") > 0;

        configOut.bCollapseClGetEventInfo = unicodeOptionsMap.count("nocollapse") == 0;

#ifdef _WIN32
        configOut.bTimeOut = unicodeOptionsMap.count("timeout") > 0;
#else
        // timeout mode is default mode for linux
        configOut.bTimeOut = true;
#endif

        if (unicodeOptionsMap.count("interval") > 0)
        {
            wstring valueStr = unicodeOptionsMap["interval"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.uiTimeOutInterval = boost::lexical_cast<unsigned int>(valueStrConverted.c_str());
        }
        else
        {
            configOut.uiTimeOutInterval = DEFAULT_TIMEOUT_INTERVAL;
        }


        if (unicodeOptionsMap.count("startdelay") > 0)
        {
            wstring valueStr = unicodeOptionsMap["startdelay"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.m_delayInMilliseconds = boost::lexical_cast<unsigned int>(valueStrConverted.c_str());
        }
        else
        {
            configOut.m_delayInMilliseconds = 0;
        }

        if (unicodeOptionsMap.count("profileduration") > 0)
        {
            wstring valueStr = unicodeOptionsMap["profileduration"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.m_durationInMilliseconds = boost::lexical_cast<unsigned int>(valueStrConverted.c_str());
        }
        else
        {
            configOut.m_durationInMilliseconds = 0;
        }


        configOut.bOutputIL = false;
        configOut.bOutputISA = false;
        configOut.bOutputHSAIL = false;
        configOut.bOutputCL = false;
        configOut.bOutputASM = false;

        if (unicodeOptionsMap.count("kerneloutput") > 0)
        {
            wstring valueStr = unicodeOptionsMap["kerneloutput"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            std::vector<string> kernelFileTypes;
            boost::split(kernelFileTypes, valueStrConverted, boost::is_any_of(","));

            for (unsigned int i = 0; i < kernelFileTypes.size(); i++)
            {
                if (kernelFileTypes[i].compare("il") == 0)
                {
                    configOut.bOutputIL = true;
                }
                else if (kernelFileTypes[i].compare("isa") == 0)
                {
                    configOut.bOutputISA = true;
                }
                else if (kernelFileTypes[i].compare("cl") == 0)
                {
                    configOut.bOutputCL = true;
                }
                else if (kernelFileTypes[i].compare("hsail") == 0)
                {
                    configOut.bOutputHSAIL = true;
                }
                else if (kernelFileTypes[i].compare("asm") == 0)
                {
                    configOut.bOutputASM = true;
                }
                else if (kernelFileTypes[i].compare("all") == 0)
                {
                    configOut.bOutputIL = true;
                    configOut.bOutputISA = true;
                    configOut.bOutputCL = true;
                    configOut.bOutputASM = true;
                    configOut.bOutputHSAIL = true;
                }
            }
        }

        configOut.bForceSinglePassPMC = unicodeOptionsMap.count("singlepass") > 0;
        configOut.bGPUTimePMC = unicodeOptionsMap.count("nogputime") == 0;

        // get the output file
        boost::filesystem::path outputFile;

        configOut.bOccupancyDisplay = unicodeOptionsMap.count("occupancydisplay") > 0;

        if (configOut.bOccupancyDisplay == true)
        {
            wstring valueStr = unicodeOptionsMap["occupancydisplay"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            std::string strOccupancyOutFile = valueStrConverted;

            outputFile = boost::filesystem::path(strOccupancyOutFile.c_str());
            configOut.strOccupancyParamsFile = outputFile.string();
        }
        else
        {
            configOut.strOccupancyParamsFile.clear();
        }

        if (unicodeOptionsMap.count("outputfile") > 0)
        {
            wstring valueStr = unicodeOptionsMap["outputfile"];
            string valueStringConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStringConverted);
            outputFile = valueStringConverted;

            configOut.strOutputFile = outputFile.string();
        }
        else
        {
            configOut.strOutputFile.clear();
        }

        if (unicodeOptionsMap.count("sessionname") > 0)
        {
            wstring valueStr = unicodeOptionsMap["sessionname"];
            string valueStringConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStringConverted);
            configOut.strSessionName = valueStringConverted;
        }
        else
        {
            configOut.strSessionName.clear();
        }

        if (unicodeOptionsMap.count("apirulesfile") > 0)
        {
            wstring valueStr = unicodeOptionsMap["apirulesfile"];
            string valueStringConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStringConverted);
            configOut.strRulesConfigFile = valueStringConverted;
            FileUtils::LoadAPIRulesConfig(configOut.strRulesConfigFile, configOut.analyzeOps);
        }
        else
        {
            configOut.strRulesConfigFile.clear();
        }

        if (unicodeOptionsMap.count("outputseparator") > 0)
        {
            wstring valueStr = unicodeOptionsMap["outputseparator"];
            string valueStringConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStringConverted);

            if (valueStringConverted.length() > 1)
            {
                cout << "Invalid command line argument. Please specify a single character as the output separator." << endl;
                return false;
            }
            else
            {
                configOut.cOutputSeparator = valueStringConverted[0];
            }
        }
        else
        {
            configOut.cOutputSeparator = LocaleSetting::GetListSeparator();
        }

        // get the working directory
        boost::filesystem::wpath workingDirectory;

        if (unicodeOptionsMap.count("workingdirectory") > 0)
        {
            workingDirectory = boost::filesystem::wpath(unicodeOptionsMap["workingdirectory"].c_str());
            configOut.strWorkingDirectory = workingDirectory.wstring().c_str();
        }
        else
        {
            configOut.strWorkingDirectory = L"";
        }

        if (unicodeOptionsMap.count("envvarfile") > 0)
        {
            wstring valueStr = unicodeOptionsMap["envvarfile"];
            string valueStringConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStringConverted);
            // read the envvarfile, parse the name=value list, and store its contents in configOut.mapEnvVars
            std::string strEnvVarFile = boost::filesystem::path(valueStringConverted.c_str()).string();

            std::ifstream fin;
            fin.open(strEnvVarFile.c_str());

            if (fin.is_open())
            {
                try
                {
                    while (!fin.eof())
                    {
                        char envVar[SP_MAX_ENVVAR_SIZE];
                        fin.getline(envVar, SP_MAX_ENVVAR_SIZE);
                        std::string strEnvVar = envVar;
                        size_t equalPos = strEnvVar.find("=");
                        std::string varName;
                        std::string varValue;

                        if (equalPos != std::string::npos)
                        {
                            varName = strEnvVar.substr(0, equalPos);
                            varValue = strEnvVar.substr(equalPos + 1);
                        }
                        else
                        {
                            varName = strEnvVar;
                            varValue = "";
                        }

                        if (!varName.empty())
                        {
                            gtString strVarName;
                            gtString strVarValue;
                            strVarName.fromUtf8String(varName);
                            strVarValue.fromUtf8String(varValue);
                            configOut.mapEnvVars[strVarName] = strVarValue;
                        }
                    }
                }
                catch (...)
                {
                    // ignore any errors and ignore any additional values in the file
                }
            }
        }

        if (unicodeOptionsMap.count("envvar") > 0)
        {
            std::wstring valueStr = unicodeOptionsMap["envvar"];
            std::vector<std::wstring> envVals;
            boost::split(envVals, valueStr, boost::is_any_of(","));

            for (unsigned int i = 0; i < envVals.size(); i++)
            {
                std::wstring envVal = envVals[i];

                size_t equalPos = envVal.find('=');
                std::wstring varName;
                std::wstring varValue;

                if (equalPos != std::wstring::npos)
                {
                    varName = envVal.substr(0, equalPos);
                    varValue = envVal.substr(equalPos + 1);
                }
                else
                {
                    varName = envVal;
                    varValue = L"";
                }

                if (!varName.empty())
                {
                    gtString strVarName(varName.c_str());
                    gtString strVarValue(varValue.c_str());
                    configOut.mapEnvVars[strVarName] = strVarValue;
                }
            }
        }

        configOut.bFullEnvBlock = unicodeOptionsMap.count("fullenv") > 0;

#if defined (_LINUX) || defined (LINUX)
        configOut.strPreloadLib.clear();

        bool xInitThreads = unicodeOptionsMap.count("xinitthreads") > 0;

        if (xInitThreads)
        {
            std::string strPreloadLib = FileUtils::GetExePath();
            strPreloadLib += "/";
            strPreloadLib += PRELOAD_XINITTHREADS_LIB;

            configOut.strPreloadLib = strPreloadLib;
        }

        if (unicodeOptionsMap.count("__preload__") > 0)
        {
            wstring valueStr = unicodeOptionsMap["__preload__"];
            string valueStringConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStringConverted);
            outputFile = valueStringConverted;

            if (xInitThreads)
            {
                configOut.strPreloadLib += ":";
            }

            configOut.strPreloadLib += outputFile.string();
        }

#endif

        boost::filesystem::path atpFilePath;

        if (unicodeOptionsMap.count("tracesummary") > 0)
        {
            configOut.bAnalyze = true;
            configOut.bAnalyzeOnly = false; // init it to false first, check later

            if (configOut.bTrace || configOut.bHSATrace)
            {
                // if perform API trace, atp file will be given after cmdline parsing
                configOut.analyzeOps.strAtpFile.clear();
            }
            else
            {
                // otherwise, user need to provide atp file.
                if (unicodeOptionsMap.count("atpfile") > 0)
                {
                    wstring valueStr = unicodeOptionsMap["atpfile"];
                    string valueStrConverted;
                    StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
                    atpFilePath = boost::filesystem::path(valueStrConverted.c_str());
                    configOut.analyzeOps.strAtpFile = atpFilePath.string();
                }
                else if (0 == unicodeOptionsMap.count("help"))
                {
                    cout << "Invalid command line argument. Please specify input .atp file." << endl;
                    return false;
                }
            }
        }
        else
        {
            configOut.bAnalyze = false;
            configOut.bAnalyzeOnly = false;
        }

        configOut.bTestMode = unicodeOptionsMap.count("__testmode__") > 0;

        configOut.bMergeMode = unicodeOptionsMap.count("__mergemode__") > 0;

        if (configOut.bMergeMode)
        {
            wstring valueStr = unicodeOptionsMap["__mergemode__"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.uiPID = boost::lexical_cast<unsigned int>(valueStrConverted.c_str());
        }
        else
        {
            configOut.uiPID = 0;
        }

        //Check whether the user timer is to be used.  If it is, then load the appropriate
        //timer DLL and get the name of the user defined timer function.
        configOut.bUserTimer = unicodeOptionsMap.count("__USER_TIMER__") > 0;

        if (configOut.bUserTimer)
        {
            boost::filesystem::path TimerDLLFile;
            wstring valueStr = unicodeOptionsMap["__USER_TIMER__"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            string strTimerDLLFile = valueStrConverted;
            TimerDLLFile = boost::filesystem::path(strTimerDLLFile.c_str());
            configOut.strTimerDLLFile = TimerDLLFile.string();
            configOut.strUserTimerFn        = "GetTime";
            configOut.strUserTimerInitFn    = "InitTimer";
            configOut.strUserTimerDestroyFn = "DestroyTimer";
        }
        else
        {
            configOut.strTimerDLLFile.clear();
            configOut.strUserTimerFn.clear();
            configOut.strUserTimerInitFn.clear();
            configOut.strUserTimerDestroyFn.clear();
        }

        configOut.bUserPMCSampler = unicodeOptionsMap.count("__user_pmc__") > 0;

        if (configOut.bUserPMCSampler)
        {
            boost::filesystem::wpath dllPath;
            wstring strPath = unicodeOptionsMap["__user_pmc__"];
            dllPath = boost::filesystem::wpath(strPath.c_str());
            configOut.strUserPMCLibPath = dllPath.string();
        }
        else
        {
            configOut.strUserPMCLibPath.clear();
        }

        configOut.bCompatibilityMode = unicodeOptionsMap.count("__compatibilitymode__") > 0;

        configOut.bNoDetours = unicodeOptionsMap.count("__nodetours__") > 0;

        configOut.bForceSingleGPU = unicodeOptionsMap.count("__hsaforcesinglegpu__") > 0;

        if (configOut.bForceSingleGPU)
        {
            wstring valueStr = unicodeOptionsMap["__hsaforcesinglegpu__"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.uiForcedGpuIndex = boost::lexical_cast<unsigned int>(valueStrConverted.c_str());
        }

        if (unicodeOptionsMap.count("maxpassperfile") > 0)
        {
            wstring valueStr = unicodeOptionsMap["maxpassperfile"];
            string valueStrConverted;
            StringUtils::WideStringToUtf8String(valueStr, valueStrConverted);
            configOut.m_maxPassPerFile = boost::lexical_cast<unsigned int>(valueStrConverted.c_str());
        }
        else
        {
            configOut.m_maxPassPerFile = 0;

        }

        // check for "list" last so that other params are checked first (like output file)
        if (unicodeOptionsMap.count("list") > 0)
        {
            if (!configOut.strOutputFile.empty() && configOut.m_maxPassPerFile >= 1)
            {
                ListCounterToFileForMaxPass(configOut.strOutputFile, configOut.m_maxPassPerFile);
            }
            else
            {
                PrintCounters(configOut.strOutputFile);
            }

            return false;
        }


        if (unicodeOptionsMap.count("listdetailed") > 0)
        {
            PrintCounters(configOut.strOutputFile, true);
            return false;
        }

        if (unicodeOptionsMap.count("help") > 0)
        {
            // when user asks for help (CodeXLGpuProfiler --help), always display the general options
            // if they don't ask for help on a specific mode, then display the list of modes
            // if they ask for help on a specific mode, then display the help specific to that mode

            bool bModeSpecificHelp = configOut.bTrace || configOut.bHSATrace ||
                                     configOut.bPerfCounter || configOut.bHSAPMC ||
                                     configOut.bAnalyze
#ifdef GDT_INTERNAL
                                     || configOut.bSubKernelProfile || configOut.bGMTrace
#endif
                                     ;

            cout << s_profilerTitleVersion << endl;
            cout << "Usage: " << GPUPROFILER_BACKEND_EXE_NAME << " <options> ApplicationToProfile [arguments for application]" << endl << endl;
            cout << genericOpt << endl << endl;

            if (!bModeSpecificHelp)
            {
                cout << profileModeOpt << endl << endl;
                cout << "For more information use: --help <mode>" << endl
                     << "  for example: " GPUPROFILER_BACKEND_EXE_NAME " --help --apitrace" << endl;
            }

            if (configOut.bTrace || configOut.bHSATrace)
            {
                cout << apiTraceOpt << endl << endl;
            }

            if (configOut.bPerfCounter || configOut.bHSAPMC)
            {
                cout << perfCounterOpt << endl << endl;
#ifdef GDT_INTERNAL
                cout << subkernelOrPerfCounterProfilingOpt << endl << endl;
#endif
            }

            if (configOut.bAnalyze)
            {
                cout << traceSummaryOpt << endl << endl;
            }

#ifdef GDT_INTERNAL

            if (configOut.bSubKernelProfile || configOut.bGMTrace)
            {
                cout << subkernelOrPerfCounterProfilingOpt << endl << endl;
            }

#endif
            //Show examples when --help or -h option is used
            ShowExamples();

            return false;
        }

        if (unicodeOptionsMap.count("numberofpass") > 0)
        {
#ifdef GDT_INTERNAL

            if (configOut.counterFileList.empty())
            {
                std::cout << "A counter file is required in the internal build.  Please specify a counter file using the --counterfile option" << std::endl << std::endl;
                return false;
            }

#endif

            if (!configOut.counterFileList.empty())
            {
                for (unsigned int i = 0; i < configOut.counterFileList.size(); ++i)
                {
                    PrintNumberOfPass(configOut.counterFileList[i], configOut.bGPUTimePMC);
                }
            }
            else
            {
                std::string emptyCounterFile;
                PrintNumberOfPass(emptyCounterFile, configOut.bGPUTimePMC);
            }

            return false;
        }

        // All command line arguments that appear after a non-option are treated as app command line
        //  and have already been collected
        wstring wstrInjectedApp;
        StringUtils::Utf8StringToWideString(s_strInjectedApp, wstrInjectedApp);
        wstring wstrInjectedAppArgs;
        s_strInjectedAppArgs = StringUtils::Trim(s_strInjectedAppArgs);
        StringUtils::Utf8StringToWideString(s_strInjectedAppArgs, wstrInjectedAppArgs);

        configOut.strInjectedApp = wstrInjectedApp.c_str();

        configOut.strInjectedAppArgs = wstrInjectedAppArgs.c_str();

        if (wstrInjectedApp.empty())
        {
            if (configOut.bMergeMode || configOut.bOccupancyDisplay)
            {
                return true;
            }
            else if (configOut.bAnalyze)
            {
                configOut.bAnalyzeOnly = true;
                return true;
            }
            else
            {
                cout << "Invalid command line argument." << endl << endl;
                cout << "Usage: " << GPUPROFILER_BACKEND_EXE_NAME << " <options> ApplicationToProfile [arguments for application]" << endl << endl;
                cout << genericOpt << endl << endl
                     << profileModeOpt << endl << endl
                     << apiTraceOpt << endl << endl
                     << perfCounterOpt << endl << endl
#ifdef GDT_INTERNAL
                     << subkernelOrPerfCounterProfilingOpt << endl << endl
#endif
                     << traceSummaryOpt << endl << endl;

                //Show the examples when used with no options
                ShowExamples();
                return false;
            }
        }
    }
    catch (exception& e)
    {
        // Problem parsing options - report and exit
        cout << e.what() << endl;
        return false;
    }

    return true;
}

#if BOOST_VERSION == 103900
// custom parse to track command line options
pair<string, string> Parser(const string& s)
{
    if (s_bEncounteredPositional)
    {
        // we've already passed a positional option
        // so append to application command line
        // put arguments in quote
        s_strInjectedAppArgs += " " + s;
        // make boost skip this option
        return make_pair(string(" "), string(" "));
    }
    else if (s[0] != '-')
    {
        // it's not an option (i.e. it's a "positional option" )
        //  so start as application command line
        s_strInjectedApp = boost::filesystem::system_complete(boost::filesystem::path(s.c_str(), boost::filesystem::native).string()).string();
        s_strInjectedAppArgs = "";
        s_bEncounteredPositional = true;
        // make boost skip this option
        return make_pair(string(" "), string(" "));
    }
    else
    {
        // it's an option so let boost handle it
        return make_pair(string(), string());
    }
}
#else
// custom parse to track command line options
pair<string, string> Parser(const string& strOptionToParse)
{
    static bool s_ignoreNextToken = false;

    if (s_ignoreNextToken)
    {
        s_ignoreNextToken = false;
        // this token is a target of a previous option so let boost handle it
        return make_pair(string(), string());
    }

    if (s_bEncounteredPositional)
    {
        // we've already passed a positional option
        // so append to application command line
        string strProcessedOption = strOptionToParse;

        // put argument in quotes if it has a space in it
        if (strProcessedOption.find(" ") != string::npos)
        {
            strProcessedOption = "\"" + strProcessedOption + "\"";
        }

        s_strInjectedAppArgs += " " + strProcessedOption;
        // make boost skip this option
        return make_pair(string(" "), string(" "));
    }
    else if (strOptionToParse[0] != '-')
    {
        // it's not an option (i.e. it's a "positional option" )
        //  so start as application command line

        s_bEncounteredPositional = true;
        s_strInjectedApp = boost::filesystem::system_complete(boost::filesystem::path(strOptionToParse.c_str()).string()).string();
        s_strInjectedAppArgs = "";

        // make boost skip this option
        return make_pair(string(" "), string(" "));
    }

    string strOptionName = strOptionToParse;

    if (strOptionName.size() > 1 && strOptionName[0] == '-' && strOptionName[1] == '-')
    {
        //the find_nothrow call below doesn't like long options preceded by --, so remove the --
        // it WILL find a short option using a single dash, however
        strOptionName.erase(0, 2);
    }

    if (s_pCmdline_options != NULL)
    {
        const po::option_description* desc = s_pCmdline_options->find_nothrow(strOptionName, false);

        if (desc != NULL)
        {
            // if the option is found and it takes an argument, then the next token can not be the injected app -- it will be the argument for this option
            boost::shared_ptr<const po::value_semantic> vs = desc->semantic();
            s_ignoreNextToken = vs->min_tokens() > 0;
        }
        else
        {
            s_ignoreNextToken = false;
        }
    }

    // it's an option so let boost handle it
    return make_pair(string(), string());
}
#endif

/// Gets a string for the specified hw generation
/// \param gen the hardware generation
/// \param[out] strGenerationName the string for the generation name
/// \return true if a string is returned, false otherwise
bool GetGenerationName(GPA_HW_GENERATION gen, std::string& strGenerationName)
{
    GDT_HW_GENERATION generation = GDT_HW_GENERATION_NONE;

    switch (gen)
    {
        case GPA_HW_GENERATION_NVIDIA:
            generation = GDT_HW_GENERATION_NVIDIA;
            break;

        case GPA_HW_GENERATION_SOUTHERNISLAND:
            generation = GDT_HW_GENERATION_SOUTHERNISLAND;
            break;

        case GPA_HW_GENERATION_SEAISLAND:
            generation = GDT_HW_GENERATION_SEAISLAND;
            break;

        case GPA_HW_GENERATION_VOLCANICISLAND:
            generation = GDT_HW_GENERATION_VOLCANICISLAND;
            break;

        default:
            generation = GDT_HW_GENERATION_NONE;
            break;
    }

    bool retVal = AMDTDeviceInfoUtils::Instance()->GetHardwareGenerationDisplayName(generation, strGenerationName);

    if (!retVal)
    {
        strGenerationName.clear();
    }

    return retVal;

}

std::string GetCounterListOutputFileName(const std::string& strOutputFile, const std::string& strAPI, const std::string& strGeneration)
{
    std::string retVal = "";

    if (!strOutputFile.empty())
    {
        gtString outputFileName;
        outputFileName.fromUtf8String(strOutputFile.c_str());

        osFilePath outputFile(outputFileName);

        gtString baseFileName;
        outputFile.getFileName(baseFileName);

        gtString apiName;
        apiName.fromASCIIString(strAPI.c_str());

        std::string noSpaceGenName = StringUtils::Replace(strGeneration, " ", "_");
        gtString genName;
        genName.fromASCIIString(noSpaceGenName.c_str());

        baseFileName.appendFormattedString(L"_%ls_%ls", apiName.asCharArray(), genName.asCharArray());

        outputFile.setFileName(baseFileName);

        retVal = std::string(outputFile.asString().asUTF8CharArray());
    }

    return retVal;
}

// print a list of public counters
void PrintCounters(const std::string& strOutputFile, const bool shouldIncludeCounterDescriptions)
{
    gtString strDirPath = FileUtils::GetExePathAsUnicode();
    string strErrorOut;
    GPAUtils gpaUtils;
    CounterList counterList;

    bool shouldWriteToFile = !strOutputFile.empty();

    bool gpaInit = gpaUtils.InitGPA(GPA_API_OPENCL,
                                    strDirPath,
                                    strErrorOut,
                                    NULL,
                                    &counterList);

    if (gpaInit)
    {
        if (!shouldWriteToFile)
        {
            cout << "OpenCL performance counters:\n";
        }

        for (int gen = GPA_HW_GENERATION_SOUTHERNISLAND; gen < GPA_HW_GENERATION_LAST; gen++)
        {
            GPA_HW_GENERATION hwGen = static_cast<GPA_HW_GENERATION>(gen);

            if (gpaUtils.GetAvailableCounters(hwGen, counterList, shouldIncludeCounterDescriptions))
            {
                string strGenerationName;

                if (GetGenerationName(hwGen, strGenerationName))
                {
                    PrintCounters(counterList, strGenerationName, GetCounterListOutputFileName(strOutputFile, "OpenCL", strGenerationName), shouldIncludeCounterDescriptions);
                }
            }
        }

        gpaUtils.Unload();
    }

    gpaInit = gpaUtils.InitGPA(GPA_API_HSA,
                               strDirPath,
                               strErrorOut,
                               NULL,
                               &counterList);

    if (gpaInit)
    {
        if (!shouldWriteToFile)
        {
            cout << "HSA performance counters:\n";
        }

        for (int gen = GPA_HW_GENERATION_SEAISLAND; gen < GPA_HW_GENERATION_LAST; gen++)
        {
            GPA_HW_GENERATION hwGen = static_cast<GPA_HW_GENERATION>(gen);

            if (gpaUtils.GetAvailableCounters(hwGen, counterList, shouldIncludeCounterDescriptions))
            {
                string strGenerationName;

                if (GetGenerationName(hwGen, strGenerationName))
                {
                    PrintCounters(counterList, strGenerationName, GetCounterListOutputFileName(strOutputFile, "HSA", strGenerationName), shouldIncludeCounterDescriptions);
                }
            }
        }

        gpaUtils.Unload();
    }

#ifdef _WIN32
    gpaInit = gpaUtils.InitGPA(GPA_API_DIRECTX_11,
                               strDirPath,
                               strErrorOut,
                               NULL,
                               &counterList);

    if (gpaInit)
    {
        if (!shouldWriteToFile)
        {
            cout << "DirectCompute performance counters:\n";
        }

        for (int gen = GPA_HW_GENERATION_SOUTHERNISLAND; gen < GPA_HW_GENERATION_LAST; gen++)
        {
            GPA_HW_GENERATION hwGen = static_cast<GPA_HW_GENERATION>(gen);

            if (gpaUtils.GetAvailableCounters(hwGen, counterList, shouldIncludeCounterDescriptions))
            {
                gpaUtils.FilterNonComputeCounters(hwGen, counterList, shouldIncludeCounterDescriptions);
                string strGenerationName;

                if (GetGenerationName(hwGen, strGenerationName))
                {
                    PrintCounters(counterList, strGenerationName, GetCounterListOutputFileName(strOutputFile, "DirectCompute", strGenerationName), shouldIncludeCounterDescriptions);
                }
            }
        }

        gpaUtils.Unload();
    }

#endif
}

void PrintNumberOfPass(const std::string counterFile, const bool& gpuTimePMCEnabled)
{
    std::function<void(CounterPassInfo, std::string)> PrintCounterPassInfo =
        [](CounterPassInfo counterPassInfo, std::string api)
    {
        std::cout << "The following set of counters  " << std::endl << std::endl;
        PrintCounterList(counterPassInfo.m_counterList);
        std::cout << "\nrequires " << counterPassInfo.m_numberOfPass << " pass(es) for a(n) " << api.c_str() << " application." << std::endl << std::endl;
    };

    CounterList counterList;

    if (!counterFile.empty() && !FileUtils::ReadFile(counterFile, counterList, true, true))
    {
        std::cout << "Unable to read counter file" << std::endl;
        return;
    }

    //OpenCL
    std::vector<CounterPassInfo> counterPassInfiListForCL;
    counterPassInfiListForCL = GetNumberOfPassForAPI(GPA_API_OPENCL, counterList);

    for (std::vector<CounterPassInfo>::iterator i = counterPassInfiListForCL.begin(); i != counterPassInfiListForCL.end(); ++i)
    {
        PrintCounterPassInfo((*i), "OpenCL");
    }

    if (gpuTimePMCEnabled && !counterPassInfiListForCL.empty())
    {
        std::cout << "Note: GPUTime will take an additional pass.\n\n\n";
    }



#if defined (_LINUX) || defined (LINUX)
    //HSA

    std::vector<CounterPassInfo> counterPassInfiListForHSA;
    counterPassInfiListForHSA = GetNumberOfPassForAPI(GPA_API_HSA, counterList);

    for (std::vector<CounterPassInfo>::iterator i = counterPassInfiListForHSA.begin(); i != counterPassInfiListForHSA.end(); ++i)
    {
        PrintCounterPassInfo((*i), "HSA");
    }

#endif

#ifdef _WIN32

    std::vector<CounterPassInfo> counterPassInfiListForDC;
    counterPassInfiListForDC = GetNumberOfPassForAPI(GPA_API_DIRECTX_11, counterList);

    for (std::vector<CounterPassInfo>::iterator i = counterPassInfiListForDC.begin(); i != counterPassInfiListForDC.end(); ++i)
    {
        PrintCounterPassInfo((*i), "Direct Compute");
    }

#endif
}

// helper function
void PrintCounters(CounterList& counterList, const string& strGenerationName, const std::string& outputFile, const bool shouldIncludeCounterDescriptions)
{
    const unsigned int nLineBreak = 5;
    unsigned int curItem = 0;

    bool shouldWriteToFile = !outputFile.empty();
    std::ofstream fout;

    if (shouldWriteToFile)
    {
        fout.open(outputFile.c_str());

        if (!fout.is_open())
        {
            cout << "Unable to open " << outputFile << std::endl;
            return;
        }
    }
    else
    {
        cout << "The list of valid counters for " << strGenerationName << " based graphics cards:\n";
    }

    for (CounterList::iterator it = counterList.begin(); it != counterList.end(); ++it)
    {

        if (shouldWriteToFile)
        {
            if (!shouldIncludeCounterDescriptions)
            {
                fout << *it << std::endl;
            }
            else
            {
                fout << it->c_str();
                it++;
                fout << '\t' << RemoveGroupFromCounterDescriptionIfPresent(*it) << std::endl;
            }
        }
        else
        {
            cout << *it;

            if (shouldIncludeCounterDescriptions)
            {
                std::string description = RemoveGroupFromCounterDescriptionIfPresent((++it)->c_str());
                cout << "\t" << description;
            }

            if (*it != counterList.back())
            {
                cout << ", ";
            }

            if (shouldIncludeCounterDescriptions)
            {
                std::cout << std::endl;
            }
            else
            {
                // line break
                if (curItem && (curItem + 1) % nLineBreak == 0)
                {
                    cout << endl;
                    curItem = 0;
                }
                else
                {
                    curItem++;
                }
            }
        }
    }

    if (shouldWriteToFile)
    {
        fout.close();
        cout << "Counters written to " << outputFile << std::endl;
    }
    else
    {
        cout << endl << endl;
    }
}

//Helper Function
std::string RemoveGroupFromCounterDescriptionIfPresent(std::string counterDescription)
{
    std::vector<std::string> tempStringVector;
    StringUtils::Split(tempStringVector, counterDescription, "#", true, true);
    std::string trimmedString = tempStringVector.size() > 0 ? tempStringVector[tempStringVector.size() - 1 ] : NULL;
    return trimmedString;
}

#ifdef CL_TRACE_TEST

bool get_s_bEncounteredPositional()
{
    return s_bEncounteredPositional;
}

void set_s_bEncounteredPositional(bool val)
{
    s_bEncounteredPositional = val;
}

string get_s_strInjectedApp()
{
    return s_strInjectedApp;
}

void set_s_strInjectedApp(string val)
{
    s_strInjectedApp = val;
}

string get_s_strInjectedAppArgs()
{
    return s_strInjectedAppArgs;
}

void set_s_strInjectedAppArgs(string val)
{
    s_strInjectedAppArgs = val;
}

#endif


//Helper Function
inline void ShowExamples()
{
    std::cout << "Examples\n\n";

    std::cout << "\tAn example to collect OpenCL or DirectCompute performance counters: \n";
    std::cout << "\tCodeXLGpuProfiler - o \"/path/to/output.csv\" -p -w \"/path/to/app/working/directory\" \"/path/to/app.exe\" --device gpu\n\n";

    std::cout << "\tAn example to collect an OpenCL API trace:\n";
    std::cout << "\tCodeXLGpuProfiler -o \"/path/to/output.atp\" -t -w \"/path/to/app/working/directory\" \"/path/to/app.exe\" --device gpu\n\n";

    std::cout << "\tAn example to collect HSA performance counters (Linux only): \n";
    std::cout << "\tCodeXLGpuProfiler -o \"/path/to/output.csv\" -C -w \"/path/to/app/working/directory\" \"/path/to/app.exe\"\n\n";

    std::cout << "\tAn example to collect an HSA API trace(Linux only) :\n";
    std::cout << "\tCodeXLGpuProfiler -o \"/path/to/output.atp\" -A -w \"/path/to/app/working/directory\" \"/path/to/app.exe\"\n\n";

    std::cout << "\tAn example to collect an OpenCL API trace with summary pages:\n";
    std::cout << "\tCodeXLGpuProfiler -o \"/path/to/output.atp\" -t -T -w \"/path/to/app/working/directory\" \"/path/to/app.exe\" --device gpu\n\n";

    std::cout << "\tAn example to generate summary pages from an .atp file:\n";
    std::cout << "\tCodeXLGpuProfiler -a \"/path/to/output.atp\" -T\n\n";

    std::cout << "\tAn example to generate an occupancy display page:\n";
    std::cout << "\tCodeXLGpuProfiler -P \"/path/to/occupancy/params/file.txt\" -o \"path/to/output.html\"\n\n";
}

bool CompareDeviceInfo(DeviceInfo first, DeviceInfo second)
{
    return ((first.m_deviceId == second.m_deviceId) && (first.m_vendorId == second.m_vendorId) && (first.m_revId == second.m_revId));
};

std::vector<DeviceInfo> RemoveDuplicateDevice(std::vector<DeviceInfo> deviceInfoList)
{
    std::vector<DeviceInfo> tempDeviceInfoList;
    GDT_DeviceInfo deviceInfo;

    for (std::vector<DeviceInfo>::iterator iterOuter = deviceInfoList.begin(); iterOuter != deviceInfoList.end(); ++iterOuter)
    {
        if (!tempDeviceInfoList.empty())
        {
            for (unsigned int iterInner = 0; iterInner < tempDeviceInfoList.size(); ++iterInner)
            {
                if (!CompareDeviceInfo((*iterOuter), tempDeviceInfoList[iterInner]) &&
                    AMDTDeviceInfoUtils::Instance()->GetDeviceInfo((*iterOuter).m_deviceId,
                                                                   REVISION_ID_ANY,
                                                                   deviceInfo))
                {
                    tempDeviceInfoList.push_back(*iterOuter);
                }
            }
        }
        else
        {
            if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo((*iterOuter).m_deviceId,
                                                               REVISION_ID_ANY,
                                                               deviceInfo))
            {
                tempDeviceInfoList.push_back(*iterOuter);
            }
        }
    }

    return tempDeviceInfoList;
};

void PrintCounterList(CounterList counterList)
{
    unsigned int lineEnd = 5;
    unsigned int counterListSize = static_cast<unsigned int>(counterList.size());
    unsigned int counterListSizeMinusOne = counterListSize - 1;
    unsigned int i = 0;

    for (std::vector<std::string>::iterator it = counterList.begin(); it != counterList.end() ; ++it)
    {
        std::cout << (*it);
        lineEnd--;

        if (lineEnd == 0)
        {
            std::cout << ", " << std::endl;
            lineEnd = 5;
        }
        else
        {
            if (counterListSizeMinusOne == i)
            {
                std::cout << " ";
            }
            else
            {
                std::cout << ", ";
            }
        }

        i++;
    }
}

std::vector<CounterPassInfo> GetNumberOfPassForAPI(GPA_API_Type apiType, CounterList counterList)
{
    std::vector<DeviceInfo> deviceInfoList;
    std::vector<CounterPassInfo> counterInfoList;

    deviceInfoList = GetDeviceInfoList(apiType);

    if (!deviceInfoList.empty())
    {
        counterInfoList = GetNumberOfPassFromGPUPerfAPI(apiType, counterList, deviceInfoList);
    }

    return counterInfoList;
}


std::vector<DeviceInfo> GetDeviceInfoList(GPA_API_Type apiType)
{
    std::vector<DeviceInfo> deviceInfoList;

    if (apiType == GPA_API_OPENCL)
    {
        CLPlatformSet platformSet;
        InitRealCLFunctions();
        bool success = CLUtils::GetPlatformInfo(platformSet);
        std::string amdVendor = "Advanced Micro Devices, Inc.";

        bool isAMDDevice = false;

        if (success && !platformSet.empty())
        {
            for (std::set<CLPlatformInfo::platform_info,
                 CLPlatformInfo::CLPlatformInfoCompare>::iterator it = platformSet.begin();
                 it != platformSet.end() && !isAMDDevice; ++it)
            {
                isAMDDevice |= it->strPlatformVendor.compare(amdVendor) == 0;
            }

            if (isAMDDevice)
            {
                AsicInfoList asicInfoList;
                bool succeed = (AMDTADLUtils::Instance()->GetAsicInfoList(asicInfoList) == ADL_SUCCESS);

                DeviceInfo deviceInfo;

                if (succeed)
                {
                    GDT_GfxCardInfo cardInfo;

                    for (std::vector<ADLUtil_ASICInfo>::iterator it = asicInfoList.begin(); it != asicInfoList.end(); ++it)
                    {
                        deviceInfo.m_deviceId = it->deviceID;
                        deviceInfo.m_vendorId = it->vendorID;
                        deviceInfo.m_revId = it->revID;

                        if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceInfo.m_deviceId, deviceInfo.m_revId, cardInfo))
                        {
                            deviceInfo.m_deviceCALName = cardInfo.m_szCALName;
                            deviceInfo.m_generation = cardInfo.m_generation;
                            deviceInfoList.push_back(deviceInfo);
                        }
                    }

                    deviceInfoList = RemoveDuplicateDevice(deviceInfoList);
                }
                else
                {
                    OpenCLModule clModule;
                    clModule.LoadModule();

                    cl_platform_id* platformId = nullptr;
                    cl_uint number_platforms;
                    success &= CL_SUCCESS == clModule.GetPlatformIDs(NULL, platformId, &number_platforms);

                    if (success && (number_platforms >= 1))
                    {
                        platformId = new(std::nothrow) cl_platform_id[number_platforms];

                        success &= CL_SUCCESS == clModule.GetPlatformIDs(number_platforms, platformId, NULL);

                        for (unsigned int platformIter = 0; success && (platformIter < number_platforms); platformIter++)
                        {
                            cl_uint numberOfDevices = 0;
                            cl_uint numberOfEntries = 0;
                            success &= CL_SUCCESS == clModule.GetDeviceIDs(platformId[platformIter], CL_DEVICE_TYPE_GPU, numberOfEntries, NULL, &numberOfDevices);

                            if (success && (numberOfDevices >= 1))
                            {
                                cl_device_id* deviceIds = new(std::nothrow) cl_device_id[numberOfDevices];
                                success &= CL_SUCCESS == clModule.GetDeviceIDs(platformId[platformIter], CL_DEVICE_TYPE_GPU, numberOfDevices, deviceIds, NULL);

                                for (unsigned int deviceIdIter = 0; success && (deviceIdIter < numberOfDevices); deviceIdIter++)
                                {
                                    size_t paramValueSize;
                                    success &= CL_SUCCESS == clModule.GetDeviceInfo(deviceIds[deviceIdIter], CL_DEVICE_VENDOR, paramValueSize, nullptr, &paramValueSize);

                                    if (success && (paramValueSize > 0))
                                    {
                                        void* paramVendorName = malloc(paramValueSize);
                                        success &= CL_SUCCESS == clModule.GetDeviceInfo(deviceIds[deviceIdIter], CL_DEVICE_VENDOR, paramValueSize, paramVendorName, nullptr);
                                        std::string vendorName(reinterpret_cast<char*>(paramVendorName));

                                        if (success && (0 == vendorName.compare(amdVendor)))
                                        {
                                            success &= CL_SUCCESS == clModule.GetDeviceInfo(deviceIds[deviceIdIter], CL_DEVICE_NAME, paramValueSize, nullptr, &paramValueSize);

                                            if (success && (paramValueSize > 0))
                                            {
                                                void* paramDeviceName = malloc(paramValueSize);
                                                success &= CL_SUCCESS == clModule.GetDeviceInfo(deviceIds[deviceIdIter], CL_DEVICE_NAME, paramValueSize, paramDeviceName, nullptr);

                                                if (success)
                                                {
                                                    std::string deviceName(reinterpret_cast<char*>(paramDeviceName));
                                                    std::vector<GDT_GfxCardInfo> cardList;
                                                    AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), cardList);

                                                    if (cardList.size() > 0)
                                                    {
                                                        deviceInfo.m_deviceId = static_cast<unsigned int>(cardList.begin()->m_deviceID);
                                                        deviceInfo.m_revId = static_cast<unsigned int>(cardList.begin()->m_revID);
                                                        deviceInfo.m_deviceCALName = cardList.begin()->m_szCALName;
                                                        deviceInfo.m_generation = cardList.begin()->m_generation;
                                                        deviceInfoList.push_back(deviceInfo);
                                                    }
                                                }

                                                if (paramDeviceName)
                                                {
                                                    free(paramDeviceName);
                                                }
                                            }
                                        }

                                        if (paramVendorName)
                                        {
                                            free(paramVendorName);
                                        }
                                    }
                                }

                                if (nullptr != deviceIds)
                                {
                                    delete[] deviceIds;
                                }
                            }
                        }

                        if (nullptr != platformId)
                        {
                            delete[] platformId;
                        }
                    }

                    clModule.UnloadModule();
                }
            }
        }
    }

#if defined (_LINUX) || defined (LINUX)

    if (apiType == GPA_API_HSA)
    {
        HSADeviceIdList hsaDeviceList;

        if (HSAUtils::Instance()->GetHSADeviceIds(hsaDeviceList))
        {
            DeviceInfo deviceInfo;
            GDT_GfxCardInfo cardInfo;

            for (std::vector<uint32_t>::iterator it = hsaDeviceList.begin(); it != hsaDeviceList.end(); ++it)
            {
                deviceInfo.m_deviceId = (*it);

                if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceInfo.m_deviceId, deviceInfo.m_revId, cardInfo))
                {
                    deviceInfo.m_deviceCALName = cardInfo.m_szCALName;
                    deviceInfo.m_generation = cardInfo.m_generation;
                    deviceInfoList.push_back(deviceInfo);
                }
            }

            deviceInfoList = RemoveDuplicateDevice(deviceInfoList);
        }
    }

#endif

#ifdef _WIN32

    if (apiType == GPA_API_DIRECTX_11)
    {
        AsicInfoList asicInfoList;
        bool success = AMDTADLUtils::Instance()->GetAsicInfoList(asicInfoList) == ADL_SUCCESS;

        DeviceInfo deviceInfo;
        GDT_GfxCardInfo cardInfo;

        if (success)
        {
            for (std::vector<ADLUtil_ASICInfo>::iterator it = asicInfoList.begin(); it != asicInfoList.end(); ++it)
            {
                deviceInfo.m_deviceId = it->deviceID;
                deviceInfo.m_vendorId = it->vendorID;
                deviceInfo.m_revId = it->revID;

                if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceInfo.m_deviceId, deviceInfo.m_revId, cardInfo))
                {
                    deviceInfo.m_deviceCALName = cardInfo.m_szCALName;
                    deviceInfo.m_generation = cardInfo.m_generation;
                    deviceInfoList.push_back(deviceInfo);
                }
            }

            deviceInfoList = RemoveDuplicateDevice(deviceInfoList);
        }
    }

#endif
    return deviceInfoList;
}


std::vector<CounterPassInfo> GetNumberOfPassFromGPUPerfAPI(GPA_API_Type apiType, CounterList counterList, std::vector<DeviceInfo> deviceInfoList)
{
    gtString strDirPath = FileUtils::GetExePathAsUnicode();
    GPA_ICounterAccessor* ppCounterAccessor = nullptr;
    GPA_ICounterScheduler* ppCounterScheduler = nullptr;
    std::vector<CounterPassInfo> counterPassInfoList;
    CounterPassInfo counterPassInfo;
    GPA_GetAvailableCountersForDeviceProc get_GPAGetAvailableCountersForDeviceProc = nullptr;
    GPAUtils gpaUtils;

    GPUPerfAPICounterLoader::Instance()->LoadPerfAPICounterDll(strDirPath);
    get_GPAGetAvailableCountersForDeviceProc = GPUPerfAPICounterLoader::Instance()->GetGPAAvailableCountersForDeviceProc();

    if (nullptr != get_GPAGetAvailableCountersForDeviceProc)
    {
        for (std::vector<DeviceInfo>::iterator i = deviceInfoList.begin(); i != deviceInfoList.end(); ++i)
        {
            if (GPA_STATUS_OK == get_GPAGetAvailableCountersForDeviceProc(apiType,
                                                                          i->m_vendorId,
                                                                          i->m_deviceId,
                                                                          i->m_revId,
                                                                          &ppCounterAccessor,
                                                                          &ppCounterScheduler) &&
                ppCounterScheduler != nullptr &&
                ppCounterAccessor != nullptr &&
                GPA_STATUS_OK == ppCounterScheduler->SetCounterAccessor(ppCounterAccessor,
                                                                        i->m_vendorId,
                                                                        i->m_deviceId,
                                                                        i->m_revId))
            {
                bool success = true;
                unsigned int numPass = 0;
                bool considerNumberOfPassToBeOne = false;
                CounterList computeCounterList;
                std::vector<std::string> counterListForHSA;

                if (!counterList.empty())
                {
                    if (apiType == GPA_API_DIRECTX_11)
                    {
                        gpaUtils.FilterNonComputeCounters(static_cast<GPA_HW_GENERATION>(i->m_generation), counterList);
                    }

                    ppCounterScheduler->DisableAllCounters();
                    computeCounterList = counterList;
                }
                else
                {
                    unsigned int availableCounters = ppCounterAccessor->GetNumCounters();

                    for (unsigned int j = 0; j < availableCounters; ++j)
                    {
                        computeCounterList.push_back(ppCounterAccessor->GetCounterName(j));
                    }

                    if (apiType == GPA_API_DIRECTX_11)
                    {
                        gpaUtils.FilterNonComputeCounters(static_cast<GPA_HW_GENERATION>(i->m_generation), computeCounterList);
                    }

                    if (apiType == GPA_API_HSA)
                    {
                        considerNumberOfPassToBeOne = true;
                    }
                }

                if (considerNumberOfPassToBeOne)
                {
                    numPass = 1u;
                    ppCounterScheduler->DisableAllCounters();
                }

                for (std::vector<std::string>::const_iterator availableCountersIter = computeCounterList.begin(); availableCountersIter != computeCounterList.end(); ++availableCountersIter)
                {
                    uint32_t index = 0;
                    success &= ppCounterAccessor->GetCounterIndex(availableCountersIter->c_str(), &index);
                    success &= ppCounterScheduler->EnableCounter(index) == GPA_STATUS_OK;

                    if (considerNumberOfPassToBeOne)
                    {
                        unsigned int tempNumberPass;
                        success &= (GPA_STATUS_OK == ppCounterScheduler->GetNumRequiredPasses(&tempNumberPass));

                        if (success)
                        {
                            if (tempNumberPass == numPass)
                            {
                                counterListForHSA.push_back(availableCountersIter->c_str());
                            }
                            else
                            {
                                ppCounterScheduler->DisableCounter(index);
                            }
                        }
                    }
                }

                if (!considerNumberOfPassToBeOne)
                {
                    success &= (GPA_STATUS_OK == ppCounterScheduler->GetNumRequiredPasses(&numPass));
                }
                else
                {
                    computeCounterList.clear();
                    computeCounterList = counterListForHSA;
                }

                if (success)
                {
                    counterPassInfo.m_deviceInfo.m_vendorId = i->m_vendorId;
                    counterPassInfo.m_deviceInfo.m_deviceId = i->m_deviceId;
                    counterPassInfo.m_deviceInfo.m_revId = i->m_revId;
                    counterPassInfo.m_deviceInfo.m_deviceCALName = i->m_deviceCALName;
                    counterPassInfo.m_deviceInfo.m_generation = i->m_generation;
                    counterPassInfo.m_counterList = computeCounterList;
                    counterPassInfo.m_numberOfPass = numPass;
                    counterPassInfoList.push_back(counterPassInfo);
                }
            }
        }
    }

    return counterPassInfoList;
}


void ListCounterToFileForMaxPass(std::string counterFile, unsigned int maxPass)
{
    CounterList counterList;
    GPA_API_Type apiType;
    std::vector<CounterList> counterListByEachPass;
    CounterList leftCounterList;
    std::vector<CounterPassInfo> counterPassInfoList;
    std::string apiTypeString;

    auto CounterToFileForMaxPassLambda = [&]()
    {
        counterPassInfoList = GetNumberOfPassForAPI(apiType, counterList);

        for (unsigned int i = 0; i < counterPassInfoList.size(); ++i)
        {
            std::string deviceNameWithPass = counterPassInfoList[i].m_deviceInfo.m_deviceCALName + "_pass";
            counterListByEachPass = GetCounterListsByMaxPassForEachDevice(apiType, counterPassInfoList[i], maxPass, leftCounterList);

            for (unsigned int j = 0; j < counterListByEachPass.size(); ++j)
            {
                std::stringstream stringStream;
                std::string cardWithPass;
                stringStream << deviceNameWithPass << (j + 1);
                cardWithPass = GetCounterListOutputFileName(counterFile, apiTypeString, stringStream.str());
                std::string fullPath = cardWithPass;
                FileUtils::WriteFile(fullPath, counterListByEachPass[j]);
            }

            if (!leftCounterList.empty())
            {
                std::cout << "The following individual counters require more than " << maxPass << " pass(es) for " << apiTypeString << std::endl;
                PrintCounterList(leftCounterList);
            }
        }
    };

    // OpenCL
    apiType = GPA_API_OPENCL;
    apiTypeString = "OpenCL";

    CounterToFileForMaxPassLambda();

#if defined (_LINUX) || defined (LINUX)
    // HSA
    apiType = GPA_API_HSA;
    apiTypeString = "HSA";

    CounterToFileForMaxPassLambda();

#endif

#if defined _WIN32
    // Direct Compute
    apiType = GPA_API_DIRECTX_11;
    apiTypeString = "DC";

    CounterToFileForMaxPassLambda();

#endif
}


std::vector<CounterList> GetCounterListsByMaxPassForEachDevice(GPA_API_Type apiType, CounterPassInfo counterPassInfo, unsigned int maxPass, CounterList& leftCounterList)
{
    std::vector<CounterList> counterListEachPass;
    gtString strDirPath = FileUtils::GetExePathAsUnicode();
    GPA_ICounterAccessor* ppCounterAccessor = nullptr;
    GPA_ICounterScheduler* ppCounterScheduler = nullptr;
    GPA_GetAvailableCountersForDeviceProc getGPAGetAvailableCountersForDeviceProc = nullptr;

    bool success = false;

    success = GPUPerfAPICounterLoader::Instance()->LoadPerfAPICounterDll(strDirPath);
    getGPAGetAvailableCountersForDeviceProc = GPUPerfAPICounterLoader::Instance()->GetGPAAvailableCountersForDeviceProc();
    success &= (getGPAGetAvailableCountersForDeviceProc != nullptr);

    if (success && counterPassInfo.m_numberOfPass > maxPass)
    {
        CounterList handleCounters = counterPassInfo.m_counterList;

        if (GPA_STATUS_OK == getGPAGetAvailableCountersForDeviceProc(apiType,
                                                                     counterPassInfo.m_deviceInfo.m_vendorId,
                                                                     counterPassInfo.m_deviceInfo.m_deviceId,
                                                                     counterPassInfo.m_deviceInfo.m_revId,
                                                                     &ppCounterAccessor,
                                                                     &ppCounterScheduler) &&
            ppCounterScheduler != nullptr &&
            ppCounterAccessor != nullptr &&
            (GPA_STATUS_OK == ppCounterScheduler->SetCounterAccessor(ppCounterAccessor,
                                                                     counterPassInfo.m_deviceInfo.m_vendorId,
                                                                     counterPassInfo.m_deviceInfo.m_deviceId,
                                                                     counterPassInfo.m_deviceInfo.m_revId)))
        {
            for (unsigned int j = 0; j < counterPassInfo.m_numberOfPass; ++j)
            {
                ppCounterScheduler->DisableAllCounters();

                CounterList includeCountersInThisPass;
                CounterList excludeCountersInThisPass;

                bool succeed = false;

                for (unsigned int k = 0; k < handleCounters.size(); ++k)
                {
                    unsigned int index = 0;
                    unsigned int numberOfPass = 0;

                    succeed = ppCounterAccessor->GetCounterIndex(handleCounters[k].c_str(), &index);
                    succeed &= (GPA_STATUS_OK == ppCounterScheduler->EnableCounter(index));
                    succeed &= (GPA_STATUS_OK == ppCounterScheduler->GetNumRequiredPasses(&numberOfPass));

                    if (succeed && numberOfPass > maxPass)
                    {
                        succeed &= (GPA_STATUS_OK == ppCounterScheduler->DisableCounter(index));

                        if (succeed)
                        {
                            excludeCountersInThisPass.push_back(handleCounters[k]);
                        }
                    }
                    else
                    {
                        includeCountersInThisPass.push_back(handleCounters[k]);
                    }
                }

                if (succeed)
                {
                    if (!includeCountersInThisPass.empty())
                    {
                        counterListEachPass.push_back(includeCountersInThisPass);
                    }

                    handleCounters = excludeCountersInThisPass;

                    if (j == counterPassInfo.m_numberOfPass - 1 && !handleCounters.empty())
                    {
                        leftCounterList = handleCounters;
                    }
                }
            }
        }
    }
    else
    {
        counterListEachPass.push_back(counterPassInfo.m_counterList);
    }

    return counterListEachPass;
}
