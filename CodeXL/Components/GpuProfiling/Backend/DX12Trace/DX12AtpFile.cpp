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
#include "DX12AtpFile.h"
#include "../Common/OSUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "../Common/Version.h"
#include "../Common/Defs.h"
#include "../Common/ATPFileUtils.h"

//Infra
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osMachine.h>

using namespace std;
using namespace GPULogger;

const static std::string s_str_apiTypeHeaderPrefix = "//API=";
const static std::string s_str_threadIDHeaderPrefix = "//ThreadID=";
const static std::string s_str_threadAPICountHeaderPrefix = "//ThreadAPICount=";

// The DX12 timestamps are double number. The data structures expect long long numbers, so we multiply the double timestamp by a GP_DX_TIMESTAMP_MILLISECONDS_TO_NANOSECONDS_FACTOR
// to make sure that we get integer value. In the front-end, we will perform the opposite operation
#define GP_DX_TIMESTAMP_MILLISECONDS_TO_NANOSECONDS_FACTOR 1000000

#define KILO_BYTE 1024
#define MEGA_BYTE (KILO_BYTE * 1024)
static const size_t s_min_vm_size_for_api_parse(100 * MEGA_BYTE);

DX12AtpFilePart::DX12AtpFilePart(const Config& config, bool shouldReleaseMemory) : IAtpFilePart(config, shouldReleaseMemory),
    m_currentParsedTraceType(API), m_currentParsedThreadID(0), m_currentParsedThreadAPICount(0)
{
#define PART_NAME "dx12"
    m_strPartName = PART_NAME;
    m_sections.push_back("//CodeXL Frame Trace");
    m_sections.push_back("API=DX12");
    m_sections.push_back("GPU Trace");
#undef PART_NAME
}

DX12AtpFilePart::~DX12AtpFilePart(void)
{
    if (m_shouldReleaseMemory)
    {
        // clean up all API object
        for (DX12APIInfoMap::iterator it = m_DXAPIInfoMap.begin(); it != m_DXAPIInfoMap.end(); it++)
        {
            std::vector<DX12APIInfo*>& apiList = it->second;

            for (std::vector<DX12APIInfo*>::iterator listIt = apiList.begin(); listIt != apiList.end(); listIt++)
            {
                if ((*listIt) != NULL)
                {
                    delete *listIt;
                }
            }

            apiList.clear();
        }

        m_DXAPIInfoMap.clear();
    }
}

void DX12AtpFilePart::WriteHeaderSection(SP_fileStream& sout)
{
    // Currently the DX12AtpFilePart class is only implementing the read of the file, therefore this function is not implemented
    GT_UNREFERENCED_PARAMETER(sout);
}


bool DX12AtpFilePart::WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID)
{
    // Currently the DX12AtpFilePart class is only implementing the read of the file, therefore this function is not implemented
    GT_UNREFERENCED_PARAMETER(sout);
    GT_UNREFERENCED_PARAMETER(strTmpFilePath);
    GT_UNREFERENCED_PARAMETER(strPID);
    return false;
}


