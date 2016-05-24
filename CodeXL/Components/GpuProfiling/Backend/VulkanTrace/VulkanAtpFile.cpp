//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/Backend/CLTraceAgent/CLAtpFile.cpp $
/// \version $Revision: #44 $
/// \brief CL Atp File writer and parser
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/Backend/CLTraceAgent/CLAtpFile.cpp#44 $
//
// Last checkin:  $DateTime: 2015/09/01 08:35:05 $
// Last edited by: $Author: salgrana $
//=====================================================================
//   ( C ) AMD, Inc. 2010 All rights reserved.
//=====================================================================

#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include "VulkanAtpFile.h"
#include "../Common/OSUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "../Common/Version.h"
#include "../Common/Defs.h"
#include "../Common/ATPFileUtils.h"

#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>

using namespace std;
using namespace GPULogger;

const static std::string s_str_apiTypeHeaderPrefix = "//API=";
const static std::string s_str_threadIDHeaderPrefix = "//ThreadID=";
const static std::string s_str_threadAPICountHeaderPrefix = "//ThreadAPICount=";

// The Vulkan timestamps are double number. The data structures expect long long numbers, so we multiply the double timestamp by a GP_VK_TIMESTAMP_FACTOR
// to make sure that we get integer value. In the front-end, we will perform the opposite operation
#define GP_VK_TIMESTAMP_FACTOR 100000

VKAtpFilePart::VKAtpFilePart(const Config& config, bool shouldReleaseMemory) : IAtpFilePart(config, shouldReleaseMemory),
    m_currentParsedTraceType(API), m_currentParsedThreadID(0), m_currentParsedThreadAPICount(0),
    m_cpuStart(0), m_cpuEnd(0), m_gpuStart(0)
{
#define PART_NAME "vulkan"
    m_strPartName = PART_NAME;
    m_sections.push_back("API=Vulkan");
    m_sections.push_back("GPU Trace");
#undef PART_NAME
}

VKAtpFilePart::~VKAtpFilePart(void)
{
    if (m_shouldReleaseMemory)
    {
        // clean up all API object
        for (VKAPIInfoMap::iterator it = m_VKAPIInfoMap.begin(); it != m_VKAPIInfoMap.end(); it++)
        {
            std::vector<VKAPIInfo*>& apiList = it->second;

            for (std::vector<VKAPIInfo*>::iterator listIt = apiList.begin(); listIt != apiList.end(); listIt++)
            {
                if ((*listIt) != NULL)
                {
                    delete *listIt;
                }
            }

            apiList.clear();
        }

        m_VKAPIInfoMap.clear();
    }
}

void VKAtpFilePart::WriteHeaderSection(SP_fileStream& sout)
{
    // Currently the VKAtpFilePart class is only implementing the read of the file, therefore this function is not implemented
    GT_UNREFERENCED_PARAMETER(sout);
}


bool VKAtpFilePart::WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID)
{
    // Currently the VKAtpFilePart class is only implementing the read of the file, therefore this function is not implemented
    GT_UNREFERENCED_PARAMETER(sout);
    GT_UNREFERENCED_PARAMETER(strTmpFilePath);
    GT_UNREFERENCED_PARAMETER(strPID);
    return false;
}


