//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief StackTrace Atp File writer and parser
//==============================================================================


#include <sstream>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../Common/Defs.h"
#include "../Common/StringUtils.h"
#include "StackTraceAtpFile.h"

StackTraceAtpFilePart::StackTraceAtpFilePart(const std::string& strModName, const Config& config, bool shouldReleaseMemory) : IAtpFilePart(config, shouldReleaseMemory)
{
    m_strPartName = strModName;
    std::string sectionName = "CodeXL ";
    sectionName += strModName;
    sectionName += " Stack Trace Output";
    m_sections.push_back(sectionName);
}

StackTraceAtpFilePart::~StackTraceAtpFilePart()
{
    // clean up all stack entry objects
    for (SymbolEntryMap::iterator it = m_SymbolEntryMap.begin(); it != m_SymbolEntryMap.end(); it++)
    {
        std::vector<SymbolFileEntry*>& stackEntryList = it->second;

        for (std::vector<SymbolFileEntry*>::iterator listIt = stackEntryList.begin(); listIt != stackEntryList.end(); listIt++)
        {
            if ((*listIt) != NULL)
            {
                SAFE_DELETE((*listIt)->m_pStackEntry);
                delete *listIt;
            }
        }

        stackEntryList.clear();
    }

    m_SymbolEntryMap.clear();
}

void StackTraceAtpFilePart::WriteHeaderSection(SP_fileStream& sout)
{
    SP_UNREFERENCED_PARAMETER(sout);
}

bool StackTraceAtpFilePart::WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID)
{
    bool ret = false;
    SpAssertRet(m_sections.size() == 1) ret;

    if (m_config.bTimeOut || m_config.bMergeMode)
    {
        std::stringstream ssExtName;
        ssExtName << "." << m_strPartName << TMP_TRACE_STACK_EXT;

        if (!m_config.bTestMode)
        {
            ret = FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ssExtName.str().c_str(), GetSectionHeader(m_sections[0]).c_str());
        }
        else
        {
            ret |= FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ssExtName.str().c_str(), GetSectionHeader(m_sections[0]).c_str(), FileUtils::MergeSummaryType_None);
        }
    }
    else
    {
        std::stringstream ss;
        ss << FileUtils::GetBaseFileName(m_config.strOutputFile) << "." << m_strPartName << TRACE_STACK_EXT;
        std::string stackFile = ss.str();
        std::string fileContent;
        ret = FileUtils::ReadFile(stackFile, fileContent, false);
        sout << fileContent.c_str();
        remove(stackFile.c_str());
    }

    return ret;
}

void StackTraceAtpFilePart::SaveToFile(const std::string& strTmpFilePath, const std::string& strPID)
{
    SpAssertRet(m_sections.size() == 1);
    std::stringstream ss;
    ss << FileUtils::GetBaseFileName(m_config.strOutputFile) << TRACE_STACK_EXT;
    std::string stackFile = ss.str();

    if (m_config.bTimeOut || m_config.bMergeMode)
    {
        std::stringstream ssExtName;
        ssExtName << "." << m_strPartName << TMP_TRACE_STACK_EXT;

        ss.str("");
        ss << GetSectionHeader(m_sections[0]) << std::endl;

        if (m_config.bCompatibilityMode)
        {
            if (!m_config.bTestMode)
            {
                ss << "ProfilerVersion=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER;
            }
            else
            {
                ss << "ProfilerVersion=" << 0 << "." << 0 << "." << 0;
            }
        }

        if (!m_config.bTestMode)
        {
            FileUtils::MergeTmpTraceFiles(stackFile, strTmpFilePath, strPID, ssExtName.str().c_str(), ss.str().c_str());
        }
        else
        {
            FileUtils::MergeTmpTraceFiles(stackFile, strTmpFilePath, strPID, ssExtName.str().c_str(), ss.str().c_str(), FileUtils::MergeSummaryType_None);
        }
    }
    else
    {
        // a simple rename
        ss.str("");
        // Construct output name: $name.$mod.st
        ss << FileUtils::GetBaseFileName(m_config.strOutputFile) << "." << m_strPartName << TRACE_STACK_EXT;
        std::string fromFile = ss.str();
        OSUtils::Instance()->osMoveFile(fromFile.c_str(), stackFile.c_str());
    }
}

