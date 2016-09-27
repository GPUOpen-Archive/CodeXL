//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  HSA Atp File writer and parser
//==============================================================================

#include "HSAAtpFile.h"
#include "../HSAFdnCommon/HSAFunctionDefsUtils.h"
#include "../Common/OSUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "../Common/Version.h"
#include "../Common/Defs.h"
#include "../Common/ATPFileUtils.h"
#include <sstream>
#include <set>

using namespace std;
using namespace GPULogger;

#define HSA_PART_NAME "hsa"
static const std::string s_PART_NAME = HSA_PART_NAME;
static const std::string s_HSA_TRACE_OUTPUT = "CodeXL " HSA_PART_NAME " API Trace Output";
static const std::string s_HSA_TIMESTAMP_OUTPUT = "CodeXL " HSA_PART_NAME " Timestamp Output";
static const std::string s_HSA_KERNEL_TIMESTAMP_OUTPUT = "CodeXL " HSA_PART_NAME " Kernel Timestamp Output";
#undef HSA_PART_NAME

#define MAX_LINE_SIZE 2048

HSAAtpFilePart::HSAAtpFilePart(const Config& config, bool shouldReleaseMemory)
    : IAtpFilePart(config, shouldReleaseMemory), m_dispatchIndex(0)
{
    m_strPartName = s_PART_NAME;
    m_sections.push_back(s_HSA_TRACE_OUTPUT);
    m_sections.push_back(s_HSA_TIMESTAMP_OUTPUT);
    m_sections.push_back(s_HSA_KERNEL_TIMESTAMP_OUTPUT);
}

HSAAtpFilePart::~HSAAtpFilePart(void)
{
}

void HSAAtpFilePart::WriteHeaderSection(SP_fileStream& sout)
{
    std::set<std::string> excludedAPIs;
    ReadExcludedAPIs(m_config.strAPIFilterFile, excludedAPIs);
    WriteExcludedAPIs(sout, "HSA", excludedAPIs);
}

bool HSAAtpFilePart::WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID)
{
    bool ret = false;
    SpAssertRet(m_sections.size() == 3) ret;

    if (m_config.bTimeOut || m_config.bMergeMode)
    {
        UpdateTmpTimestampFiles(strTmpFilePath, strPID);

        stringstream ss;
        ss << "." << m_strPartName << TMP_TRACE_EXT;
        ret = FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ss.str().c_str(), GetSectionHeader(m_sections[0]).c_str());

        ss.str("");
        ss << "." << m_strPartName << TMP_TIME_STAMP_EXT;
        ret |= FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ss.str().c_str(), GetSectionHeader(m_sections[1]).c_str());

        ss.str("");
        ss << "." << m_strPartName << TMP_KERNEL_TIME_STAMP_EXT;
        ret |= FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ss.str().c_str(), GetSectionHeader(m_sections[2]).c_str(), FileUtils::MergeSummaryType_CumulativeNumEntries);
    }
    else
    {
        // read .$(ModName).atp and write to sout
        stringstream ss;

        std::string strExtension = FileUtils::GetFileExtension(m_config.strOutputFile);

        if (strExtension == TRACE_EXT || strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .atp, .csv or .occupancy and append .$ModName.atp
            string strBaseFileName = FileUtils::GetBaseFileName(m_config.strOutputFile);
            ss << strBaseFileName << "." << m_strPartName << "." << TRACE_EXT;
        }
        else
        {
            // append $ModName.atp
            ss << m_config.strOutputFile << "." << m_strPartName << "." << TRACE_EXT;
        }

        string fileContent;
        ret = FileUtils::ReadFile(ss.str(), fileContent, false);
        sout << fileContent.c_str();
        remove(ss.str().c_str());
    }

    return ret;
}