bool VKAtpFilePart::ParseGPUAPICallString(const std::string& apiStr, VKGPUTraceInfo& apiInfo)
{
    bool retVal = false;

    string temp;
    istringstream ss(apiStr);

    ss >> apiInfo.m_queueIndexStr;
    CHECK_SS_ERROR(ss);
    ss >> apiInfo.m_commandListType;
    CHECK_SS_ERROR(ss);

    ss >> apiInfo.m_commandBufferHandleStr;
    CHECK_SS_ERROR(ss);

    int intVal = 0;
    ss >> intVal;
    CHECK_SS_ERROR(ss);
    apiInfo.m_apiType = (vkAPIType)intVal;

    ss >> intVal;
    CHECK_SS_ERROR(ss);
    apiInfo.m_apiId = (VkFuncId)intVal;

    ss >> apiInfo.m_strName;
    CHECK_SS_ERROR(ss);

    // Append the strings until we close the parameters brackets
    while ((apiInfo.m_strName.find(')') == std::string::npos) && !ss.eof())
    {
        ss >> temp;
        CHECK_SS_ERROR(ss);
        apiInfo.m_strName.append(temp);
    }

    // If we got here, we already closed the ')'
    if (apiInfo.m_strName.find(')') != std::string::npos)
    {
        size_t argsOpenPos = apiInfo.m_strName.find('(');
        size_t argsClosePos = apiInfo.m_strName.find(')');
        apiInfo.m_ArgList = apiInfo.m_strName.substr(argsOpenPos + 1, argsClosePos - argsOpenPos - 1);
        apiInfo.m_strName = apiInfo.m_strName.substr(0, argsOpenPos);

        // Read the '='
        ss >> temp;
        CHECK_SS_ERROR(ss);

        ss >> apiInfo.m_strRet;
        CHECK_SS_ERROR(ss);

        double timeStartDouble, timeEndDouble;
        ss >> timeStartDouble;
        CHECK_SS_ERROR(ss);
        ss >> timeEndDouble;
        CHECK_SS_ERROR(ss);

        apiInfo.m_ullStart = ULONGLONG(timeStartDouble * GP_VK_TIMESTAMP_FACTOR);
        apiInfo.m_ullEnd = ULONGLONG(timeEndDouble * GP_VK_TIMESTAMP_FACTOR);

        /// Store the GPU start time if it is not stored yet
        /// GPU timestamps to fit the CPU timeline
        if ((m_gpuStart == 0) || (m_gpuStart > apiInfo.m_ullStart))
        {
            m_gpuStart = apiInfo.m_ullStart;
        }

        if (m_cpuStart != 0)
        {
            // If the CPU start time is stored, "normalize" the GPU times according to the CPU time
            apiInfo.m_ullStart = apiInfo.m_ullStart - m_gpuStart + m_cpuEnd - (m_cpuEnd - m_cpuStart) / 3;
            apiInfo.m_ullEnd = apiInfo.m_ullEnd - m_gpuStart + m_cpuEnd - (m_cpuEnd - m_cpuStart) / 3;
        }

        // Checking the status is done before attempting to read 'sample id' because 
        // the reading of 'sample id' is not critical and can fail without affecting the rest of the parsing.
        retVal = (!ss.fail());

        apiInfo.m_sampleId = 0;
        ss >> apiInfo.m_sampleId;
    }

    return retVal;
}

bool VKAtpFilePart::ParseSectionHeaderLine(const string& line)
{
    // Assume that this is not a section line by default:
    bool retVal = false;

    if ((line[0] == '/') && (line[1] == '/'))
    {
        retVal = true;

        if (line == "//==GPU Trace==")
        {
            // Switch to GPU trace
            m_currentParsedTraceType = GPU;
        }

        else if (line.find(s_str_apiTypeHeaderPrefix) != string::npos)
        {
            // Parse the API string
            m_apiStr = line.substr(s_str_apiTypeHeaderPrefix.size(), line.size() - s_str_apiTypeHeaderPrefix.size());
        }
        else if (line.find(s_str_threadIDHeaderPrefix) != string::npos)
        {
            string threadIDStr;
            threadIDStr = line.substr(s_str_threadIDHeaderPrefix.size(), line.size() - s_str_threadIDHeaderPrefix.size());
            istringstream ss(threadIDStr);
            ss >> m_currentParsedThreadID;
            CHECK_SS_ERROR(ss);
        }
        else if (line.find(s_str_threadAPICountHeaderPrefix) != string::npos)
        {
            string threadIDStr;
            threadIDStr = line.substr(s_str_threadAPICountHeaderPrefix.size(), line.size() - s_str_threadAPICountHeaderPrefix.size());
            istringstream ss(threadIDStr);
            ss >> m_currentParsedThreadAPICount;
            CHECK_SS_ERROR(ss);

            // Update the listeners with the API number for this thread
            for (std::vector<IParserListener<VKAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); it++)
            {
                if ((*it) != NULL)
                {
                    (*it)->SetAPINum(m_currentParsedThreadID, m_currentParsedThreadAPICount);
                }
            }

        }
    }

    return retVal;
}

