//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Data Transfer Analyzer
//==============================================================================

#include <sstream>
#include <vector>
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "DeviceInfoUtils.h"
#include "CLDataTransferAnalyzer.h"

using namespace std;
using namespace GPULogger;

#define NUM_ARG_CL_ENQUEUE_NDRANGE_KERENL 9
#define NUM_ARG_CL_CREATE_BUFFER          5
#define NUM_ARG_CL_ENQUEUE_MAP_BUFFER     10

CLDataTransferAnalyzer::CLDataTransferAnalyzer(CLAPIAnalyzerManager* p) : CLAPIAnalyzer(p)
{
    m_strName = "DataTransferAnalyzer";
    m_bRequireAPIFlattening = true;
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMapBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueNDRangeKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateKernelsInProgram);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clSetKernelArg);
}

CLDataTransferAnalyzer::~CLDataTransferAnalyzer(void)
{
}

void CLDataTransferAnalyzer::SetEnable(const AnalyzeOps& op)
{
    CLAPIAnalyzer::SetEnable(op);

    if (m_bEnabled)
    {
        m_pParent->EnableKernelArgsSetupAnalyzer();
    }
}

void CLDataTransferAnalyzer::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLAPIInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLAPIInfo)
    {
        if (pCLAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clCreateBuffer)
        {
            if (pCLAPIInfo->m_strRet != "NULL")
            {
                // extract mem_flag
                vector<string> output;
                StringUtils::Split(output, pCLAPIInfo->m_ArgList, string(";"));
                SpAssert(output.size() == NUM_ARG_CL_CREATE_BUFFER);

                if (output.size() == NUM_ARG_CL_CREATE_BUFFER)
                {
                    string handle = pCLAPIInfo->m_strRet;
                    string flag = output[1];
                    // we don't care about pointer reuse here, if it's used, replace the old flag.
                    m_BufferFlagMap[handle] = flag;
                }
            }
        }
        else if (pCLAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clEnqueueMapBuffer)
        {
            if (pCLAPIInfo->m_strRet != "NULL")
            {
                vector<string> output;
                StringUtils::Split(output, pCLAPIInfo->m_ArgList, string(";"));
                SpAssert(output.size() == NUM_ARG_CL_ENQUEUE_MAP_BUFFER);

                if (output.size() == NUM_ARG_CL_ENQUEUE_MAP_BUFFER)
                {
                    string strBuf = output[1];
                    string flag = output[3];

                    // search from bufferflagmap
                    BufferFlagMap::iterator it = m_BufferFlagMap.find(strBuf);
                    SpAssert(it != m_BufferFlagMap.end());

                    if (it != m_BufferFlagMap.end())
                    {
                        if (flag.find("CL_MAP_READ") != string::npos)
                        {
                            // Rule 1: MemFlag = CL_MEM_USE_PERSISTENT_MEM_AMD && MapFlag = CL_READ
                            if (it->second.find("CL_MEM_USE_PERSISTENT_MEM_AMD") != string::npos)
                            {
                                // match
                                APIAnalyzerMessage msg;
                                msg.type = MSGTYPE_BestPractices;
                                msg.uiSeqID = pCLAPIInfo->m_uiSeqID;
                                msg.uiDisplaySeqID = pCLAPIInfo->m_uiDisplaySeqID;
                                msg.bHasDisplayableSeqId = pCLAPIInfo->m_bHasDisplayableSeqId;
                                msg.uiTID = pCLAPIInfo->m_tid;
                                msg.strMsg = "Host Visible Device Memory(CL_MEM_USE_PERSISTENT_MEM_AMD) directly accessed by host, resulting in sub-optimal performance. Please use clEnqueueReadBuffer( .., buf, ..) or clEnqueueCopyBuffer( .., buf, zero copy host buffer, .. ) instead.";
                                m_msgList.push_back(msg);
                            }
                        }
                    }
                }
            }
        }
        else if (pCLAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clEnqueueNDRangeKernel)
        {
            if (pCLAPIInfo->m_strRet == "CL_SUCCESS")
            {
                vector<string> output;
                StringUtils::Split(output, pCLAPIInfo->m_ArgList, string(";"));
                SpAssert(output.size() == NUM_ARG_CL_ENQUEUE_NDRANGE_KERENL);

                if (output.size() == NUM_ARG_CL_ENQUEUE_NDRANGE_KERENL)
                {
                    CLEnqueueAPI* pEnQAPI = reinterpret_cast<CLEnqueueAPI*>(pCLAPIInfo);
                    SpAssertRet(nullptr != pEnQAPI);
                    bool bIsAPU = false;
                    AMDTDeviceInfoUtils::Instance()->IsAPU(pEnQAPI->m_strDevice.c_str(), bIsAPU);

                    if (!bIsAPU)
                    {
                        // find CL_MEM_ALLOC_HOST_PTR type buffer in all kernel args
                        bool bFound = false;
                        string strKernel = output[1];
                        CLKernelArgsSetupMap::const_iterator kit = m_pParent->GetKernelArgsSetupMap().find(strKernel);
                        SpAssert(kit != m_pParent->GetKernelArgsSetupMap().end());

                        if (kit != m_pParent->GetKernelArgsSetupMap().end())
                        {
                            for (CLKernelArgsSetup::const_iterator setupIt = kit->second.kernelArgsSetup.begin(); setupIt != kit->second.kernelArgsSetup.end(); ++setupIt)
                            {
                                if (setupIt->second[0] == '[')
                                {
                                    // kernel arg size == sizeof(void*), it means the kernel arg could be a mem type
                                    string strMemHandle = setupIt->second.substr(1, setupIt->second.length() - 2);
                                    BufferFlagMap::iterator bufIt = m_BufferFlagMap.find(strMemHandle);

                                    if (bufIt != m_BufferFlagMap.end() && bufIt->second.find("CL_MEM_ALLOC_HOST_PTR") != string::npos)
                                    {
                                        bFound = true;
                                        break;
                                    }
                                }
                            }

                            // Rule 2: Is NOT APU && MemFlag = CL_MEM_ALLOC_HOST_PTR
                            if (bFound)
                            {
                                // match
                                APIAnalyzerMessage msg;
                                msg.type = MSGTYPE_BestPractices;
                                msg.uiSeqID = pCLAPIInfo->m_uiSeqID;
                                msg.uiDisplaySeqID = pCLAPIInfo->m_uiDisplaySeqID;
                                msg.bHasDisplayableSeqId = pCLAPIInfo->m_bHasDisplayableSeqId;
                                msg.uiTID = pCLAPIInfo->m_tid;
                                msg.strMsg = "Device visible Host Memory(CL_MEM_ALLOC_HOST_PTR) directly accessed by Non-Fusion device, resulting in sub-optimal performance.";
                                m_msgList.push_back(msg);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CLDataTransferAnalyzer::Analyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void CLDataTransferAnalyzer::EndAnalyze()
{

}