HSAAPIInfo* HSAAtpFilePart::CreateAPIInfo(const std::string& strAPIName)
{
    HSAAPIInfo* retObj = NULL;

    HSA_API_Type apiType = HSAFunctionDefsUtils::Instance()->ToHSAAPIType(strAPIName);


    if (apiType == HSA_API_Type_hsa_amd_memory_async_copy)
    {
        retObj = new(nothrow) HSAMemoryTransferAPIInfo();
    }
    else if (apiType == HSA_API_Type_hsa_memory_allocate || apiType == HSA_API_Type_hsa_memory_copy ||
        apiType == HSA_API_Type_hsa_memory_register || apiType == HSA_API_Type_hsa_memory_deregister ||
        apiType == HSA_API_Type_hsa_amd_memory_pool_allocate ||
        apiType == HSA_API_Type_hsa_amd_memory_lock || apiType == HSA_API_Type_hsa_amd_memory_fill ||
        apiType == HSA_API_Type_hsa_amd_interop_map_buffer)
    {
        retObj = new(nothrow)HSAMemoryAPIInfo();
    }
    else
    {
        retObj = new(nothrow)HSAAPIInfo();
    }

    if (retObj)
    {
        retObj->m_strName = strAPIName;
    }

    return retObj;
}

// TODO:Should this live in HSAStringUtils or HSAAtpFileParser?
bool ParseWorkgroup(istream& str, string& raw, size_t& dim, size_t& x, size_t& y, size_t& z)
{
    string tmp;
    str >> tmp;
    raw = tmp;

    if (tmp.size() < 3)
    {
        // smallest workgroup string: {1}
        return false;
    }

    // remove tailing and leading bracket
    tmp = tmp.substr(1);
    tmp = tmp.substr(0, tmp.size() - 1);

    // extract numbers;
    vector<string> nums;
    StringUtils::Split(nums, tmp, string(","));
    SpAssertRet(nums.size() > 0) false;

    dim = nums.size();

    switch (dim)
    {
        case 3:
            SpAssertRet(StringUtils::Parse(nums[2], z)) false;

        // fall through is intentional
        case 2:
            SpAssertRet(StringUtils::Parse(nums[1], y)) false;

        // fall through is intentional
        case 1:
            SpAssertRet(StringUtils::Parse(nums[0], x)) false;
            break;

        default:
            return false;
    }

    return true;
}

bool HSAAtpFilePart::ParseHostTimestamp(const char* buf, HSAAPIInfo* pAPIInfo, bool bTimeoutMode)
{
    SP_UNREFERENCED_PARAMETER(bTimeoutMode);

    stringstream ss(buf);
    int apiTypeID;
    string apiName;
    ULONGLONG ullStart;
    ULONGLONG ullEnd;

    ss >> apiTypeID;
    CHECK_SS_ERROR(ss)
    ss >> apiName;
    CHECK_SS_ERROR(ss)

    if (!pAPIInfo)
    {
        return false;
    }

    pAPIInfo->m_apiID = static_cast<HSA_API_Type>(apiTypeID);

    if (apiName != pAPIInfo->m_strName)
    {
        // unmatched num of API trace items and num of timestamp items
        // in this case, number of api could greater than number of timestamp item,
        // it's harder to recover from this problem.
        Log(logWARNING, "Unexpected data in input file. Inconsistent API trace item and timestamp item.\n");
        //m_strWarningMsg = "[Warning]Unexpected data in input file. Incomplete summary pages may be generated.";
        return false;
    }

    ss >> ullStart;
    CHECK_SS_ERROR(ss)
    ss >> ullEnd;
    CHECK_SS_ERROR(ss)

    pAPIInfo->m_ullStart = ullStart;
    pAPIInfo->m_ullEnd = ullEnd;

    if (HSA_API_Type_hsa_amd_memory_async_copy == pAPIInfo->m_apiID)
    {
        ULONGLONG ullAsyncCopyStart;
        ULONGLONG ullAsyncCopyEnd;

        ss >> ullAsyncCopyStart;
        CHECK_SS_ERROR(ss)
        ss >> ullAsyncCopyEnd;
        CHECK_SS_ERROR(ss)

        HSAMemoryTransferAPIInfo* pAsyncCopyApi = dynamic_cast<HSAMemoryTransferAPIInfo*>(pAPIInfo);

        pAsyncCopyApi->m_transferStartTime = ullAsyncCopyStart;
        pAsyncCopyApi->m_transferEndTime = ullAsyncCopyEnd;
    }

    return true;
}

