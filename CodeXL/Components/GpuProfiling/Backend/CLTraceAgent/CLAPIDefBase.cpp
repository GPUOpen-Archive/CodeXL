//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines the CLAPI abstract base class.
//==============================================================================

#include "../Common/Logger.h"
#include "CLAPIDefBase.h"
#include "CLAPIInfoManager.h"

using namespace std;
using namespace GPULogger;

#ifndef WIN32
// On Linux, the search heuristic is simple, the first module that is not profiler nor libOpenCL is the application.
void CLAPIBase::CreateStackEntry()
{
    m_strName = CLStringUtils::GetCLAPINameString(m_type);

    if (m_pStackEntry != NULL)
    {
        return;
    }

    for (vector<StackEntry>::iterator it = m_stack.begin(); it != m_stack.end(); ++it)
    {
        if (it->m_strModName.empty())
        {
            continue;
        }

        // CodeXLGpuProfiler has to make sure libCLTraceAgent is the first agent in multi-agent case.
        if (it->m_strModName.find(GPU_PROFILER_LIBRARY_NAME_PREFIX "CLTraceAgent") == string::npos && it->m_strModName.find("libOpenCL") == string::npos)
        {
            // found
            StackTracer::Instance()->GetSymbolName(it->m_dwAddress, *it);
            m_pStackEntry = new(nothrow) StackEntry(*it);

            if (m_pStackEntry != NULL)
            {
                m_pStackEntry->m_strSymName = "NA";
            }

            break;
        }
    }
}
#else

bool CLAPIBase::IsValidStackEntry(const std::string& strSymName, const std::string& strModName)
{
    // make sure the symbol is not from the cpp cl wrapper && the module isn't the OpenCL module or this profiler module
    return (!strSymName.empty() && strSymName.find("cl::") == string::npos &&
            strModName.find("OpenCL") == string::npos &&
            strModName.find(GPU_PROFILER_LIBRARY_NAME_PREFIX "CLTraceAgent") == string::npos);
}

void CLAPIBase::CreateStackEntry()
{
    m_strName = CLStringUtils::GetCLAPINameString(m_type);

    if (m_pStackEntry != NULL)
    {
        return;
    }

#ifdef DEBUG_ST
    stringstream ss;
#endif
    bool bFound = false;
    bool bRetrievedAllSymbols = true;
    unsigned int nIdx = (unsigned int) - 1;
    unsigned int i = 0;

    for (vector<StackEntry>::iterator it = m_stack.begin(); it != m_stack.end(); ++it)
    {
        StackTracer::Instance()->GetSymbolName(it->m_dwAddress, *it);
        string symStr = it->m_strSymName;
        string modStr = it->m_strModName;
#ifdef DEBUG_ST

        if (symStr.empty())
        {
            ss << it->m_dwAddress << "\t(" << modStr << ")";
        }
        else
        {
            ss << symStr << "(" << modStr << ")\t";
        }

#endif

        if (bFound)
        {
            // check whether current symbol name == API name
            if (symStr == m_strName)
            {
                nIdx = i + 1;
#ifndef DEBUG_ST
                bRetrievedAllSymbols = false;
                break;
#endif
            }
            else if (nIdx == (unsigned int) - 1 && IsValidStackEntry(symStr, modStr))
            {
                nIdx = i;
            }
        }

#ifdef _DEBUG

        if (symStr.find("CL_API_TRACE") != std::string::npos)
#else
        if (symStr.find("clAgent_OnLoad") != std::string::npos)
#endif
        {
            bFound = true;
        }

        i++;
    }

#ifndef DEBUG_ST

    if (nIdx < m_stack.size())
    {
        if (!bRetrievedAllSymbols)
        {
            StackTracer::Instance()->GetSymbolName(m_stack[nIdx].m_dwAddress, m_stack[nIdx]);
        }

        StackEntry stackEntry = m_stack[nIdx];

        // if the calling stack frame does not have debug info (file is an empty string),
        // then keep going up the stack to see if a frame with debug info is found
        if (m_stack[nIdx].m_strFile.empty() && (nIdx < m_stack.size() - 1))
        {
            for (vector<StackEntry>::iterator it = m_stack.begin() + nIdx; it != m_stack.end(); ++it)
            {
                if (!bRetrievedAllSymbols)
                {
                    StackTracer::Instance()->GetSymbolName(it->m_dwAddress, *it);
                }

                if (!it->m_strFile.empty() && IsValidStackEntry(it->m_strSymName, it->m_strModName))
                {
                    stackEntry = *it;
                    break;
                }
            }
        }

        m_pStackEntry = new(nothrow) StackEntry(stackEntry);
    }

#else
    m_pStackEntry = new(nothrow) StackEntry();

    if (m_pStackEntry != NULL)
    {
        m_pStackEntry->m_dwLineNum = 0;
        m_pStackEntry->m_strSymName = ss.str();
    }

#endif
}
#endif

bool CLAPIBase::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    SP_UNREFERENCED_PARAMETER(bTimeout);
    m_strName = CLStringUtils::GetCLAPINameString(m_type);

    // APIType
    sout << left << setw(5) << m_type;

    // APIName
    sout << left << setw(45) << m_strName;

    // start time
    sout << left << setw(21) << m_ullStart;

#ifdef AMDT_INTERNAL

    if (m_PrePMCs.size() > 0)
    {
        sout << " { ";

        for (size_t i = 0; i < m_PrePMCs.size(); ++i)
        {
            sout << setw(21) << m_PrePMCs[i];
        }

        sout << " } ";
    }

