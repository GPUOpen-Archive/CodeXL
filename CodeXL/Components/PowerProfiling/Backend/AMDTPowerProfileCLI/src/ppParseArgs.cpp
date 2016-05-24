//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppParseArgs.cpp
///
//==================================================================================

// System
#include <stdio.h>
#include <stdlib.h>

// Infra
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Project
#include <ppParseArgs.h>
#include <PowerProfileCLI.h>

//
//     Macros
//

#define    PP_BAD_CHAR        (int)'?'
#define    PP_BAD_ARGUMENT    (int)':'
#define    PP_ERR_MESSAGE      ""
static char pErrMessage[2] = { "" };

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define HEX_FORMAT "0x%llx"
#else
    #define HEX_FORMAT "0x%lx"
#endif // Windows

int       optError = 1;    // if error message should be printed
int       optIndex = 1;    // index into parent argv vector
int       optOption;       // character checked for validity
int       optReset;        // reset getopt
char*     optArg;          // argument associated with option


// ppGetOpt
// Parse the argc/argv argument vector.
static int ppGetOpt(int argc, char* const argv[], const char* optstring)
{
    static char* place = pErrMessage;        /* option letter processing */
    char* oli;                /* option letter list index */

    if (optReset || *place == 0)          /* update scanning pointer */
    {
        optReset = 0;
        place = argv[optIndex];

        if (optIndex >= argc || *place++ != '-')
        {
            /* Argument is absent or is not an option */
            place = pErrMessage;
            return (-1);
        }

        optOption = *place++;

        if (optOption == '-' && *place == 0)
        {
            /* "--" => end of options */
            ++optIndex;
            place = pErrMessage;
            return (-1);
        }

        if (optOption == 0)
        {
            /* Solitary '-', treat as a '-' option
            if the program (eg su) is looking for it. */
            place = pErrMessage;

            if (strchr(optstring, '-') == NULL)
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

    /* See if option letter is one the caller wanted... */
    if (optOption == ':' || (oli = (char*)strchr(optstring, optOption)) == NULL)
    {
        if (*place == 0)
        {
            ++optIndex;
        }

        if (optError && *optstring != ':')
        {
            (void)fprintf(stderr, "unknown option -- %c\n", optOption);
        }

        return (PP_BAD_CHAR);
    }

    /* Does this option need an argument? */
    if (oli[1] != ':')
    {
        /* don't need argument */
        optArg = NULL;

        if (*place == 0)
        {
            ++optIndex;
        }
    }
    else
    {
        /* Option-argument is either the rest of this argument or the
        entire next argument. */
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
            place = pErrMessage;

            if (*optstring == ':')
            {
                return (PP_BAD_ARGUMENT);
            }

            if (optError)
            {
                (void)fprintf(stderr, "option requires an argument -- %c\n", optOption);
            }

            return (PP_BAD_CHAR);
        }

        place = pErrMessage;
        ++optIndex;
    }

    return (optOption); /* return option letter */
} // ppGetOpt


// ppParseArgs
//
ppParseArgs::ppParseArgs() : m_profileMode(AMDT_PWR_PROFILE_MODE_ONLINE),
    m_profileType(0),
    m_coreMask(static_cast<AMDTUInt64>(-1)),
    m_coreAffinityMask(static_cast<AMDTUInt64>(-1)),
    m_samplingInterval(PP_SAMPLING_INTERVAL_DEFAULT),
    m_profileDuration(0),
    m_isListCounters(false),
    m_hasProfileCounters(false),
    m_terminateLaunchApp(false),
    m_isPrintHelp(false),
    m_isPrintVersion(false),
    m_isGroupByDevice(false),
    m_simulateGui(false),
    m_reportFileFormat(PP_REPORT_EXTENSION_CSV),
    m_reportType(PP_REPORT_TYPE_CSV),
    m_exportToDb(false),
    m_isReportPathSet(false)
{
} // ppParseArgs


// Initialize
//
bool ppParseArgs::Initialize(int nbrArgs, char* args[])
{
    bool retVal = false;

    int i;
    gtString allArgs;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    (void) args;
    LPWSTR commandLineW = GetCommandLineW();
    allArgs = commandLineW;
#define HEX_FORMAT "0x%llx"
#else

    for (i = 0; i < nbrArgs; i++)
    {
        wchar_t tmpArg[OS_MAX_PATH];
        memset(tmpArg, 0, sizeof(tmpArg));
        mbstowcs(tmpArg, args[i], OS_MAX_PATH - 1);

        allArgs += tmpArg;
        allArgs += L" ";
    }

#endif // Windows

    // create an array of wchar pointers
    char** ppArgs = (char**)malloc(sizeof(char*) * nbrArgs * 2);

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
        ppArgs[newArgIdx++] = strdup(newArg.asASCIICharArray());

        endPos += 2;
    }
    while (endPos < allArgs.length());

    retVal = InitializeArgs(newArgIdx, ppArgs);

    // free the memory allocated for the args
    if (NULL != ppArgs)
    {
        for (i = 0; i < newArgIdx; i++)
        {
            if (NULL != ppArgs[i])
            {
                free(ppArgs[i]);
                ppArgs[i] = NULL;
            }
        }

        free(ppArgs);
    }

    return retVal;
} // Initialize


