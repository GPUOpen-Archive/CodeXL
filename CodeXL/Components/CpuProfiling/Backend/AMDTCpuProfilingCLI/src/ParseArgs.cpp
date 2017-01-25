//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ParseArgs.cpp
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#include <cstdio>
#include <cwchar>
#include <cstdlib>

// Project:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <ParseArgs.h>
#include <CommonUtils.h>

//
//     Macros
//
#define    BAD_CHAR        (int)'?'
#define    BAD_ARGUMENT    (int)':'

int       optError = 1;    // if error message should be printed
int       optIndex = 1;    // index into parent argv vector
int       optOption;       // character checked for validity
int       optReset;        // reset getopt
wchar_t*  optArg;          // argument associated with option

static wchar_t errorMessage[] = { L"" };

// Parse argc/argv argument vector.
static int getOption(int argc, wchar_t* const argv[], const char* optstring)
{
    static wchar_t* place = errorMessage;    // option letter processing
    char* oli;                              // option letter list index

    if (optReset || *place == 0)
    {
        optReset = 0;
        place = argv[optIndex];

        if (optIndex >= argc || *place++ != '-')
        {
            // Argument is absent or is not an option
            place = errorMessage;
            return (-1);
        }

        optOption = *place++;

        if (optOption == '-' && *place == 0)
        {
            /* "--" => end of options */
            ++optIndex;
            place = errorMessage;
            return (-1);
        }

        if (optOption == 0)
        {
            // Solitary '-', treat as a '-' option
            // if the program (eg su) is looking for it.
            place = errorMessage;

            if (strchr(optstring, '-') == nullptr)
            {
                return -1;
            }

            optOption = '-';
        }
    }
    else
    {
        optOption = *place++;
    }

    // See if option letter is one the caller wanted...
    if (optOption == ':' || (oli = (char*)strchr(optstring, optOption)) == nullptr)
    {
        if (*place == 0)
        {
            ++optIndex;
        }

        if (optError && *optstring != ':')
        {
            (void)fprintf(stderr, "Unknown Option(-%c).\n", optOption);
        }

        return (BAD_CHAR);
        //return (optOption);
    }

    // Does this option need an argument ?
    if (oli[1] != ':')
    {
        // don't need argument
        optArg = nullptr;

        if (*place == 0)
        {
            ++optIndex;
        }
    }
    else
    {
        // Option-argument is either the rest of this argument or the
        //   entire next argument.
        if (*place)
        {
            optArg = place;
        }
        else if (argc > ++optIndex)
        {
            optArg = argv[optIndex];
        }
        else
        {
            /* option-argument absent */
            place = errorMessage;

            if (*optstring == ':')
            {
                return (BAD_ARGUMENT);
            }

            if (optError)
            {
                (void)fprintf(stderr, "Option(-%c) requires an argument.\n", optOption);
            }

            return (BAD_CHAR);
        }

        place = errorMessage;
        ++optIndex;
    }

    return (optOption);  // return option letter
}

ParseArgs::ParseArgs() { }


bool ParseArgs::Initialize(int nbrArgs, char* args[])
{
    bool retVal = false;

    // create an array of wchar pointers
    wchar_t** ppArgs = (wchar_t**)malloc(sizeof(wchar_t*) * nbrArgs);

    if (nullptr != ppArgs)
    {
        wchar_t tmpArg[OS_MAX_PATH];

        int i;

        for (i = 0; i < nbrArgs; i++)
        {
            memset(tmpArg, 0, sizeof(tmpArg));
            mbstowcs(tmpArg, args[i], OS_MAX_PATH - 1);
            ppArgs[i] = wcsdup(tmpArg);
        }

        retVal = InitializeArgs(nbrArgs, ppArgs);

        // free the memory allocated for the args
        for (i = 0; i < nbrArgs; i++)
        {
            if (nullptr != ppArgs[i])
            {
                free(ppArgs[i]);
                ppArgs[i] = nullptr;
            }
        }

        free(ppArgs);
    }

    return retVal;
}