bool HSAAtpFilePart::ParseDeviceTimestamp(const char* buf, HSADispatchInfo* pDispatchInfo)
{
    unsigned int curDispatchIndex = m_dispatchIndex;
    m_dispatchIndex++;

    SpAssertRet(NULL != pDispatchInfo) false;
    stringstream ss(buf);
    string strKernelName;
    ULONGLONG uiKernelHandle;
    string strDeviceName;
    string strDeviceHandle;
    unsigned int queueIndex = 0;
    string strQueueHandle;
    ULONGLONG ullStart;
    ULONGLONG ullEnd;

    ss >> strKernelName;
    CHECK_SS_ERROR(ss)
    ss >> std::hex >> uiKernelHandle >> std::dec;
    CHECK_SS_ERROR(ss)

    ss >> ullStart;
    CHECK_SS_ERROR(ss)
    ss >> ullEnd;
    CHECK_SS_ERROR(ss)

    ss >> strDeviceName;
    CHECK_SS_ERROR(ss)

    ss >> strDeviceHandle;
    CHECK_SS_ERROR(ss)

    // ATP files prior to 3.2 did not contain the queueIndex
    if (m_atpMajorVer > 3 || (m_atpMajorVer == 3 && m_atpMinorVer >= 2))
    {
        ss >> queueIndex;
        CHECK_SS_ERROR(ss)
    }

    ss >> strQueueHandle;
    CHECK_SS_ERROR(ss)

    pDispatchInfo->m_strKernelName = strKernelName;
    pDispatchInfo->m_uiKernelHandle = uiKernelHandle;
    pDispatchInfo->m_ullStart = ullStart;
    pDispatchInfo->m_ullEnd = ullEnd;
    pDispatchInfo->m_strDeviceName = strDeviceName;
    pDispatchInfo->m_strDeviceHandle = strDeviceHandle;
    pDispatchInfo->m_queueIndex = queueIndex;
    pDispatchInfo->m_strQueueHandle = strQueueHandle;
    pDispatchInfo->m_uiSeqID = curDispatchIndex;

    return true;
}

