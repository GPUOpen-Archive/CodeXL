//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Atp file creation and parsing
//==============================================================================

#include <sstream>
#include <iostream>
#include <OSDefs.h>
#include <ProfilerOutputFileDefs.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include "AMDTOSWrappers/Include/osProcess.h"
#include "CXLAtpFile.h"
#include <Version.h>
#include <Defs.h>

using namespace std;

void IAtpFilePartParser::AddProgressMonitor(IParserProgressMonitor* pProgressMonitor)
{
    if (pProgressMonitor != NULL)
    {
        m_progressMonitorList.push_back(pProgressMonitor);
    }
}

void IAtpFilePartParser::ReportProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    for (std::vector<IParserProgressMonitor*>::const_iterator it = m_progressMonitorList.begin(); it != m_progressMonitorList.end(); ++it)
    {
        (*it)->OnParserProgress(strProgressMessage, uiCurItem, uiTotalItems);
    }
}

void IAtpFilePartParser::SetCurrentSection(const std::string& strSectionName)
{
    m_strCurrentSectionName = strSectionName;
}

std::string IAtpFilePart::GetSectionHeader(const std::string& strSectionName)
{
    stringstream ss;
    ss << ATP_SECTION_HEADER_START_END << strSectionName << ATP_SECTION_HEADER_START_END;
    return ss.str();
}

AtpFileWriter::AtpFileWriter(const Config& config, const std::string& strPID)
    : m_config(config)
{
    m_AtpFileVersionMajor = 3;
    m_AtpFileVersionMinor = 2;
    m_strPID = strPID;
}

AtpFileWriter::~AtpFileWriter()
{}