bool ParseArgs::Initialize(int nbrArgs, wchar_t* args[])
{
    bool retVal = false;

    int i;
    gtString allArgs;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    (void) args;
    LPWSTR commandLineW = GetCommandLineW();
    allArgs = commandLineW;
#else

    for (i = 0; i < nbrArgs; i++)
    {
        allArgs += args[i];
        allArgs += L" ";
    }

#endif // Windows

    // create an array of wchar pointers
    wchar_t** ppArgs = (wchar_t**)malloc(sizeof(wchar_t*)* nbrArgs * 3);

    // Now process the all args
    int startPos = 0;
    int endPos = 0;
    int newArgIdx = 0;
    int startPos1 = 0;

    allArgs.trim();

    do
    {
        startPos = allArgs.findFirstNotOf(L" ", endPos);
        startPos1 = allArgs.find(L"\"", endPos);

        if ((-1 != startPos1) && (startPos1 == startPos))
        {
            startPos = startPos1 + 1;
            endPos = allArgs.find(L"\"", startPos);
        }
        else
        {
            endPos = allArgs.find(L" ", startPos);
        }

        endPos = (-1 != endPos) ? endPos - 1 : allArgs.length();

        gtString newArg;
        allArgs.getSubString(startPos, endPos, newArg);
        ppArgs[newArgIdx++] = wcsdup(newArg.asCharArray());

        endPos += 2;
    }
    while (endPos < allArgs.length());

    retVal = InitializeArgs(newArgIdx, ppArgs);

    // free the memory allocated for the args
    if (nullptr != ppArgs)
    {
        for (i = 0; i < newArgIdx; i++)
        {
            if (nullptr != ppArgs[i])
            {
                free(ppArgs[i]);
                ppArgs[i] = nullptr;
            }
        }

        free(ppArgs);
    }

    return retVal;
}