/// GPU Trace response format is as follows :
/// CommandQueuePtr D3D12_COMMAND_LIST_TYPE CommandListPtr APIType FuncId APIInterface_FunctionName(Arguments) = ReturnValue StartMillisecond EndMillisecond SampleId
/// 0x02B7B12E210 0 0x02B7B12E9E0 128 5 ID3D12GraphicsCommandList_DrawIndexedInstanced(3, 1, 6, 0, 0) = void 6122.031 6122.967 2
bool DX12AtpFilePart::ParseGPUAPICallString(const std::string& apiStr, DX12GPUTraceInfo& apiInfo)
{
    bool retVal = false;

    char* pCurrentToken = strtok((char*)apiStr.data(), " ");

    if (pCurrentToken != nullptr)
    {
        // Set the command queue string
        apiInfo.m_commandQueuePtrStr = pCurrentToken;

        pCurrentToken = strtok(nullptr, " ");

        if (pCurrentToken != nullptr)
        {
            // Set the command list type
            apiInfo.m_commandListType = atoi(pCurrentToken);

            pCurrentToken = strtok(nullptr, " ");

            if (pCurrentToken != nullptr)
            {
                apiInfo.m_commandListPtrStr = pCurrentToken;

                pCurrentToken = strtok(nullptr, " ");

                if (pCurrentToken != nullptr)
                {
                    // Set the API type
                    apiInfo.m_apiType = (eAPIType)atoi(pCurrentToken);

                    pCurrentToken = strtok(nullptr, " ");

                    if (pCurrentToken != nullptr)
                    {
                        apiInfo.m_apiId = (FuncId)atoi(pCurrentToken);

                        pCurrentToken = strtok(nullptr, " ");

                        if (pCurrentToken != nullptr)
                        {
                            apiInfo.m_strName = pCurrentToken;

                            // Append the strings until we close the parameters brackets
                            while ((apiInfo.m_strName.find(')') == std::string::npos) && (pCurrentToken != nullptr))
                            {
                                pCurrentToken = strtok(nullptr, " ");
                                apiInfo.m_strName.append(" ");
                                apiInfo.m_strName.append(pCurrentToken);
                            }

                            // If we got here, we already closed the ')'
                            if (apiInfo.m_strName.find(')') != std::string::npos)
                            {
                                size_t argsOpenPos = apiInfo.m_strName.find('(');
                                size_t argsClosePos = apiInfo.m_strName.find(')');
                                apiInfo.m_ArgList = apiInfo.m_strName.substr(argsOpenPos + 1, argsClosePos - argsOpenPos - 1);

                                // The arguments string may be marked with +'s to indicate that an argument is a special API object.
                                // Remove all instances of "+" from the arguments string
                                static const char p = '+';

                                for (string::size_type i = apiInfo.m_ArgList.find(p); i != string::npos; i = apiInfo.m_ArgList.find(p))
                                {
                                    apiInfo.m_ArgList.erase(i, 1);
                                }

                                apiInfo.m_strName = apiInfo.m_strName.substr(0, argsOpenPos);

                                // Read the '='
                                pCurrentToken = strtok(nullptr, " ");

                                pCurrentToken = strtok(nullptr, " ");

                                if (pCurrentToken != nullptr)
                                {
                                    apiInfo.m_strRet = pCurrentToken;
                                }

                                double timeStartDouble = 0, timeEndDouble = 0;

                                pCurrentToken = strtok(nullptr, " ");

                                if (pCurrentToken != nullptr)
                                {
                                    timeStartDouble = atof(pCurrentToken);
                                }

                                pCurrentToken = strtok(nullptr, " ");

                                if (pCurrentToken != nullptr)
                                {
                                    timeEndDouble = atof(pCurrentToken);
                                }

                                // The timestamps are stored in a double number, in milliseconds units.
                                // We multiply it by 1000000, to keep the accuracy, and we will refer to it as nanoseconds
                                apiInfo.m_ullStart = ULONGLONG(timeStartDouble * GP_DX_TIMESTAMP_MILLISECONDS_TO_NANOSECONDS_FACTOR);
                                apiInfo.m_ullEnd = ULONGLONG(timeEndDouble * GP_DX_TIMESTAMP_MILLISECONDS_TO_NANOSECONDS_FACTOR);

                                pCurrentToken = strtok(nullptr, " ");

                                if (pCurrentToken != nullptr)
                                {
                                    apiInfo.m_sampleId = atoi(pCurrentToken);
                                    retVal = true;
                                }
                            }

                        }
                    }
                }

            }
        }
    }

    return retVal;
}