void AtpFileWriter::SaveToAtpFile()
{
    gtString strTmpFilePath;

    if (m_config.bMergeMode)
    {
        if (m_config.strWorkingDirectory.isEmpty())
        {
            char buf[SP_MAX_PATH];
            strTmpFilePath = strTmpFilePath.fromASCIIString(SP_getcwd(buf, SP_MAX_PATH));
        }
        else
        {
            strTmpFilePath = m_config.strWorkingDirectory;
        }
    }
    else
    {
#ifdef _WIN32
        osGetCurrentProcessEnvVariableValue(L"TEMP", strTmpFilePath);
        strTmpFilePath.append(L"\\");
#else //_LINUX || LINUX
        osGetCurrentProcessEnvVariableValue(L"HOME", strTmpFilePath);
        strTmpFilePath.append(L"/");
#endif

    }

    if (!m_config.bCompatibilityMode)
    {
        stringstream ss;
        gtString outputFilePathgtString;
        outputFilePathgtString = outputFilePathgtString.fromASCIIString(m_config.strOutputFile.c_str());
        osFilePath outputFilePath(outputFilePathgtString);
        gtString fileExtension;
        outputFilePath.getFileExtension(fileExtension);
        std::string strExtension = fileExtension.asASCIICharArray();

        if (strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .csv or .occupancy and append .atp
            gtString baseFileName;
            outputFilePath.getFileName(baseFileName);
            string strBaseFileName = baseFileName.asASCIICharArray();
            ss << strBaseFileName << "." << TRACE_EXT;
        }
        else
        {
            ss << m_config.strOutputFile;
        }

        string strOutputFile = ss.str();

        SP_fileStream fout(strOutputFile.c_str());

        if (fout.fail())
        {
            cout << "Failed to write output file: " << strOutputFile << endl;
            return;
        }

        // Write common headers
        // Write common headers
        fout << FILE_HEADER_TRACE_FILE_VERSION << EQUAL_SIGN_STR << m_AtpFileVersionMajor << "." << m_AtpFileVersionMinor << endl;
        fout << FILE_HEADER_PROFILER_VERSION << EQUAL_SIGN_STR << RCP_MAJOR_VERSION_STR << "." << RCP_MINOR_VERSION_STR << "." << RCP_BUILD_NUMBER_STR << endl;
        fout << FILE_HEADER_APPLICATION << EQUAL_SIGN_STR << m_config.strInjectedApp.asUTF8CharArray() << endl;
        fout << FILE_HEADER_APPLICATION_ARGS << EQUAL_SIGN_STR << m_config.strInjectedAppArgs.asUTF8CharArray() << endl;
        fout << FILE_HEADER_WORKING_DIRECTORY << EQUAL_SIGN_STR << m_config.strWorkingDirectory.asUTF8CharArray() << endl;

        if (!m_config.mapEnvVars.empty())
        {
            fout << FILE_HEADER_FULL_ENVIRONMENT << EQUAL_SIGN_STR << m_config.bFullEnvBlock << endl;

            for (EnvVarMap::const_iterator it = m_config.mapEnvVars.begin(); it != m_config.mapEnvVars.end(); ++it)
            {
                fout << FILE_HEADER_ENV_VAR << EQUAL_SIGN_STR << (it->first).asUTF8CharArray() << "=" << (it->second).asUTF8CharArray() << endl;
            }
        }

        fout << FILE_HEADER_USER_TIMER << EQUAL_SIGN_STR << (m_config.bUserTimer ? "True" : "False") << endl;

        auto OSVersion = []()->std::string
        {
            std::string retVal;

            int majorVersion = 0;
            int minorVersion = 0;
            int buildNumber = 0;
            gtString osVersionName;

            bool success = osGetOperatingSystemVersionString(osVersionName);

            if (success)
            {
                std::stringstream osInfo(std::stringstream::in | std::stringstream::out);
                osInfo.clear();

                osInfo << osVersionName.asUTF8CharArray();

                success = osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber);

                if (success)
                {
                    osInfo << " " << "Build " << majorVersion << "." << minorVersion << "." << buildNumber;
                }

                retVal = osInfo.str();
            }
            else
            {
                retVal.clear();
            }

            return retVal;
        };

        fout << FILE_HEADER_OS_VERSION << EQUAL_SIGN_STR << OSVersion().c_str() << endl;

        fout << FILE_HEADER_DISPLAY_NAME << EQUAL_SIGN_STR << m_config.strSessionName.c_str() << endl;

        // Write module specific headers if any
        for (std::vector<IAtpFilePart*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        {
            (*it)->WriteHeaderSection(fout);
        }

        bool contentsWritten = false;

        // Write contents
        for (std::vector<IAtpFilePart*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        {
            contentsWritten |= (*it)->WriteContentSection(fout, strTmpFilePath.asASCIICharArray(), m_strPID);
        }

        fout.close();

        // if no contents were written, then delete the file that was created containing only the file header -- BUG389307 and BUG403113
        if (!contentsWritten)
        {
            remove(strOutputFile.c_str());
        }
    }
    else
    {
        for (std::vector<IAtpFilePart*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        {
            (*it)->SaveToFile(strTmpFilePath.asASCIICharArray(), m_strPID);
        }
    }
}

bool AtpFileParser::ParseSectionName(const std::string& input, std::string& sectionName)
{
    bool retVal = true;

    if (m_atpFileVersion == 0)
    {
        // ATP Version 0: Section name is formated like: =====$Name=====
        if (input.size() > 10)
        {
            return false;
        }

        // remove leading =====
        sectionName = input.substr(5);

        // remove trailing =====
        sectionName = sectionName.substr(0, sectionName.length() - 6);
    }
    else if (m_atpFileVersion == 1)
    {
        // ATP Version 1: Section name is formated like: "//API=API_NAME"
        // remove leading //
        sectionName = input.substr(2);
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

bool AtpFileParser::ParseSuspectedHeaderLine(const string& headerLine)
{
    bool retVal = false;

    // First find out if this is still a header line (according to the file version)

    // For file version 0, Header is done when: the line that start with '====='
    if (m_atpFileVersion == 0)
    {
        // ATR: first line of format //==API Trace== or //==GPU Trace==
        retVal = (headerLine[0] == '=');
    }
    else if (m_atpFileVersion == 1)
    {
        retVal = (headerLine.find("//==API Trace==") != string::npos) || (headerLine.find("//==GPU Trace==") != string::npos);
    }
    else
    {
        // revisit here
        assert("Unsupported file format");
    }

    if (!retVal)
    {
        // Still in header, read the header content
        size_t idx = headerLine.find_first_of("=");
        string val = headerLine.substr(idx + 1);
        string key = headerLine.substr(0, idx);

        for (std::vector<IAtpFilePart*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        {
            if ((*it)->IsParser())
            {
                // pass header item to AtpFileParts to process
                IAtpFilePartParser* pParser = dynamic_cast<IAtpFilePartParser*>(*it);

                if (pParser != NULL)
                {
                    if (pParser->ShouldStopParsing())
                    {
                        m_shouldStopParsing = true;
                        break;
                    }

                    pParser->ParseHeader(key, val);
                }
                else
                {
                    //Revisit - Here
                    assert(0);
                    //SpBreak("Failed to cast to IAtpFilePartParser");
                }
            }
        }

        if (m_atpFileVersion == 1)
        {
            // Remove the '//' in the first of the key
            key.erase(0, 2);
        }

        m_headerMap[key] = val;
    }

    return retVal;
}

bool AtpFileParser::ParseFileSectionsLine(const std::string& sectionLine)
{
    bool retVal = false;

    bool isStartingSection = false;

    if (m_atpFileVersion == 0)
    {
        // When getting a version 0 atp file, we expect a section with the following format:
        // ===== CodeXL ocl API Trace Output =====
        isStartingSection = (sectionLine[0] == '=');
    }
    else if (m_atpFileVersion == 1)
    {
        // When getting a version 1 atp file, we expect a section with the following format:
        //==API Trace== or //==GPU Trace==
        //API=DX12
        //ThreadID=7532
        //ThreadAPICount=156327
        if (sectionLine.length() > 6)
        {
            isStartingSection = (strncmp(sectionLine.c_str(), "//API=", 6) == 0);
        }
    }

    if (isStartingSection)
    {
        // Read content
        string strSecName;
        retVal = ParseSectionName(sectionLine, strSecName);

        if (retVal)
        {
            for (std::vector<IAtpFilePart*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
            {
                if ((*it)->IsParser())
                {
                    // pass header item to AtpFileParts to process
                    IAtpFilePartParser* pParser = dynamic_cast<IAtpFilePartParser*>(*it);

                    if (pParser->ShouldStopParsing())
                    {
                        m_shouldStopParsing = true;
                        break;
                    }

                    if (pParser != NULL)
                    {
                        if ((*it)->HasSection(strSecName))
                        {
                            // Pass section content parsing to IAtpFilePartParser
                            // IAtpFilePartParser sets stream pointer to the beginning of the next section or eof
                            pParser->SetCurrentSection(strSecName);
                            retVal = pParser->Parse(fin, m_strWarningMsg);

                            if (!retVal)
                            {
                                m_shouldStopParsing = true;
                                m_bWarning = true;

                                if (m_strWarningMsg.empty())
                                {
                                    std::stringstream ss;
                                    ss << "AtpFileParser: Failed to parse section " << strSecName.c_str() << ", file: " << m_strFileName.c_str() << " @ line " << m_nLine;
                                    m_strWarningMsg = ss.str();
                                }

                            }
                        }
                    }
                    else
                    {
                        //Revisit to add log function
                        assert(0);
                        //SpBreak("Failed to cast to IAtpFilePartParser");
                    }
                }
            }
        }

        else
        {
            m_bWarning = true;
            std::stringstream ss;
            ss << "AtpFileParser: Failed to parse section name, file: " << m_strFileName.c_str() << " @ line " << m_nLine;
            m_strWarningMsg = ss.str();
        }
    }
    else
    {
        // Just ignore the content if this is not a section start
        retVal = true;
    }

    return retVal;
}

bool AtpFileParser::Parse()
{
    bool retVal = false;

    if (m_bFileOpen)
    {
        retVal = true;
        bool isHeaderDone = false;

        do
        {
            string line;

            if (m_shouldStopParsing)
            {
                break;
            }

            retVal = ReadLine(line);

            if (!retVal)
            {
                m_bWarning = true;
                std::stringstream ss;
                ss << "AtpFileParser: Failed to read input file "<< m_strFileName.c_str() << " @ line " << m_nLine;
                m_strWarningMsg = ss.str();
                break;
            }

            if (line.length() == 0)
            {
                continue;
            }

            gtString gtLinsString;
            gtLinsString = gtLinsString.fromASCIIString(line.c_str());
            gtLinsString = gtLinsString.trim();

            // While there are still header line, parse it
            if (!isHeaderDone)
            {
                isHeaderDone = ParseSuspectedHeaderLine(line);
            }

            // Once the header is done, parse the content
            if (isHeaderDone)
            {
                retVal = ParseFileSectionsLine(line);
            }

        }
        while (!fin.eof() && retVal);
    }

    return retVal;
}