bool HSAAtpFilePart::Parse(std::istream& in, std::string& outErrorMsg)
{
    // Assumption: 1) HSA API trace section first then HSA timestamp + HSA kernel timestamp OR 2) just HSA kernel timestamp
    bool bError = false;
    bool bTSStart = false;
    bool bKTSStart = false;
    std::string strProgressMessage = "Parsing Trace Data...";
    ErrorMessageUpdater errorMessageUpdater(outErrorMsg, this);

    if (s_HSA_KERNEL_TIMESTAMP_OUTPUT == m_strCurrentSectionName)
    {
        // if the .atp file has only the kernel timestamp section, then we will start on the s_HSA_KERNEL_TIMESTAMP_OUTPUT section
        bTSStart = true;
        bKTSStart = true;
    }

    do
    {
        string line;

        if (m_shouldStopParsing)
        {
            // proceed to the next section and continue parsing there
            line.clear();

            while (line[0] != '=')
            {
                READLINE(line)
            }

            if (line.length() == 0)
            {
                continue;
            }

            if (line[0] == '=')
            {
                m_shouldStopParsing = false;
                RewindToPreviousPos(in);
            }
        }

        READLINE(line)

        if (line.length() == 0)
        {
            continue;
        }

        if (line[0] == '=')
        {
            if (!bTSStart)
            {
                bTSStart = true;
                strProgressMessage = "Parsing Timeline Data...";
                // read thread id
                ReadLine(in, line);
            }
            else
            {
                size_t sectionNameStart = line.find_first_not_of('=');
                size_t sectionNameEnd = line.find_last_not_of('=');
                string sectionName = line.substr(sectionNameStart, line.size() - sectionNameStart - sectionNameEnd);
                if (!bKTSStart && (0 == sectionName.compare(s_HSA_KERNEL_TIMESTAMP_OUTPUT)))
                {
                    bKTSStart = true;
                    strProgressMessage = "Parsing Kernel Timestamp Data...";
                    // read api count
                    ReadLine(in, line);
                }
                else
                {
                    // finished reading HSA trace sections
                    RewindToPreviousPos(in);
                    return true;
                }
            }
        }

        //else it's thread id
        osThreadId tid = 0;
        bool ret = false;

        if (!bKTSStart)
        {
            ret = StringUtils::Parse(line, tid);

            if (!ret)
            {
                Log(logERROR, "Failed to parse thread ID, Unexpected data in input file.\n");
                return false;
            }

            READLINE(line)
        }

        unsigned int apiNum = 0;

        if (!line.empty())
        {
            ret = StringUtils::Parse(line, apiNum);

            if (!ret)
            {
                Log(logERROR, "Failed to parse thread number, Unexpected data in input file.\n");
                return false;
            }
        }

        int apiIndex = -1;

        if (!bKTSStart)
        {
            for (std::vector<IParserListener<HSAAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); it++)
            {
                if ((*it) != NULL)
                {
                    (*it)->SetAPINum(tid, apiNum);
                }
            }
        }

        // read all apis for this thread
        for (unsigned int i = 0; i < apiNum && !m_shouldStopParsing; i++)
        {
            apiIndex++;
            READLINE(line)

            ReportProgress(strProgressMessage, i, apiNum);

            if (!bTSStart)
            {
                // api trace
                string apiTraceStr = line;

                if (apiTraceStr.empty())
                {
                    continue;
                }

                string name;

                // First find the API name (required for the API info object allocation)
                size_t equalSignPos = apiTraceStr.find_first_of("=");
                size_t openParenPos = apiTraceStr.find_first_of("(");

                if (openParenPos != string::npos)
                {
                    size_t nameStartIndex = 0;

                    // In HSA, not all APIs have a return value -- those with no return value do not have an equal sign before the API name
                    if (equalSignPos == string::npos || equalSignPos > openParenPos)
                    {
                        equalSignPos = 0;
                    }
                    else
                    {
                        // If there is an equal sign, increment the start index (we expect a space before the name starts)
                        nameStartIndex = equalSignPos + 2;
                    }

                    // Back up from the opening parentheses, this is where the name actually ends
                    size_t nameEndIndex = openParenPos - 1;

                    // Get the name
                    size_t nameLength = nameEndIndex - nameStartIndex;

                    if ((nameStartIndex < apiTraceStr.size()) && (nameStartIndex + nameLength <= apiTraceStr.size()))
                    {
                        name = apiTraceStr.substr(nameStartIndex, nameEndIndex - nameStartIndex);

                        // Create the HSA API info object
                        HSAAPIInfo* pAPIInfo = CreateAPIInfo(name);

                        // Parse the return value
                        if (0 != equalSignPos)
                        {
                            pAPIInfo->m_strRet = apiTraceStr.substr(0, equalSignPos - 1);
                        }
                        else
                        {
                            pAPIInfo->m_strRet.clear();
                        }

                        // Parse the arg list
                        size_t argListEndIndex = apiTraceStr.find_first_of(")");

                        if (argListEndIndex != string::npos)
                        {
                            size_t argListLength = argListEndIndex - openParenPos;

                            if (nameEndIndex + argListLength < apiTraceStr.size())
                            {
                                pAPIInfo->m_ArgList = StringUtils::Trim(apiTraceStr.substr(nameEndIndex + 2, argListLength - 2));
                                pAPIInfo->ParseArgList();
                            }
                        }
                        else
                        {
                            Log(logERROR, "Failed to parse API trace (%s), Unexpected data in input file.\n", line.c_str());
                            return false;
                        }

                        pAPIInfo->m_tid = tid;
                        pAPIInfo->m_uiSeqID = i;
                        pAPIInfo->m_bHasDisplayableSeqId = true;
                        pAPIInfo->m_uiDisplaySeqID = i;

                        m_HSAAPIInfoMap[tid].push_back(pAPIInfo);
                    }

                }
                else
                {
                    Log(logERROR, "Failed to parse API trace (%s), Unexpected data in input file.\n", line.c_str());
                    return false;
                }
            }
            else if (!bKTSStart)
            {
                // timestamp
                // find APIInfo object from InfoMap
                HSAAPIInfo* pAPIInfo = NULL;
                std::vector<HSAAPIInfo*>& apiList = m_HSAAPIInfoMap[ tid ];

                if (apiList.size() <= i)
                {
                    // unmatched num of API trace items and num of timestamp items
                    // skip this timestamp
                    // This could happen in timeout mode
                    Log(logWARNING, "Unexpected data in input file. Inconsistent number of API trace items and timestamp items. Number of timestamp item is greater than number of api trace item.\n");
                    m_bWarning = true;
                    m_strWarningMsg = "[Warning]Unexpected data in input file. Incomplete summary pages may be generated.";
                    continue;
                }
                else
                {
                    pAPIInfo = apiList[i];

                    if (!ParseHostTimestamp(line.c_str(), pAPIInfo))
                    {
                        Log(logERROR, "Unexpected data in input file. Failed to parse timestamp entry.\n");
                        return false;
                    }
                }

                for (std::vector<IParserListener<HSAAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
                {
                    if (((*it) != NULL) && (pAPIInfo != NULL))
                    {
                        (*it)->OnParse(pAPIInfo, m_shouldStopParsing);
                    }
                    else
                    {
                        SpBreak("pAPIInfo == NULL");
                    }
                }
            }
            else
            {
                HSADispatchInfo* dispatchInfo = new(std::nothrow) HSADispatchInfo();

                if (NULL == dispatchInfo)
                {
                    Log(logERROR, "Error: out of memory\n");
                    return false;
                }

                if (!ParseDeviceTimestamp(line.c_str(), dispatchInfo))
                {
                    Log(logERROR, "Error parsing device timestamp entry\n");
                    return false;
                }

                m_HSADispatchInfoList.push_back(dispatchInfo);

                for (std::vector<IParserListener<HSAAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
                {
                    if (((*it) != NULL))
                    {
                        (*it)->OnParse(dispatchInfo, m_shouldStopParsing);
                    }
                }
            }
        }
    }
    while (!in.eof());

    return true;
}

bool HSAAtpFilePart::ParseHeader(const std::string& strKey, const std::string& strVal)
{
    if (0 == strKey.compare(ATP_FILE_HEADER_TraceFileVersion))
    {
        StringUtils::ParseMajorMinorVersion(strVal, m_atpMajorVer, m_atpMinorVer);
    }

    return true;
}

bool HSAAtpFilePart::UpdateTmpTimestampFiles(const std::string& strTmpFilePath, const std::string& strFilePrefix)
{
    std::vector<std::string> files;

    if (!FileUtils::GetFilesUnderDir(strTmpFilePath, files, strFilePrefix))
    {
        return false;
    }
    else
    {
        std::string strAsyncCopyTSFile;
        std::stringstream ss;
        ss << '.' << m_strPartName;
        std::string strPartName = ss.str();
        ThreadCopyItemMap threadCopyTimestamps;

        // first locate the async copy timestamp file
        for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            size_t part_found;
            part_found = it->find(strPartName);

            if (part_found == string::npos)
            {
                Log(logMESSAGE, "Skipping file: %s. It does not contain: %s.\n", it->c_str(), strPartName.c_str());
                continue;
            }

            size_t found = it->find_last_of(".");

            if (found != string::npos)
            {
                std::string strExt = it->substr(found);

                if (strExt == TMP_ASYNC_COPY_TIME_STAMP_EXT)
                {
                    strAsyncCopyTSFile = strTmpFilePath + '/' + *it;

                    // once we find the async copy timestamp file, load the data into threadCopyTimestamps
                    if (!LoadAsyncCopyTimestamps(strAsyncCopyTSFile, threadCopyTimestamps))
                    {
                        return false;
                    }

                    break;
                }
            }
            else
            {
                // wrong file name -- ignore this one
                Log(logMESSAGE, "Incorrect file name : %s.\n", it->c_str());
                continue;
            }
        }

        bool retVal = true;

        // now search the temp trace files to match up the timestamps to the correct API (using the signal handle string)
        for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            size_t part_found = it->find(strPartName);

            if (part_found == string::npos)
            {
                Log(logMESSAGE, "Skipping file: %s. It does not contain: %s.\n", it->c_str(), strPartName.c_str());
                continue;
            }

            size_t found = it->find_last_of(".");

            if (found != string::npos)
            {
                std::string strExt = it->substr(found);

                if (strExt == TMP_TRACE_EXT)
                {
                    string strFullName = strTmpFilePath + '/' + *it;
                    retVal |= UpdateApiIndexes(strFullName, threadCopyTimestamps);
                }
            }
        }

        if (retVal)
        {
            // now update the timestamp data for the has_amd_memory_async_copy to include the data transfer timing
            for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
            {
                size_t part_found;
                part_found = it->find(strPartName);

                if (part_found == string::npos)
                {
                    Log(logMESSAGE, "Skipping file: %s. It does not contain: %s.\n", it->c_str(), strPartName.c_str());
                    continue;
                }

                size_t found = it->find_last_of(".");

                if (found != string::npos)
                {
                    std::string strExt = it->substr(found);

                    if (strExt == TMP_TIME_STAMP_EXT)
                    {
                        string strFullName = strTmpFilePath + '/' + *it;
                        UpdateAsyncCopyTimestamps(strFullName, threadCopyTimestamps);
                    }
                }
            }
        }

        if (retVal && !strAsyncCopyTSFile.empty())
        {
            // delete async copy timestamp file.
            remove(strAsyncCopyTSFile.c_str());
        }

        return retVal;
    }
}

