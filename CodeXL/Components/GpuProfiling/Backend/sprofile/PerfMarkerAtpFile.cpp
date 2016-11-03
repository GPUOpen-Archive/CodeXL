//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file PerfMarkerAtpFile.cpp
/// \brief  PerfMarker Atp File writer and parser
//==============================================================================

#include <sstream>
#include <iostream>

#include "../Common/StringUtils.h"

#include "PerfMarkerAtpFile.h"

using namespace GPULogger;

#define READ_STREAM(istream, output, errMsg) \
    istream >> output; \
    if (istream.fail()) \
    { \
        Log(logERROR, "%s\n", errMsg); \
        m_strWarningMsg = errMsg; \
        m_bWarning = true; \
        return false; \
    }

bool PerfMarkerAtpFilePart::WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID)
{
    bool ret = false;
    SpAssertRet(m_sections.size() >= 1) ret;

    SP_fileStream::pos_type pos = sout.tellp();

    if (m_config.bTimeOut || m_config.bMergeMode)
    {
        ret = FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, PERFMARKER_EXT, GetSectionHeader(m_sections[0]).c_str());
    }

    // if we didn't merge any temp files into the output stream, then check for a .amdtperfmarker file
    if (!ret || pos == sout.tellp())
    {
        std::string strPerfFileName = FileUtils::GetBaseFileName(m_config.strOutputFile) + PERFMARKER_EXT;

        // Optional file
        if (FileUtils::FileExist(strPerfFileName))
        {
            std::string fileContent;
            ret = FileUtils::ReadFile(strPerfFileName, fileContent, false);
            sout << fileContent.c_str();
            remove(strPerfFileName.c_str());
        }
    }

    return ret;
}

void PerfMarkerAtpFilePart::SaveToFile(const std::string& strTmpFilePath, const std::string& strPID)
{
    SpAssertRet(m_sections.size() >= 1);

    if (m_config.bTimeOut || m_config.bMergeMode)
    {
        std::string strPerfFileName = FileUtils::GetBaseFileName(m_config.strOutputFile) + PERFMARKER_EXT;
        FileUtils::MergeTmpTraceFiles(strPerfFileName, strTmpFilePath, strPID, PERFMARKER_EXT, GetSectionHeader(m_sections[0]).c_str());
    }

    // else, nothing to be done
}

bool PerfMarkerAtpFilePart::Parse(std::istream& in, std::string& outErrorMsg)
{
    bool bError = false;
    std::string strProgressMessage = "Parsing Performance Marker Data...";
    unsigned int numBeginEntries = 0;
    unsigned int numEndEntries = 0;
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
            // finished reading PerfMarker entry sections
            RewindToPreviousPos(in);
            return true;
        }

        if (line.find("ProfilerVersion=") != std::string::npos)
        {
            // skip version property if it exists, this allows use of this parser for sessions that have a separate .amdtperfmarker file
            continue;
        }

        //thread id
        osThreadId tid = 0;
        bool ret = StringUtils::Parse(line, tid);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread ID, Unexpected data in input file.\n");
            return false;
        }

        READLINE(line)
        unsigned int numEntries = 0;
        ret = StringUtils::Parse(line, numEntries);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread perf marker count, Unexpected data in input file.\n");
            return false;
        }

        // read all entries for this thread
        for (unsigned int i = 0; i < numEntries && !m_shouldStopParsing; i++)
        {
            READLINE(line)

            ReportProgress(strProgressMessage, i, numEntries);

            if (line.empty())
            {
                continue;
            }

            std::string strMarkerType;
            std::string strMarkerName;
            std::string strGroupName;
            ULONGLONG ts;

            std::istringstream ss(line);

            READ_STREAM(ss, strMarkerType, "Failed to parse marker type.");

            PerfMarkerEntry* pEntry = NULL;

            if (strMarkerType == "clBeginPerfMarker")
            {
                std::string m_strID;
                READ_STREAM(ss, strMarkerName, "Failed to parse marker name.");
                READ_STREAM(ss, ts, "Failed to parse marker timestamp.");
                READ_STREAM(ss, strGroupName, "Failed to parse marker group.");

                // check to see if the group name has spaces not replaced with &nbsp; (older versions of the CLPerfMarker library didn't replace " " with &nbsp; in the group name
                while (!ss.eof())
                {
                    std::string tempGroupPart;
                    READ_STREAM(ss, tempGroupPart, "Failed to parse marker group.");
                    strGroupName.append(std::string(" ")).append(tempGroupPart);
                }

                strMarkerName = StringUtils::Replace(strMarkerName, std::string(SPACE), std::string(" "));
                strGroupName = StringUtils::Replace(strGroupName, std::string(SPACE), std::string(" "));

                pEntry = new PerfMarkerBeginEntry(PerfMarkerEntry::PerfMarkerType_Begin, ts, tid, strMarkerName, strGroupName);
                numBeginEntries++;
            }
            else if (strMarkerType == "clEndPerfMarker")
            {
                READ_STREAM(ss, ts, "Failed to parse marker timestamp.");

                pEntry = new PerfMarkerEntry(PerfMarkerEntry::PerfMarkerType_End, ts, tid);
                numEndEntries++;
            }
            else if (strMarkerType == "clEndPerfMarkerEx")
            {
                READ_STREAM(ss, ts, "Failed to parse marker timestamp.");
                READ_STREAM(ss, strMarkerName, "Failed to parse marker name.");
                READ_STREAM(ss, strGroupName, "Failed to parse marker group.");

                pEntry = new PerfMarkerEndExEntry(PerfMarkerEntry::PerfMarkerType_EndEx, ts, tid, strMarkerName, strGroupName);
                numEndEntries++;
            }

            // notify listeners
            for (std::vector<IParserListener<PerfMarkerEntry>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; ++it)
            {
                if (*it)
                {
                    (*it)->OnParse(pEntry, m_shouldStopParsing);
                }
            }

            delete pEntry;
        }

    }
    while (!in.eof());

    if (numBeginEntries != numEndEntries)
    {
        m_strWarningMsg = "Unbalanced perf markers in the output file.";
        m_bWarning = true;
    }

    return true;
}

bool PerfMarkerAtpFilePart::ParseHeader(const std::string& strKey, const std::string& strVal)
{
    SP_UNREFERENCED_PARAMETER(strKey);
    SP_UNREFERENCED_PARAMETER(strVal);
    return true;
}