// InitializeArgs
//
bool ppParseArgs::InitializeArgs(int nbrArgs, char* args[])
{
    bool retVal = true;
    int opt;
    int tmpNbr;
    gtString tmp;
    unsigned long long tmpULL;

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

    bool errorExit = false;
    int pos;
    int startPosition = 0;
    wchar_t optArgW[OS_MAX_PATH];

    while ((opt = ppGetOpt(nbrArgs, args, "C:D:F:P:T:M:Xbc:d:e:ghlo:vw:z:")) != -1)
    {
        memset(optArgW, 0, sizeof(optArgW));

        if (NULL != optArg)
        {
            mbstowcs(optArgW, optArg, OS_MAX_PATH - 1);
        }

        switch (opt)
        {
            case 'C':
                // Core Affinity Mask
                tmp = gtString(optArgW);

                if (tmp.toUnsignedLongLongNumber(tmpULL))
                {
                    m_coreAffinityMask = static_cast<AMDTUInt64>(tmpULL);

                    // check if -c is non negtive
                    if (static_cast<signed long long>(m_coreAffinityMask) <= 0)
                    {
                        fprintf(stderr, "Negative or zero core affinity mask value" HEX_FORMAT "is specified with option(-c).\n",
                                m_coreAffinityMask);
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

            // Comma separated device id list
            case 'D':
                tmp = gtString(optArgW);

                do
                {
                    pos = tmp.find(L",", startPosition);
                    gtString tmpStr;
                    int endPosition = (-1 != pos) ? (pos - 1) : tmp.length();
                    tmp.getSubString(startPosition, endPosition, tmpStr);

                    if (tmpStr.isIntegerNumber())
                    {
                        tmpStr.toIntNumber(tmpNbr);

                        if ((static_cast<AMDTUInt32>(tmpNbr) != AMDT_PWR_ALL_DEVICES) && tmpNbr < 0)
                        {
                            fprintf(stderr, "Invalid Device Id(%d) is specified with option(-D).\n",
                                    tmpNbr);
                            retVal = false;
                        }
                        else
                        {
                            m_deviceIDList.push_back(tmpNbr);
                        }
                    }
                    //else if (tmpStr.isAlnum(L"- _"))
                    //{
                    //    m_deviceNameList.push_back(tmpStr);
                    //}
                    else
                    {
                        fprintf(stderr, "Invalid argument(%s) is passed with option(-D).\n", tmpStr.asASCIICharArray());
                        retVal = false;
                        break;
                    }

                    startPosition = pos + 1;
                }
                while (-1 != pos);

                if (!m_deviceIDList.empty())
                {
                    m_hasProfileCounters = true;
                }
                else
                {
                    //fprintf(stderr, "Invalid devices(%s) are passed with option(-D).\n", tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'F':
                // REPORT Option - Output file format CSV / TEXT
                m_reportFileFormat = gtString(optArgW);

                if (!m_reportFileFormat.compareNoCase(PP_REPORT_EXTENSION_CSV))
                {
                    m_reportType = PP_REPORT_TYPE_CSV;
                }
                else if (!m_reportFileFormat.compareNoCase(PP_REPORT_EXTENSION_TEXT))
                {
                    m_reportType = PP_REPORT_TYPE_TEXT;
                }
                else
                {
                    fprintf(stderr, "Unsupported output file format(%s) specified with option(-F).\n", m_reportFileFormat.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'P':
                // Pre defined comma seperated profile option string.
                // No support for number with this option
                tmp = gtString(optArgW);

                do
                {
                    pos = tmp.find(L",", startPosition);
                    gtString tmpStr;
                    int endPosition = (-1 != pos) ? (pos - 1) : tmp.length();
                    tmp.getSubString(startPosition, endPosition, tmpStr);

                    if (tmpStr.toUnsignedLongLongNumber(tmpULL))
                    {
                        fprintf(stderr, "Invalid argument(%llu) is passed with option(-P).\n", tmpULL);
                        retVal = false;
                        //break;
                    }
                    else if (tmpStr.isAlnum(L"- _"))
                    {
                        m_counterGroupNameList.push_back(tmpStr);
                    }
                    else
                    {
                        fprintf(stderr, "Invalid argument(%s) is passed with option(-P).\n",
                                tmpStr.asASCIICharArray());
                        retVal = false;
                    }

                    startPosition = pos + 1;
                }
                while (-1 != pos);

                if (!m_counterGroupNameList.empty())
                {
                    m_hasProfileCounters = true;
                }
                else
                {
                    fprintf(stderr, "Invalid argument(%s) is passed with option(-P).\n", tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'T':
                // Sampling Interval
                tmp = gtString(optArgW);

                if (tmp.isIntegerNumber())
                {
                    tmp.toIntNumber(m_samplingInterval);

                    // check if -T is non negative
                    if (m_samplingInterval <= 0)
                    {
                        fprintf(stderr, "Negative or zero sampling interval value(%d) is specified with option(-T).\n",
                                m_samplingInterval);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid sampling interval is specified with option(-T).\n");
                    retVal = false;
                }

                break;

            case 'M':
                tmp = gtString(optArgW);

                if (tmp.toUnsignedLongLongNumber(tmpULL))
                {
                    fprintf(stderr, "Invalid argument(%llu) is passed with option(-M).\n", tmpULL);
                    retVal = false;
                    //break;
                }
                else if ((tmp.isAlnum(L"- _")) && tmp.isEqual("counters"))
                {
                    // Process profile type
                    m_profileType = 0;
                }
                else if ((tmp.isAlnum(L"- _")) && tmp.isEqual("process"))
                {
                    // Process profile type
                    m_profileType = 1;
                    m_hasProfileCounters = true;
                }

#ifndef LINUX
                else if ((tmp.isAlnum(L"- _")) && tmp.isEqual("module"))
                {
                    // Process profile types
                    m_profileType = 2;
                    m_hasProfileCounters = true;
                }

#endif
                else
                {
                    fprintf(stderr, "Invalid argument(%s) is passed with option(-M).\n",
                            tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'X':
                m_simulateGui = true;
                break;

            case 'b':
                // Terminate/Break the launched application after the profile run is complete
                m_terminateLaunchApp = true;
                break;

            case 'c':
                // Core Mask
                tmp = gtString(optArgW);

                if (tmp.toUnsignedLongLongNumber(tmpULL))
                {
                    m_coreMask = static_cast<AMDTUInt64>(tmpULL);

                    // check if -c is non negtive
                    if (static_cast<signed long long>(m_coreMask) <= 0)
                    {
                        fprintf(stderr, "Negative or zero core mask value" HEX_FORMAT " is specified with option(-c).\n",
                                m_coreMask);
                        retVal = false;
                    }
                }
                else
                {
                    fprintf(stderr, "Invalid core mask value(%s) is specified with option(-c).\n",
                            tmp.asASCIICharArray());
                    retVal = false;
                }

                break;

            case 'd':
                // Profile Duration
                tmp = gtString(optArgW);

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

            // Comma separated Counter Names. Multiple -e's are supported
            case 'e':
                tmp = gtString(optArgW);

                do
                {
                    pos = tmp.find(L",", startPosition);
                    gtString tmpStr;
                    int endPosition = (-1 != pos) ? (pos - 1) : tmp.length();
                    tmp.getSubString(startPosition, endPosition, tmpStr);

                    if (tmpStr.isIntegerNumber())
                    {
                        tmpStr.toIntNumber(tmpNbr);

                        if (tmpNbr < 0)
                        {
                            fprintf(stderr, "Invalid Counter Id(%d) is specified with option(-e).\n",
                                    tmpNbr);
                            retVal = false;
                        }

                        m_counterIDList.push_back(tmpNbr);
                    }
                    else if (tmpStr.isAlnum(L"- _"))
                    {
                        m_counterNameList.push_back(tmpStr);
                    }

                    else
                    {
                        fprintf(stderr, "Invalid argument(%s) is passed with option(-e).\n", tmpStr.asASCIICharArray());
                        retVal = false;
                        break;
                    }

                    startPosition = pos + 1;
                }
                while (-1 != pos);

                if ((!m_counterIDList.empty()) || (!m_counterNameList.empty()))
                {
                    m_hasProfileCounters = true;
                }
                else
                {
                    fprintf(stderr, "Invalid counters(%s) are passed with option(-e).\n", tmp.asASCIICharArray());
                }

                break;

            case 'g':
                m_isGroupByDevice = true;
                fprintf(stderr, "Unsupported option -P.\n");
                retVal = false;
                break;

            case 'h':
                m_isPrintHelp = true;
                break;

            case 'l':
                m_isListCounters = true;
                break;

            case 'o':
                // Output file
                m_isReportPathSet = true;
                m_reportFile = gtString(optArgW);
                break;

            case 'z':
            {
                // DB file
                m_exportToDb  = true;
                m_dbFileOutDir = gtString(optArgW);
                osFilePath tmpFilePath(m_dbFileOutDir);
                osDirectory tmpDir(tmpFilePath);

                if (!tmpDir.exists())
                {
                    fprintf(stderr, "Directory not found: (%s).\n", m_dbFileOutDir.asASCIICharArray());
                    retVal = false;
                }
            }
            break;

            case 'v':
                m_isPrintVersion = true;
                break;

            case 'w':
                m_workingDir = gtString(optArgW);
                break;

            case PP_BAD_CHAR:
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
                memset(optArgW, 0, sizeof(optArgW));
                mbstowcs(optArgW, args[i + optIndex], OS_MAX_PATH - 1);
                m_launchApp = optArgW;
            }
            else
            {
                memset(optArgW, 0, sizeof(optArgW));
                mbstowcs(optArgW, args[i + optIndex], OS_MAX_PATH - 1);

                m_launchAppArgs.append(optArgW);
                m_launchAppArgs.append(L" ");
            }

            i++;
        }
    }

    return retVal;
} // InitializeArgs


// GetReportFilePath
//
bool ppParseArgs::GetReportFilePath(osFilePath& filePath)
{
    bool retVal = true;

    // If we have not already constructed the outputfile path, return it
    if (m_reportFilePath.isEmpty())
    {
        // If the user does not specify any outputfile path, use the default path.
        // The default base name is "%TEMP%/CodeXL-PowerProfile-<TimeStamp>"
        if (m_reportFile.isEmpty())
        {
            // TODO - linker error
            // osFilePath osTempPath(osFilePath::OS_TEMP_DIRECTORY);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            wchar_t  tmpPath[OS_MAX_PATH] = { L'\0' };
            GetTempPathW(OS_MAX_PATH, tmpPath);
            m_reportFile = gtString(tmpPath);
#else
            // TODO: USe P_tmpdir on Linux
            wchar_t  tmpPath[OS_MAX_PATH] = L"/tmp/";
            m_reportFile = gtString(tmpPath);
#endif // AMDT_BUILD_TARGET

            gtString profTime;
            osTime currentTime;
            currentTime.setFromCurrentTime();
            currentTime.dateAsString(profTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);

            m_reportFile.appendFormattedString(STR_FORMAT L"-" STR_FORMAT,
                                               PP_DEFAULT_OUTPUTFILE_NAME,
                                               profTime.asCharArray());
        }

        // Handle the ENVIRONMENT VARIABLES USED in the output file PATH
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (m_reportFile.startsWith(L"%"))
        {
            int endPos = m_reportFile.find(L"%", 1);

            if (-1 != endPos)
            {
                gtString tmpStr;
                m_reportFile.getSubString(0, endPos, tmpStr);

                gtString expandedPath;
                osExpandEnvironmentStrings(tmpStr, expandedPath);

                osFilePath reportPath(expandedPath);
                reportPath.reinterpretAsDirectory();

                expandedPath = reportPath.asString(true);

                tmpStr.makeEmpty();
                m_reportFile.getSubString(endPos + 1, m_reportFile.length(), tmpStr);
                expandedPath += tmpStr;

                m_reportFile = expandedPath;
            }
        }

#endif

        // Validate the output file path
        if (m_reportFilePath.isEmpty() && !m_reportFile.isEmpty())
        {
            // This cast forces the compiler to use the copy CTOR rather than
            // the move CTOR. This solves the linux build.
            m_reportFilePath = (const osFilePath&)osFilePath(m_reportFile);

            // check if the base dir exists
            osDirectory osDir;
            m_reportFilePath.getFileDirectory(osDir);

            if (!osDir.exists())
            {
                fprintf(stderr, "Output Directory (%s) does not exist\n", m_reportFilePath.fileDirectoryAsString().asASCIICharArray());

                m_reportFilePath.clear();
                retVal = false;
            }

            if (!osDir.isWriteAccessible())
            {
                ReportError(false, "Output Directory(%s) does not have write permission.\n", m_reportFilePath.fileDirectoryAsString().asASCIICharArray());
                return false;
            }

            gtString fileName;
            m_reportFilePath.getFileName(fileName);

            if (fileName.isEmpty())
            {
                ReportError(false, "Output file name(%s) is missing with option (-o).\n",
                            m_reportFilePath.asString(true).asCharArray());
                return false;
            }

            // If special characters like \ / : * ? " < > | are mentioned, report an error
            if ((fileName.find(L"\\") != -1) || (fileName.find(L"/") != -1) || (fileName.find(L":") != -1)
                || (fileName.find(L"*") != -1) || (fileName.find(L"?") != -1) || (fileName.find(L"\"") != -1)
                || (fileName.find(L"<") != -1) || (fileName.find(L">") != -1) || (fileName.find(L"|") != -1))
            {
                ReportError(false, "Output file name(%s) contains invalid characters.\n", fileName.asCharArray());
                return false;
            }
        }

        // Add the output file extension
        if (PP_REPORT_TYPE_CSV == m_reportType)
        {
            m_reportFilePath.setFileExtension(PP_REPORT_EXTENSION_CSV);
        }
        else if (PP_REPORT_TYPE_TEXT == m_reportType)
        {
            m_reportFilePath.setFileExtension(PP_REPORT_EXTENSION_TEXT);
        }
        else
        {
            fprintf(stderr, "Invalid report file format.\n");
            retVal = false;
        }
    }

    filePath = m_reportFilePath;
    return retVal;
} // GetOutputFile