/// Expecting the following API call format:
/// Type
/// vkAPIType   VkFuncId    InterfacePtr       Interface_Call                       Args                                     = Result     StartTime       EndTime        GPUCallIndex
/// 128         90              0x0000000000000000 NonTrackedObject_vkBeginCommandBuffer(0x00000001362066F0, 0x000000009F7DE710) = VK_SUCCESS 89349181.540584 89365867.767216 0
bool VKAtpFilePart::ParseCPUAPICallString(const std::string& apiStr, VKAPIInfo& apiInfo)
{
    bool retVal = false;
    string temp;
    istringstream ss(apiStr);

    // Get the thread ID from the saved member
    apiInfo.m_tid = m_currentParsedThreadID;

    // Get the API type
    int intVal = 0;
    ss >> intVal;
    CHECK_SS_ERROR(ss);
    apiInfo.m_apiType = (vkAPIType)intVal;

    // Get the API ID
    ss >> intVal;
    CHECK_SS_ERROR(ss);
    apiInfo.m_apiId = (VkFuncId)intVal;

    // Get the interface name
    ss >> apiInfo.m_interfacePtrStr;
    CHECK_SS_ERROR(ss);

    // Get the interface name
    ss >> apiInfo.m_strName;
    CHECK_SS_ERROR(ss);

    // Append the strings until we close the parameters brackets
    while ((apiInfo.m_strName.find(')') == std::string::npos) && !ss.eof())
    {
        ss >> temp;
        CHECK_SS_ERROR(ss);
        apiInfo.m_strName.append(temp);
    }

    // If we got here, we already closed the ')'
    if (apiInfo.m_strName.find(')') != std::string::npos)
    {
        size_t argsOpenPos = apiInfo.m_strName.find('(');
        size_t argsClosePos = apiInfo.m_strName.find(')');
        apiInfo.m_ArgList = apiInfo.m_strName.substr(argsOpenPos + 1, argsClosePos - argsOpenPos - 1);
        apiInfo.m_strName = apiInfo.m_strName.substr(0, argsOpenPos);

        // Read the '='
        ss >> temp;
        CHECK_SS_ERROR(ss);

        ss >> apiInfo.m_strRet;
        CHECK_SS_ERROR(ss);

        double timeStartDouble, timeEndDouble;
        ss >> timeStartDouble;
        CHECK_SS_ERROR(ss);
        ss >> timeEndDouble;
        CHECK_SS_ERROR(ss);

        apiInfo.m_ullStart = ULONGLONG(timeStartDouble * GP_VK_TIMESTAMP_FACTOR);
        apiInfo.m_ullEnd = ULONGLONG(timeEndDouble * GP_VK_TIMESTAMP_FACTOR);

        // Store the CPU start and end time (if not stored yet)
        if (m_cpuStart == 0)
        {
            m_cpuStart = apiInfo.m_ullStart;
        }
        if (apiInfo.m_ullEnd > m_cpuEnd)
        {
            m_cpuEnd = apiInfo.m_ullEnd;
        }

        ss >> apiInfo.m_sampleId;
        CHECK_SS_ERROR(ss);

        retVal = (!ss.fail());
    }

    return retVal;
}