bool HSAAtpFilePart::LoadAsyncCopyTimestamps(const std::string& strFile, ThreadCopyItemMap& threadCopyItemMap)
{
    char buf[MAX_LINE_SIZE];
    ifstream fin(strFile.c_str());

    if (!fin.is_open())
    {
        return false;
    }

    threadCopyItemMap.clear();

    while (!fin.eof())
    {
        memset(buf, 0, MAX_LINE_SIZE * sizeof(char));
        fin.getline(buf, MAX_LINE_SIZE);
        std::istringstream ss(buf);

        // check empty line.
        if (ss.str().empty())
        {
            continue;
        }

        int threadId = 0;
        AsyncCopyItem asyncCopyItem;

        ss >> threadId;
        ss >> asyncCopyItem.m_strSignalHandle;
        ss >> asyncCopyItem.m_start;
        ss >> asyncCopyItem.m_end;

        asyncCopyItem.m_apiIndex = 0;

        if (!ss.fail())
        {
           if (0 < threadCopyItemMap.count(threadId))
           {
               threadCopyItemMap[threadId].push_back(asyncCopyItem);
           }
           else
           {
               AsyncCopyItemList itemList;
               itemList.push_back(asyncCopyItem);
               threadCopyItemMap[threadId] = itemList;
           }
        }
        else
        {
            Log(logERROR, "Unable to parse async copy timestamp data\n");
        }
    }

    fin.close();

    return true;
}