bool StackTraceAtpFilePart::Parse(std::istream& in, std::string& outErrorMsg)
{
    bool bError = false;
    std::string strProgressMessage = "Parsing Symbol Data...";
    ErrorMessageUpdater errorMessageUpdater(outErrorMsg, this);

    do
    {
        if (m_shouldStopParsing)
        {
            break;
        }

        std::string line;
        READLINE(line)

        if (line.length() == 0)
        {
            continue;
        }

        if (line[0] == '=')
        {
            // finished reading Stack entry sections
            RewindToPreviousPos(in);
            return true;
        }

        if (line.find("ProfilerVersion=") != std::string::npos)
        {
            // skip version property if it exists, this allows use of this parser for sessions that have a separate .st file
            continue;
        }

        // parse thread id
        osThreadId tid = 0;
        bool ret = StringUtils::Parse(line, tid);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread ID, Unexpected data in input file.\n");
            return false;
        }

        READLINE(line)
        unsigned int apiNum = 0;
        ret = StringUtils::Parse(line, apiNum);

        if (!ret)
        {
            Log(logERROR, "Failed to parse stack entry number, Unexpected data in input file.\n");
            return false;
        }

        for (std::vector<IParserListener<SymbolFileEntry>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); it++)
        {
            if ((*it) != NULL)
            {
                (*it)->SetAPINum(tid, apiNum);
            }
        }

        int apiIndex = -1;

        // read all stack entries for this thread
        for (unsigned int i = 0; i < apiNum && !m_shouldStopParsing; i++)
        {
            apiIndex++;
            READLINE(line)

            ReportProgress(strProgressMessage, i, apiNum);

            // stack entry
            std::string stackEntryStr = line;

            if (stackEntryStr.empty())
            {
                continue;
            }

            SymbolFileEntry* symFileEntry = NULL;

            if (!ParseSymbolEntry(stackEntryStr, symFileEntry))
            {
                Log(logERROR, "Failed to parse stack entry, val = %s.\n", stackEntryStr.c_str());
            }
            else
            {
                symFileEntry->m_tid = tid;
                m_SymbolEntryMap[tid].push_back(symFileEntry);

                for (std::vector<IParserListener<SymbolFileEntry>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
                {
                    if (((*it) != NULL) && (symFileEntry != NULL))
                    {
                        (*it)->OnParse(symFileEntry, m_shouldStopParsing);
                    }
                    else
                    {
                        SpBreak("symFileEntry == NULL");
                    }
                }
            }
        }

    }
    while (!in.eof());

    return true;
}

bool StackTraceAtpFilePart::ParseHeader(const std::string& strKey, const std::string& strVal)
{
    SP_UNREFERENCED_PARAMETER(strKey);
    SP_UNREFERENCED_PARAMETER(strVal);
    return true;
}

bool StackTraceAtpFilePart::ParseSymbolEntry(const std::string& buf, SymbolFileEntry*& pSymbolFileEntry)
{
    std::stringstream ss(buf);
    std::string strApiName;
    Address address = 0;
    LineNum displacement = 0;
    LineNum lineNum = 0;
    std::string strSymAddr;
    std::string strFileName;
    std::string strSymName;

    ss >> strApiName;
    CHECK_SS_ERROR(ss)

    std::string secondToken;
    ss >> secondToken;
    CHECK_SS_ERROR(ss)

    if (secondToken.find('+') != std::string::npos)
    {
        std::vector<std::string> tokens;
        StringUtils::Split(tokens, secondToken, std::string("+"), true, true);

        if (tokens.size() == 2)
        {
            strSymAddr = tokens[0];

            if (!StringUtils::Parse(strSymAddr, address))
            {
                return false;
            }

            //convert tokens[1] to displacement
            if (!StringUtils::Parse(strSymAddr, displacement))
            {
                return false;
            }
        }
    }
    else
    {
        // symbol entry may contain &nbsp; in place of spaces in symbol name (see APIBase::WriteStackEntry), replace those with spaces
        strSymName = StringUtils::Replace(secondToken, std::string(SPACE), " ");
        ss >> lineNum;
        CHECK_SS_ERROR(ss)

        // entry may or may not have a filename
        ss >> strFileName;

        if (!ss.fail())
        {
            // symbol entry may contain &nbsp; in place of spaces in filenames, replace those with spaces so that the file can be opened/read
            strFileName = StringUtils::Replace(strFileName, std::string(SPACE), " ");
        }
    }

    StackEntry* stackEntry = new StackEntry();
    stackEntry->m_dwAddress = address;
    stackEntry->m_dwDisplacement = displacement;
    stackEntry->m_dwLineNum = lineNum;
    stackEntry->m_strFile = strFileName;
    stackEntry->m_strSymAddr = strSymAddr;
    stackEntry->m_strSymName = strSymName;

    pSymbolFileEntry = new SymbolFileEntry(m_strPartName, strApiName, stackEntry);

    return true;
}