#endif
    // end time
    sout << left << setw(21) << m_ullEnd;
#ifdef AMDT_INTERNAL

    if (m_PostPMCs.size() > 0)
    {
        sout << " { ";

        for (size_t i = 0; i < m_PostPMCs.size(); ++i)
        {
            sout << setw(21) << m_PostPMCs[i];
        }

        sout << " } ";
    }

#endif

    return true;
}

bool CLEnqueueAPIBase::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    // APIType APITypeName StartTime  EndTime [ EnqueueCMDID EnqueueCMDName Queued Submitted Start End QueueID QueueHandle ContextID ContextHandle DeviceName [ KernelHandle KernelName GlobalWorkSize LocalWorkSize | DataTransferSize ] ]
    if (!IsReady())
    {
        Log(traceMESSAGE, "Entry not ready, but forced to flush\n");
    }

    CLAPIBase::WriteTimestampEntry(sout, bTimeout);

    // dump GPU data
    if (GetAPISucceeded())
    {

        CLEvent* en = m_pEvent.get();
#ifdef NON_BLOCKING_TIMEOUT

        // unmap gpu timestamp to cpu time
        // In timeout mode, timestamp is unmapped on the fly.
        if (!bTimeout)
        {
            en->Unmap();
        }

#else
        en->Unmap();
#endif
        sout << left << setw(8) << en->m_clCommandType;
        sout << left << setw(40) << CLStringUtils::GetCommandTypeString(en->m_clCommandType);

#ifdef NON_BLOCKING_TIMEOUT

        if (bTimeout)
        {
            // place holder
            sout << left << setw(21) << 0;
            sout << left << setw(21) << 0;
            sout << left << setw(21) << 0;
            sout << left << setw(21) << 0;
        }
        else
        {
            sout << left << setw(21) << en->m_ullQueued;
            sout << left << setw(21) << en->m_ullSubmitted;
            sout << left << setw(21) << en->m_ullRunning;
            sout << left << setw(21) << en->m_ullComplete;
        }

#else
        sout << left << setw(21) << en->m_ullQueued;
        sout << left << setw(21) << en->m_ullSubmitted;
        sout << left << setw(21) << en->m_ullRunning;
        sout << left << setw(21) << en->m_ullComplete;
#endif

        // command queue handle
        sout << std::dec << setw(10) << m_uiQueueID;
        sout << setw(25) << StringUtils::ToHexString(m_command_queue);
        // context handle
        sout << std::dec << setw(10) << m_uiContextID;
        sout << setw(25) << StringUtils::ToHexString(m_context);

        // device name
        sout << setw(30) << m_strDeviceName;
    }

    return true;
}

void CLEnqueueAPIBase::GetContextInfo()
{
    const CLAPI_clCreateCommandQueueBase* clCreateCmdQueueAPIObj = CLAPIInfoManager::Instance()->GetCreateCommandQueueAPIObj(m_command_queue);

    if (clCreateCmdQueueAPIObj != NULL)
    {
        const CLAPI_clCreateContextBase* obj = clCreateCmdQueueAPIObj->GetCreateContextAPIObject();

        if (obj != NULL)
        {
            m_context = obj->GetContext();
            m_uiContextID = obj->GetInstanceID();
        }
        else
        {
            Log(logWARNING, "context object missing from clCreateCmdQueueAPIObj\n");
            m_context = 0;
            m_uiContextID = 0;
        }

        m_uiQueueID = clCreateCmdQueueAPIObj->GetInstanceID();


        // device name
        if (clCreateCmdQueueAPIObj->GetDeviceType() == CL_DEVICE_TYPE_CPU)
        {
            m_strDeviceName = "CPU_Device";
        }
        else
        {
            const char* szDeviceName = clCreateCmdQueueAPIObj->GetDeviceName();
            std::string strDeviceName = std::string(szDeviceName);
            m_strDeviceName = StringUtils::Replace(strDeviceName, " ", "_");
        }
    }
    else
    {
        Log(logWARNING, "clCreateCmdQueueAPI object missing\n");
        m_context = NULL;
        m_uiQueueID = 0;
        m_uiContextID = 0;
        m_strDeviceName.clear();
    }
}

bool CLEnqueueAPIBase::IsReady()
{
    if (NULL != m_pEvent)
    {
        return m_pEvent->m_bIsReady;
    }
    else
    {
        return true;
    }
}

bool CLEnqueueDataTransfer::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    bool bRet = CLEnqueueAPIBase::WriteTimestampEntry(sout, bTimeout);

    if (!bRet)
    {
        return bRet;
    }

    if (!GetAPISucceeded())
    {
        return true;
    }

    sout << std::dec << setw(20) << GetDataSize();

    // switch back the dec mode
    sout << std::dec;

    return true;
}


bool CLEnqueueOther::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    bool bRet = CLEnqueueAPIBase::WriteTimestampEntry(sout, bTimeout);

    if (!bRet)
    {
        return bRet;
    }

    if (!GetAPISucceeded())
    {
        return true;
    }

    return true;
}


bool CLEnqueueData::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    bool bRet = CLEnqueueOther::WriteTimestampEntry(sout, bTimeout);

    if (!bRet)
    {
        return bRet;
    }

    if (!GetAPISucceeded())
    {
        return true;
    }

    sout << std::dec << setw(20) << GetDataSize();

    return true;
}