bool HSAAtpFilePart::UpdateApiIndexes(const std::string strFile, ThreadCopyItemMap& threadCopyInfoMap)
{
    osThreadId tid;
    bool correctTidFile = IsCorrectTidFile(strFile, threadCopyInfoMap, tid);

    if (!correctTidFile)
    {
        return false;
    }

    AsyncCopyItemList itemList = threadCopyInfoMap[tid];

    char buf[MAX_LINE_SIZE];
    std::ifstream fin(strFile.c_str());

    if (!fin.is_open())
    {
        return false;
    }

    uint32_t curIndex = static_cast<uint32_t>(-1);

    while (!fin.eof())
    {
        memset(buf, 0, MAX_LINE_SIZE * sizeof(char));
        fin.getline(buf, MAX_LINE_SIZE);

        std::string line(buf);

        if (line.empty())
        {
            continue;
        }

        curIndex++;

        size_t equalSignPos = line.find_first_of("=");
        size_t openParenPos = line.find_first_of("(");

        if (openParenPos != string::npos)
        {
            size_t nameStartIndex = 0;

            // In HSA, not all APIs have a return value -- those with no return value do not have an equal sign before the API name
            if (equalSignPos == string::npos || equalSignPos > openParenPos)
            {
                equalSignPos = 0;
            }
            else
            {
                // If there is an equal sign, increment the start index (we expect a space before the name starts)
                nameStartIndex = equalSignPos + 2;
            }

            // Back up from the opening parentheses, this is where the name actually ends
            size_t nameEndIndex = openParenPos - 1;

            // Get the name
            size_t nameLength = nameEndIndex - nameStartIndex;

            if ((nameStartIndex < line.size()) && (nameStartIndex + nameLength <= line.size()))
            {
                std::string name = line.substr(nameStartIndex, nameEndIndex - nameStartIndex);

                if (0 != name.compare("hsa_amd_memory_async_copy"))
                {
                    continue;
                }

                // Parse the return value
                std::string returnValue;

                if (0 != equalSignPos)
                {
                    returnValue = line.substr(0, equalSignPos - 1);
                }

                if (0 != returnValue.compare("HSA_STATUS_SUCCESS"))
                {
                    continue;
                }

                // Parse the arg list
                size_t argListEndIndex = line.find_first_of(")");

                if (argListEndIndex != string::npos)
                {
                    size_t argListLength = argListEndIndex - openParenPos;
                    std::string argList;

                    if (nameEndIndex + argListLength < line.size())
                    {
                        argList = StringUtils::Trim(line.substr(nameEndIndex + 2, argListLength - 2));

                        size_t signalHandleStart = argList.find_last_of("{");

                        if (string::npos != signalHandleStart)
                        {
                            size_t signalHandleEnd = argList.find_last_of("}");

                            if (string::npos != signalHandleEnd)
                            {
                                std::string signalHandle = argList.substr(signalHandleStart + 1, signalHandleEnd - 1 - signalHandleStart);

                                for (auto it = itemList.begin(); it != itemList.end(); ++it)
                                {
                                    if (0 == it->m_apiIndex && 0 == it->m_strSignalHandle.compare(signalHandle))
                                    {
                                        it->m_apiIndex = curIndex;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    Log(logERROR, "Failed to parse API trace (%s), Unexpected data in input file.\n", line.c_str());
                    return false;
                }
            }
        }
        else
        {
            Log(logERROR, "Failed to parse API trace (%s), Unexpected data in input file.\n", line.c_str());
            return false;
        }
    }

    threadCopyInfoMap[tid] = itemList;

    fin.close();
    return true;
}

bool HSAAtpFilePart::UpdateAsyncCopyTimestamps(const std::string strFile, ThreadCopyItemMap threadCopyInfoMap)
{
    osThreadId tid;
    bool correctTidFile = IsCorrectTidFile(strFile, threadCopyInfoMap, tid);

    if (!correctTidFile)
    {
        return false;
    }

    AsyncCopyItemList itemList = threadCopyInfoMap[tid];

    std::vector<std::string> fileLines;
    bool fileUpdated = false;

    if (FileUtils::ReadFile(strFile, fileLines, false))
    {
        for (auto it = itemList.begin(); it != itemList.end(); ++it)
        {
            std::stringstream ss;
            ss << fileLines[it->m_apiIndex];
            ss << std::left << std::setw(21) << it->m_start;
            ss << std::left << std::setw(21) << it->m_end;

            fileLines[it->m_apiIndex] = ss.str();
            fileUpdated = true;
        }
    }

    if (fileUpdated)
    {
        remove(strFile.c_str());
        FileUtils::WriteFile(strFile, fileLines);
    }

    return true;
}

bool HSAAtpFilePart::IsCorrectTidFile(const std::string strFile, ThreadCopyItemMap threadCopyInfoMap, osThreadId& threadId)
{
    size_t tidStartPos = strFile.find_first_of("_");
    size_t tidEndPos = strFile.find_first_of(".");

    bool correctTidFile = false;

    if (string::npos != tidStartPos && string::npos != tidEndPos)
    {
        std::string tidString = strFile.substr(tidStartPos + 1, tidEndPos - 1);
        std::stringstream ss(tidString);
        ss >> threadId;

        if (!ss.fail())
        {
            correctTidFile = threadCopyInfoMap.count(threadId) > 0;
        }
    }

    if (!correctTidFile)
    {
        return false;
    }

    return true;
}