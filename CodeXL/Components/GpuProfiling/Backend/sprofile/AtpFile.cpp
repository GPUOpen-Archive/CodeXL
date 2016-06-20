//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Atp file creation and parsing
//==============================================================================

#include <sstream>
#include <iostream>
#include "AtpFile.h"
#include "Version.h"
#include "../Common/OSUtils.h"
#include "../Common/StringUtils.h"
#include "../Common/FileUtils.h"

#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>

using namespace std;
using namespace GPULogger;


void IAtpFilePartParser::AddProgressMonitor(IParserProgressMonitor* pProgressMonitor)
{
    if (pProgressMonitor != NULL)
    {
        m_progressMonitorList.push_back(pProgressMonitor);
    }
}

void IAtpFilePartParser::ReportProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    for (std::vector<IParserProgressMonitor*>::const_iterator it = m_progressMonitorList.begin(); it != m_progressMonitorList.end(); it++)
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
    ss << "=====" << strSectionName << "=====";
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
        strTmpFilePath = FileUtils::GetTempFragFilePathAsUnicode();
    }

    if (!m_config.bCompatibilityMode)
    {
        stringstream ss;

        std::string strExtension = FileUtils::GetFileExtension(m_config.strOutputFile);

        if (strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .csv or .occupancy and append .atp
            string strBaseFileName = FileUtils::GetBaseFileName(m_config.strOutputFile);
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
        fout << ATP_FILE_HEADER_TraceFileVersion << "=" << m_AtpFileVersionMajor << "." << m_AtpFileVersionMinor << endl;
        fout << ATP_FILE_HEADER_ProfilerVersion << "=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER << endl;
        fout << ATP_FILE_HEADER_Application << "=" << m_config.strInjectedApp.asUTF8CharArray() << endl;
        fout << ATP_FILE_HEADER_ApplicationArgs << "=" << m_config.strInjectedAppArgs.asUTF8CharArray()  << endl;
        fout << ATP_FILE_HEADER_WorkingDirectory << "=" << m_config.strWorkingDirectory.asUTF8CharArray() << endl;

        if (!m_config.mapEnvVars.empty())
        {
            fout << ATP_FILE_HEADER_FullEnvironment << "=" << m_config.bFullEnvBlock << endl;

            for (EnvVarMap::const_iterator it = m_config.mapEnvVars.begin(); it != m_config.mapEnvVars.end(); ++it)
            {
                fout << ATP_FILE_HEADER_EnvVar << "=" << (it->first).asUTF8CharArray() << "=" << (it->second).asUTF8CharArray() << endl;
            }
        }

        fout << ATP_FILE_HEADER_UserTimer << "=" << (m_config.bUserTimer ? "True" : "False") << endl;

        fout << ATP_FILE_HEADER_OSVersion << "=" << OSUtils::Instance()->GetOSInfo().c_str() << endl;

        fout << ATP_FILE_HEADER_DisplayName << "=" << m_config.strSessionName.c_str() << endl;

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
        SpAssertRet(input.size() > 10) false;

        // remove leading =====
        sectionName = input.substr(5);

        // remove trailing =====
        sectionName = sectionName.substr(0, sectionName.length() - 5);
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
        SpAssert("Unsupported file format");
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
                    SpBreak("Failed to cast to IAtpFilePartParser");
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
                                    m_strWarningMsg = StringUtils::FormatString("AtpFileParser: Failed to parse section %s, file: %s @ line %d", strSecName.c_str(), m_strFileName.c_str(), m_nLine);
                                }

                            }
                        }
                    }
                    else
                    {
                        SpBreak("Failed to cast to IAtpFilePartParser");
                    }
                }
            }
        }

        else
        {
            m_bWarning = true;
            m_strWarningMsg = StringUtils::FormatString("AtpFileParser: Failed to parse section name, file: %s @ line %d", m_strFileName.c_str(), m_nLine);
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
                m_strWarningMsg = StringUtils::FormatString("AtpFileParser: Failed to read input file %s @ line %d", m_strFileName.c_str(), m_nLine);
                break;
            }

            if (line.length() == 0)
            {
                continue;
            }

            StringUtils::TrimInPlace(line);

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

