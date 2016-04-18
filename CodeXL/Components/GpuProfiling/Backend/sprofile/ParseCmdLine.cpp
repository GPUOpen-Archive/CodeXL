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

#include <string>
#include <iostream>
#include <fstream>
#include "../Common/Version.h"
#include "ParseCmdLine.h"
#include "../Common/StringUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/LocaleSetting.h"
#include "../Common/Defs.h"
#include "../Common/GPAUtils.h"
#include "DeviceInfoUtils.h"

namespace po = boost::program_options;
using namespace std;

void PrintCounters();
void PrintCounters(CounterList& counterList, const string& strGenerationName);

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
        ("startdisabled,d", "Start the application with profiling disabled. This is useful for applications that call amdtStopProfiling and amdtResumeProfiling from the AMDTActivityLogger library.")
        ("envvar,e", po::value< std::vector<string> >()->multitoken(), "Environment variable that should be defined when running the profiled application. Argument should be in the format NAME=VALUE.")
        ("envvarfile,E", po::value<string>(), "Path to a file containing a list of environment variables that should be defined when running the profiled application. The file should contain one line for each variable in the format NAME=VALUE.")
        ("fullenv,f", "The environment variables specified with the envvar or envvarfile switch represent the full environment block.  If not specified, then the environment variables represent additions or changes to the system environment block.")
        ("list,l", "Print a list of valid counter names.")
        ("sessionname,N", po::value<string>(), "Name of the generated session.")
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
        ("apifilterfile,F", po::value<string>(), "Path to the API filter file which contains a list of OpenCL APIs to be filtered out when performing an API trace.")
#ifdef _WIN32
        ("interval,i", po::value<unsigned int>()->default_value(DEFAULT_TIMEOUT_INTERVAL), "Timeout interval. Ignored when not using timeout mode.")
#else
        ("interval,i", po::value<unsigned int>()->default_value(DEFAULT_TIMEOUT_INTERVAL), "Timeout interval.")
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
        ("counterfile,c", po::value<string>(), "Path to the counter file to enable selected counters (case-sensitive). If not provided, all counters will be used.")
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
        ("__nodetours__", "Don't launch application using detours (will prevent DirectCompute from being profiled).");

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

        // get the counter file
        boost::filesystem::wpath counterFile;

        if (unicodeOptionsMap.count("counterfile") > 0)
        {
            counterFile = boost::filesystem::wpath(unicodeOptionsMap["counterfile"]);
            configOut.strCounterFile = counterFile.string();
        }
        else
        {
            configOut.strCounterFile.clear();
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

        if (unicodeOptionsMap.count("list") > 0)
        {
            PrintCounters();
            return false;
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

// print a list of public counters
void PrintCounters()
{
    gtString strDirPath = FileUtils::GetExePathAsUnicode();
    string strErrorOut;
    GPAUtils gpaUtils;
    CounterList counterList;

    bool gpaInit = gpaUtils.InitGPA(GPA_API_OPENCL,
                                    strDirPath,
                                    strErrorOut,
                                    NULL,
                                    &counterList);

    if (gpaInit)
    {
        cout << "OpenCL performance counters:\n";

        for (int gen = GPA_HW_GENERATION_SOUTHERNISLAND; gen < GPA_HW_GENERATION_LAST; gen++)
        {
            GPA_HW_GENERATION hwGen = static_cast<GPA_HW_GENERATION>(gen);

            if (gpaUtils.GetAvailableCounters(hwGen, counterList))
            {
                string strGenerationName;

                if (GetGenerationName(hwGen, strGenerationName))
                {
                    PrintCounters(counterList, strGenerationName);
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
        cout << "HSA performance counters:\n";

        for (int gen = GPA_HW_GENERATION_SEAISLAND; gen < GPA_HW_GENERATION_LAST; gen++)
        {
            GPA_HW_GENERATION hwGen = static_cast<GPA_HW_GENERATION>(gen);

            if (gpaUtils.GetAvailableCounters(hwGen, counterList))
            {
                string strGenerationName;

                if (GetGenerationName(hwGen, strGenerationName))
                {
                    PrintCounters(counterList, strGenerationName);
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
        cout << "DirectCompute performance counters:\n";

        for (int gen = GPA_HW_GENERATION_SOUTHERNISLAND; gen < GPA_HW_GENERATION_LAST; gen++)
        {
            GPA_HW_GENERATION hwGen = static_cast<GPA_HW_GENERATION>(gen);

            if (gpaUtils.GetAvailableCounters(hwGen, counterList))
            {
                gpaUtils.FilterNonComputeCounters(hwGen, counterList);
                string strGenerationName;

                if (GetGenerationName(hwGen, strGenerationName))
                {
                    PrintCounters(counterList, strGenerationName);
                }
            }
        }

        gpaUtils.Unload();
    }

#endif
}

// helper function
void PrintCounters(CounterList& counterList, const string& strGenerationName)
{
    const unsigned int nLineBreak = 5;
    unsigned int curItem = 0;

    cout << "The list of valid counters for " << strGenerationName << " based graphics cards:\n";

    for (CounterList::iterator it = counterList.begin(); it != counterList.end(); ++it)
    {
        cout << *it;

        if (*it != counterList.back())
        {
            cout << ", ";
        }

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

    cout << endl << endl;
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