bool VKAtpFilePart::Parse(std::istream& in, std::string& outErrorMsg)
{
    bool retVal = true;
    int currentParsedAPI = 0;
    ErrorMessageUpdater errorMessageUpdater(outErrorMsg, this);

    do
    {
        if (m_shouldStopParsing)
        {
            break;
        }

        // Map containing the already parsed threads. When a new thread is being parsed, apiIndex should be reset
        std::map <osThreadId, bool> threadsMap;
        int apiIndex = -1;
        string line;
        bool rc = ReadLine(in, line);

        while (!in.eof() && rc)
        {
            // Skip empty lines in the trace
            if (!line.empty() && (line != "NODATA"))
            {

                bool rcParseLine = false;
                bool isSectionHeader = ParseSectionHeaderLine(line);


                if (isSectionHeader)
                {
                    currentParsedAPI = 0;
                }

                if (!isSectionHeader)
                {
                    // Create the API info object
                    VKAPIInfo* pAPIInfo = nullptr;

                    if (m_currentParsedTraceType == API)
                    {
                        pAPIInfo = new VKAPIInfo;
                        rcParseLine = ParseCPUAPICallString(line, *pAPIInfo);
                    }
                    else
                    {
                        VKGPUTraceInfo* pGPUTraceInfo = new VKGPUTraceInfo;
                        pAPIInfo = pGPUTraceInfo;
                        rcParseLine = ParseGPUAPICallString(line, *pGPUTraceInfo);
                    }

                    if (rcParseLine)
                    {
                        // Add this thread to the map, and reset the API index if necessary
                        if (threadsMap.find(pAPIInfo->m_tid) == threadsMap.end())
                        {
                            apiIndex = 0;
                            threadsMap[pAPIInfo->m_tid] = true;
                        }

                        m_VKAPIInfoMap[pAPIInfo->m_tid].push_back(pAPIInfo);

                        pAPIInfo->m_uiSeqID = apiIndex++;
                        pAPIInfo->m_uiDisplaySeqID = apiIndex;
                        pAPIInfo->m_bHasDisplayableSeqId = true;

                        if (retVal)
                        {
                            for (std::vector<IParserListener<VKAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
                            {
                                if ((*it) != nullptr)
                                {
                                    (*it)->OnParse(pAPIInfo, m_shouldStopParsing);
                                }
                            }
                        }
                    }
                    else
                    {
                        delete pAPIInfo;
                        SpBreak("Failed parsing");
                    }

                    // Update the progress bar
                    ReportProgress("Parsing the frame data", currentParsedAPI++, m_currentParsedThreadAPICount);
                }
            }

            if (m_shouldStopParsing)
            {
                break;
            }

            // Read the next line
            rc = ReadLine(in, line);
        }
    }
    while (!in.eof());

    return retVal;
}

bool VKAtpFilePart::ParseHeader(const std::string& strKey, const std::string& strVal)
{
    SP_UNREFERENCED_PARAMETER(strKey);
    SP_UNREFERENCED_PARAMETER(strVal);
    return true;
}

void VKAtpFilePart::SaveToFile(const std::string& strTmpFilePath, const std::string& strPID)
{
    stringstream ss;

    std::string strExtension = FileUtils::GetFileExtension(m_config.strOutputFile);

    if (strExtension != TRACE_EXT)
    {
        if (strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .csv or .occupancy and append .atp
            string strBaseFileName = FileUtils::GetBaseFileName(m_config.strOutputFile);
            ss << strBaseFileName << "." << TRACE_EXT;
        }
        else
        {
            // append .atp
            ss << m_config.strOutputFile << "." << TRACE_EXT;
        }
    }
    else
    {
        // use original name
        ss << m_config.strOutputFile;
    }

    string strOutputFile = ss.str();

    SP_fileStream fout(strOutputFile.c_str());

    if (fout.fail())
    {
        cout << "Failed to write to file " << strOutputFile << endl;
        return;
    }

    fout << ATP_FILE_HEADER_TraceFileVersion << "=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << endl;
    fout << ATP_FILE_HEADER_ProfilerVersion << "=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER << endl;
    fout << ATP_FILE_HEADER_Application << "=" << m_config.strInjectedApp.asUTF8CharArray() << endl;
    fout << ATP_FILE_HEADER_ApplicationArgs << "=" << m_config.strInjectedAppArgs.asUTF8CharArray() << endl;
    fout << ATP_FILE_HEADER_WorkingDirectory << "=" << m_config.strWorkingDirectory.asUTF8CharArray() << endl;

    if (m_config.mapEnvVars.size() > 0)
    {
        fout << ATP_FILE_HEADER_FullEnvironment << "=" << (m_config.bFullEnvBlock  ? "True" : "False") << endl;

        for (EnvVarMap::const_iterator it = m_config.mapEnvVars.begin(); it != m_config.mapEnvVars.end(); ++it)
        {
            fout << ATP_FILE_HEADER_EnvVar << "=" << (it->first).asUTF8CharArray() << "=" << (it->second).asUTF8CharArray() << endl;
        }
    }

    fout << ATP_FILE_HEADER_UserTimer << "=" << (m_config.bUserTimer ? "True" : "False") << endl;

    fout << ATP_FILE_HEADER_OSVersion << "=" << OSUtils::Instance()->GetOSInfo().c_str() << endl;
    fout << ATP_FILE_HEADER_DisplayName << "=" << m_config.strSessionName.c_str() << endl;

    WriteHeaderSection(fout);
    WriteContentSection(fout, strTmpFilePath, strPID);
    fout.close();
}