bool DX12AtpFilePart::ParseSectionHeaderLine(const string& line)
{
    // Assume that this is not a section line by default:
    bool retVal = false;

    if ((line[0] == '/') && (line[1] == '/'))
    {
        retVal = true;

        if (line.find("//==GPU Trace==") == 0)
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
            for (std::vector<IParserListener<DX12APIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); it++)
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

bool DX12AtpFilePart::ParseCPUAPICallString(const std::string& apiStr, DX12APIInfo& apiInfo)
{
    bool retVal = false;

    char* pCurrentToken = strtok((char*)apiStr.data(), " ");

    if (pCurrentToken != nullptr)
    {
        // Get the thread ID from the saved member
        apiInfo.m_tid = m_currentParsedThreadID;

        // Get the API type
        apiInfo.m_apiType = (eAPIType)atoi(pCurrentToken);

        pCurrentToken = strtok(nullptr, " ");

        if (pCurrentToken != nullptr)
        {
            // Get the API ID
            apiInfo.m_apiId = (FuncId)atoi(pCurrentToken);

            pCurrentToken = strtok(nullptr, " ");

            if (pCurrentToken != nullptr)
            {
                apiInfo.m_interfacePtrStr = pCurrentToken;
            }

            pCurrentToken = strtok(nullptr, " ");

            if (pCurrentToken != nullptr)
            {
                apiInfo.m_strName = pCurrentToken;
            }

            // Append the strings until we close the parameters brackets
            int leftParenthesesCounter = 1;

            if (apiInfo.m_strName.find(')') == std::string::npos)
            {
                while ((leftParenthesesCounter > 0) && (pCurrentToken != nullptr))
                {
                    pCurrentToken = strtok(nullptr, " ");

                    // Append the space contained in the parameters list
                    apiInfo.m_strName.append(" ");

                    // Append the next token to the name (will later be parsed into the arguments list)
                    apiInfo.m_strName.append(pCurrentToken);

                    if (strchr(pCurrentToken, ')') != nullptr)
                    {
                        leftParenthesesCounter--;
                    }

                    if (strchr(pCurrentToken, '(') != nullptr)
                    {
                        leftParenthesesCounter++;
                    }
                }
            }

            // If we got here, we already closed the ')'
            if (apiInfo.m_strName.find(')') != std::string::npos)
            {
                size_t argsOpenPos = apiInfo.m_strName.find_first_of('(');
                size_t argsClosePos = apiInfo.m_strName.find_last_of(')');
                apiInfo.m_ArgList = apiInfo.m_strName.substr(argsOpenPos + 1, argsClosePos - argsOpenPos - 1);

                // Some arguments may be bound by bracket characters - these are array parameters that are expanded to display
                // the values of the elements inside the arrays. Bracket characters will be retained for display in the UI.
                // The arguments string may be marked with +'s to indicate that an argument is a special API object.
                // Remove all instances of "+" from the arguments string
                static const char p = '+';

                for (string::size_type i = apiInfo.m_ArgList.find(p); i != string::npos; i = apiInfo.m_ArgList.find(p))
                {
                    apiInfo.m_ArgList.erase(i, 1);
                }

                apiInfo.m_strName = apiInfo.m_strName.substr(0, argsOpenPos);

                // Read the '='
                pCurrentToken = strtok(nullptr, " ");

                pCurrentToken = strtok(nullptr, " ");

                if (pCurrentToken != nullptr)
                {
                    apiInfo.m_strRet = pCurrentToken;
                }

                double timeStartDouble = 0, timeEndDouble = 0;

                pCurrentToken = strtok(nullptr, " ");

                if (pCurrentToken != nullptr)
                {
                    timeStartDouble = atof(pCurrentToken);
                }

                pCurrentToken = strtok(nullptr, " ");

                if (pCurrentToken != nullptr)
                {
                    timeEndDouble = atof(pCurrentToken);
                }

                // The timestamps are stored in a double number, in milliseconds units.
                // We multiply it by 1000000, to keep the accuracy, and we will refer to it as nanoseconds
                apiInfo.m_ullStart = ULONGLONG(timeStartDouble * GP_DX_TIMESTAMP_MILLISECONDS_TO_NANOSECONDS_FACTOR);
                apiInfo.m_ullEnd = ULONGLONG(timeEndDouble * GP_DX_TIMESTAMP_MILLISECONDS_TO_NANOSECONDS_FACTOR);

                pCurrentToken = strtok(nullptr, " ");

                if (pCurrentToken != nullptr)
                {
                    apiInfo.m_sampleId = atoi(pCurrentToken);
                    retVal = true;
                }
            }
        }

    }

    return retVal;
}

bool DX12AtpFilePart::Parse(std::istream& in, std::string& outErrorMsg)
{
    bool retVal = true;
    int currentParsedAPI = 0;
    ErrorMessageUpdater errorMessageUpdater(outErrorMsg, this);
    size_t fileLineCount = 0;

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
                //**************** Patch for FA, we don't want to crash if VM is exceeded ******************************/
                /***************** this should be removed when we implement SqlLite based parsing solution**************/
                SP_TODO("Remove this patch when we implement SqlLite based solution for parsing")
                ++fileLineCount;

                //check every 1000 lines if we still got enough virtual memory
                if (fileLineCount % 1000 == 0)
                {
                    gtUInt64 totalRamSizet = 0;
                    gtUInt64 availRamSizet = 0;
                    gtUInt64 totalPageSizet = 0;
                    gtUInt64 availPageSizet = 0;
                    gtUInt64 totalVirtualSizet = 0;
                    gtUInt64 availVirtualSizet = 0;

                    bool res = osGetLocalMachineMemoryInformation(totalRamSizet, availRamSizet, totalPageSizet, availPageSizet, totalVirtualSizet, availVirtualSizet);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    if (res && availVirtualSizet < s_min_vm_size_for_api_parse)
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
                    if (res && availPageSizet < s_min_vm_size_for_api_parse)
#endif
                    {
                        m_shouldStopParsing = true;
                        m_bWarning = false;
                        m_strWarningMsg = outErrorMsg = "Low on Virtual Memory, stopped processing";
                        retVal = false;
                        break;
                    }
                }

                /***************************************  End Patch     ****************************************************/

                bool rcParseLine = false;
                bool isSectionHeader = ParseSectionHeaderLine(line);


                if (isSectionHeader)
                {
                    currentParsedAPI = 0;
                }

                if (!isSectionHeader)
                {
                    // Create the API info object
                    DX12APIInfo* pAPIInfo = nullptr;

                    if (m_currentParsedTraceType == API)
                    {
                        pAPIInfo = new DX12APIInfo;
                        rcParseLine = ParseCPUAPICallString(line, *pAPIInfo);
                    }
                    else
                    {
                        DX12GPUTraceInfo* pGPUTraceInfo = new DX12GPUTraceInfo;
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

                        m_DXAPIInfoMap[pAPIInfo->m_tid].push_back(pAPIInfo);

                        pAPIInfo->m_uiSeqID = apiIndex++;
                        pAPIInfo->m_uiDisplaySeqID = apiIndex;
                        pAPIInfo->m_bHasDisplayableSeqId = true;

                        if (retVal)
                        {
                            for (std::vector<IParserListener<DX12APIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
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

bool DX12AtpFilePart::ParseHeader(const std::string& strKey, const std::string& strVal)
{
    SP_UNREFERENCED_PARAMETER(strKey);
    SP_UNREFERENCED_PARAMETER(strVal);
    return true;
}

void DX12AtpFilePart::SaveToFile(const std::string& strTmpFilePath, const std::string& strPID)
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