bool ParseArgs::InitializeArgs(int nbrArgs, wchar_t* args[])
{
    bool retVal = true;
    int opt;
    int tmpNbr;
    gtString tmp;
    //unsigned long long tmpULL;
    int nbrCores = 0;

    if (nbrArgs == 1)
    {
        // no option is specified by the user.. just print the help
        m_isPrintHelp = true;
        return true;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // To support windows style command line options
    for (int i = 1; i < nbrArgs; i++)
    {
        if (args[i][0] == '/')
        {
            args[i][0] = '-';
        }
    }

#endif // WINDOWS

    // Check if the first argument is the "COMMAND"
    if (nullptr == (wcsstr(args[1], L"-")))
    {
        m_command = gtString(args[1]);
        optIndex = 2;
    }
    else
    {
        optIndex = 1;
    }

    bool errorExit = false;
    int pos;
    int startPosition = 0;
    bool printCSSWarning = true;

    while ((opt = getOption(nbrArgs, args, "C:D:E:F:GIL:NOPR:S:T:V:X:abc:d:e:fg:hi:m:o:p:s:vw:")) != -1)
    {
        switch (opt)
        {
            case 'C':
                // Custom Profile
                m_isPredefinedProfile = false;
                m_customFile = gtString(optArg);
                break;

            case 'D':
                // REPORT option - debug symbol paths  (semicolon separated)
                m_debugSymbolPaths = gtString(optArg);
                break;

            // Not supported yet
            case 'E':
                // Raw Event support
                // multiple -E's are supported
                m_rawEventsStringVec.push_back(optArg);
                break;

            case 'F':
                // REPORT Option - Output file format CSV / TEXT
                m_outputFileFormat = gtString(optArg);
                break;

            case 'G':
                // Call Stack Sampling with default unwind interval and unwind depth values
                if (m_cssWithDefaultValues && printCSSWarning)
                {
                    fprintf(stderr, "Option to enable Callstack sampling (-G) is specified multiple times.\n");
                    printCSSWarning = false;
                }
                else
                {
                    m_cssWithDefaultValues = true;
                }

                break;

            case 'I':
                // REPORT option - ignore system modules
                m_ignoreSystemModules = true;
                break;

            case 'L':
                // Enable logging debug statements
                tmp = gtString(optArg);

                if (tmp.isIntegerNumber())
                {
                    tmp.toIntNumber(m_debugLogLevel);

                    if (m_debugLogLevel == 9)
                    {
                        m_printSampleCount = true;
                        m_debugLogLevel = 0;
                    }
                    else
                    {
                        // check if -L is non negative
                        if (m_debugLogLevel < OS_DEBUG_LOG_ERROR || m_debugLogLevel > OS_DEBUG_LOG_EXTENSIVE)
                        {
                            fprintf(stderr, "Invalid debug log level(%d) specified with option(-L), setting this to default value(0).\n", m_debugLogLevel);
                            m_debugLogLevel = 0; // set to default
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid debug log level(%s) is specified with option(-L), setting this to default value(0).\n", tmp.asASCIICharArray());
                    m_debugLogLevel = 0; // set to default
                }

                break;

            case 'N':
                // REPORT option - Display by NUMA;
                m_reportByNuma = true;
                break;

            case 'O':
                // REPORT option - Display by Core;
                m_reportByCore = true;
                break;

            case 'P':
                // REPORT option - Show Percentage
                m_showPercentage = true;
                break;

            case 'R':
                // REPORT option - Show Section
                m_sectionsToReport = gtString(optArg);
                break;

            case 'S':
                // REPORT Option - Symbol Server dirs (semicolon separated)
                m_symbolServerDirs = gtString(optArg);
                break;

            case 'T':
                // Special case TBP
                tmp = gtString(optArg);

                if (tmp.isIntegerNumber())
                {
                    tmp.toIntNumber(m_tbpSamplingInterval);

                    // check if -T is non negative
                    if (m_tbpSamplingInterval <= 0)
                    {
                        fprintf(stderr, "Negative or zero sampling interval value(%d) is specified with option(-T).\n",
                                m_tbpSamplingInterval);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid sampling interval is specified for Time-based profile with option(-T).\n");
                    retVal = false;
                }

                break;

            case 'V':
                // REPORT Option - View Config XML. By default "all"
                m_viewConfigName = gtString(optArg);
                break;

            case 'X':
                // REPORT option - Path to store downloaded symbols
                m_cachePath = gtString(optArg);
                break;

            case 'a':
                // System Wide Profile
                m_isSWP = true;
                break;

            case 'b':
                // Terminate/Break the launched application after the profile run is complete
                m_terminateLaunchApp = true;
                break;

            case 'c':
                tmp = gtString(optArg);
                startPosition = 0;
                osGetAmountOfLocalMachineCPUs(nbrCores);

                do
                {
                    pos = tmp.find(L",", startPosition);

                    gtString tmpStr;
                    int endPosition = (-1 != pos) ? (pos - 1) : tmp.length();
                    tmp.getSubString(startPosition, endPosition, tmpStr);

                    if (tmpStr.isIntegerNumber())
                    {
                        tmpStr.toIntNumber(tmpNbr);

                        if ((tmpNbr < 0) || (tmpNbr >= nbrCores))
                        {
                            fprintf(stderr, "Invalid Core Id(%d) is specified with option(-c). Valid core-ids are 0 to %d.\n",
                                tmpNbr, (nbrCores-1));
                            retVal = false;
                            break;
                        }

                        m_coreMaskInfo.AddCoreId(tmpNbr);
                    }
                    else
                    {
                        fprintf(stderr, "Invalid argument(%s) is passed with option(-c).\n", tmpStr.asASCIICharArray());
                        retVal = false;
                        break;
                    }

                    startPosition = pos + 1;
                } while (-1 != pos);

                if (retVal && m_coreMaskInfo.GetCoresList().empty())
                {
                    fprintf(stderr, "Invalid Core Ids(%s) are passed with option(-c).\n", tmp.asASCIICharArray());
                }

                break;

#if 0
            case 'c':
                // Core Affinity Mask
                // in SWP, only these cores will be profiled
                // in Per Process mode, the processor affinity will be set for the launch app
                // in attach mode, only these cores will be profiled
                tmp = gtString(optArg);

                if (tmp.toUnsignedLongLongNumber(tmpULL))
                {
                    m_coreAffinityMask = static_cast<gtUInt64>(tmpULL);

                    // check if -c is non negtive
                    if (static_cast<signed long long>(m_coreAffinityMask) <= 0)
                    {
                        fprintf(stderr, "Negative or zero core affinity mask value(0x%lx) is specified with option(-c).\n",
                                (unsigned long)m_coreAffinityMask);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid core affinity mask value(%s) is specified with option(-c).\n",
                            tmp.asASCIICharArray());
                    retVal = false;
                }

                break;
#endif //0

            case 'd':
                // Profile Duration
                tmp = gtString(optArg);

                if (tmp.isIntegerNumber())
                {
                    tmp.toIntNumber(m_profileDuration);

                    if (m_profileDuration <= 0)
                    {
                        fprintf(stderr, "Negative or zero profile duration value(%d) is specified with option(-d).\n",
                                m_profileDuration);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid profile duration value(%s) is specified with option(-d).\n",
                            tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'e':
                // REPORT Option - Sort Event Index
                tmp = gtString(optArg);

                if (tmp.isIntegerNumber())
                {
                    tmp.toIntNumber(m_sortEventIndex);

                    if (m_sortEventIndex < -1)
                    {
                        fprintf(stderr, "Negative Sort Event Index(%d) is specified with option(-e).\n",
                                m_sortEventIndex);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid Sort Event Index(%s) is specified with option(-e).\n",
                            tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'f':
                // Profile the children the launched application
                m_profileChildren = true;
                break;

            case 'g':
                // Call Stack Sampling
                tmp = gtString(optArg);
                pos = tmp.find(L':');

                if (-1 != pos)
                {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    int posScope = tmp.find(L':', pos + 1);
#else
                    int posScope = tmp.length() + 1;
#endif

                    if (-1 != posScope)
                    {
                        gtString tmpStr;
                        tmp.getSubString(0, pos - 1, tmpStr);

                        if (tmpStr.isIntegerNumber())
                        {
                            tmpStr.toIntNumber(m_unwindInterval);

                            if (m_unwindInterval <= 0)
                            {
                                fprintf(stderr, "Negative or zero unwind interval value(%d) is specified with option(-g).\n",
                                        m_unwindInterval);
                                retVal = false;
                            }
                            else if ((m_unwindInterval < CP_CSS_MIN_UNWIND_INTERVAL) ||
                                     (m_unwindInterval > CP_CSS_MAX_UNWIND_INTERVAL))
                            {
                                fprintf(stderr, "Unwind interval value(%d) should be within the range %d - %d.\n",
                                        m_unwindInterval, CP_CSS_MIN_UNWIND_INTERVAL, CP_CSS_MAX_UNWIND_INTERVAL);
                                retVal = false;
                            }
                        }
                        else
                        {
                            fprintf(stderr, "Invalid unwind interval value(%s) is specified with option(-g).\n",
                                    tmp.asASCIICharArray());
                            retVal = false;
                        }


                        tmp.getSubString(pos + 1, posScope - 1, tmpStr);

                        if (tmpStr.isIntegerNumber())
                        {
                            tmpStr.toIntNumber(m_unwindDepth);

                            if (m_unwindDepth <= 0)
                            {
                                fprintf(stderr, "Negative or zero unwind depth value(%d) is specified with option(-g).\n",
                                        m_unwindDepth);
                                retVal = false;
                            }
                            else if ((m_unwindDepth < CP_CSS_MIN_UNWIND_DEPTH) ||
                                     (m_unwindDepth > CP_CSS_MAX_UNWIND_DEPTH))
                            {
                                fprintf(stderr, "Unwind depth value(%d) should be within the range %d - %d.\n",
                                        m_unwindDepth, CP_CSS_MIN_UNWIND_DEPTH, CP_CSS_MAX_UNWIND_DEPTH);
                                retVal = false;
                            }
                        }
                        else
                        {
                            fprintf(stderr, "Invalid unwind depth value(%s) is passed with option(-g).\n",
                                    tmp.asASCIICharArray());
                            retVal = false;
                        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                        int withFpoSupport = tmp.find(L':', posScope + 1);
                        int posScopeLen = (-1 != withFpoSupport) ? (withFpoSupport - 1) : tmp.length();
                        tmpStr.makeEmpty();
                        tmp.getSubString(posScope + 1, posScopeLen, tmpStr);

                        if (0 == tmpStr.compareNoCase(L"user"))
                        {
                            m_cssScope = CP_CSS_SCOPE_USER;
                        }
                        else if (0 == tmpStr.compareNoCase(L"kernel"))
                        {
                            m_cssScope = CP_CSS_SCOPE_KERNEL;
                        }
                        else if (0 == tmpStr.compareNoCase(L"all"))
                        {
                            m_cssScope = CP_CSS_SCOPE_ALL;
                        }
                        else
                        {
                            fprintf(stderr, "Invalid scope value(%s) is passed with option(-g).\n", tmpStr.asASCIICharArray());
                            retVal = false;
                        }

                        if (-1 != withFpoSupport)
                        {
                            tmpStr.makeEmpty();
                            tmp.getSubString(withFpoSupport + 1, tmp.length(), tmpStr);

                            if (0 == tmpStr.compareNoCase(L"fpo"))
                            {
                                m_cssSupportFpo = true;
                            }
                            else if (0 == tmpStr.compareNoCase(L"nofpo"))
                            {
                                m_cssSupportFpo = false;
                            }
                            else
                            {
                                fprintf(stderr, "Invalid argument (%s) is passed with option(-g).\n", tmpStr.asASCIICharArray());
                                retVal = false;
                            }
                        }

#endif

                        m_isCSSEnabled = true;
                    }
                    else
                    {
                        fprintf(stderr, "Invalid argument(%s) is passed with option(-g).\n", tmp.asASCIICharArray());
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid argument(%s) is passed with option(-g).\n", tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'h':
                m_isPrintHelp = true;
                break;

            case 'i':
                // REPORT Option - input file
                m_inputFile = gtString(optArg);
                break;

            case 'm':
                // Predefined Profile
                m_isPredefinedProfile = true;

                if (!m_profileConfig.isEmpty())
                {
                    fprintf(stderr, "Multiple instances of option(-m) is specified. Only the last instance will be used.\n");
                }

                SetProfileConfig(optArg);
                break;

            case 'o':
                // Output file
                m_outputFile = gtString(optArg);
                break;

            case 'p':
                // Attach case
                tmp = gtString(optArg);
                startPosition = 0;

                do
                {
                    pos = tmp.find(L",", startPosition);

                    gtString tmpStr;
                    int endPosition = (-1 != pos) ? (pos - 1) : tmp.length();
                    tmp.getSubString(startPosition, endPosition, tmpStr);

                    if (tmpStr.isIntegerNumber())
                    {
                        tmpStr.toIntNumber(tmpNbr);

                        if (tmpNbr <= 0)
                        {
                            fprintf(stderr, "Invalid Process Id(%d) is specified with option(-p).\n",
                                    tmpNbr);
                            retVal = false;
                        }

                        m_pidsList.push_back(tmpNbr);
                    }
                    else
                    {
                        fprintf(stderr, "Invalid argument(%s) is passed with option(-p).\n", tmpStr.asASCIICharArray());
                        retVal = false;
                        break;
                    }

                    startPosition = pos + 1;
                }
                while (-1 != pos);

                if (! m_pidsList.empty())
                {
                    m_isAttach = true;
                }
                else
                {
                    fprintf(stderr, "Invalid Process Ids(%s) are passed with option(-p).\n", tmp.asASCIICharArray());
                }

                break;

            case 's':
                // Start Delay
                tmp = gtString(optArg);

                if (tmp.isIntegerNumber())
                {
                    tmp.toIntNumber(m_startDelay);

                    if (m_startDelay < 0)
                    {
                        fprintf(stderr, "Negative start delay duration(%d) is specified with option(-s).\n",
                                m_startDelay);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid start delay duration(%s) is specified with option(-s).\n",
                            tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'v':
                m_isPrintVersion = true;
                break;

            case 'w':
                m_workingDir = gtString(optArg);
                break;

            case BAD_CHAR:
                // exit
                errorExit = true;
                break;

            default:
                fprintf(stderr, "Invalid Option(%c).\n", (char)opt);
                errorExit = true;
        }
    }

    if (errorExit)
    {
        exit(-1);
    }

    if (optIndex < nbrArgs)
    {
        int i = 0;

        while ((i + optIndex) < nbrArgs)
        {
            if (m_launchApp.isEmpty())
            {
                m_launchApp = args[i + optIndex];
            }
            else
            {
                m_launchAppArgs.append(args[i + optIndex]);
                m_launchAppArgs.append(L" ");
            }

            i++;
        }
    }

    return retVal;
}

void ParseArgs::GetCoreMask(gtUInt64*& pCoreMask, gtUInt32& coreMaskSize)
{
    pCoreMask = nullptr;
    coreMaskSize = 0;

    if (!IsProfileAllCores())
    {
        m_coreMaskInfo.GetCoreMask(pCoreMask, coreMaskSize);
    }
}

gtString ParseArgs::GetInputFile()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_inputFile.startsWith(L"%"))
    {
        int endPos = m_inputFile.find(L"%", 1);

        if (-1 != endPos)
        {
            gtString tmpStr;
            m_inputFile.getSubString(0, endPos, tmpStr);

            gtString expandedPath;
            osExpandEnvironmentStrings(tmpStr, expandedPath);

            osFilePath inputPath(expandedPath);
            inputPath.reinterpretAsDirectory();

            expandedPath = inputPath.asString(true);

            tmpStr.makeEmpty();
            m_inputFile.getSubString(endPos + 1, m_inputFile.length(), tmpStr);
            expandedPath += tmpStr;

            m_inputFile = expandedPath;
        }
    }

#endif // WINDOWS

    return m_inputFile;
} // GetInputFile

gtString ParseArgs::GetOutputFile()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_outputFile.startsWith(L"%"))
    {
        int endPos = m_outputFile.find(L"%", 1);

        if (-1 != endPos)
        {
            gtString tmpStr;
            m_outputFile.getSubString(0, endPos, tmpStr);

            gtString expandedPath;
            osExpandEnvironmentStrings(tmpStr, expandedPath);

            osFilePath outputPath(expandedPath);
            outputPath.reinterpretAsDirectory();

            expandedPath = outputPath.asString(true);

            tmpStr.makeEmpty();
            m_outputFile.getSubString(endPos + 1, m_outputFile.length(), tmpStr);
            expandedPath += tmpStr;

            m_outputFile = expandedPath;
        }
    }

#endif

    return m_outputFile;
} // GetOutputFile

bool ParseArgs::SetProfileConfig(gtString config)
{
    bool retVal = false;

    if (! config.isEmpty())
    {
        m_profileConfig = config;
        retVal = true;
    }

    return retVal;
}
