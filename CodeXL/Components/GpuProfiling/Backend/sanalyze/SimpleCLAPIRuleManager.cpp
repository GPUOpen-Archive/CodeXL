//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class manages all simple API rules. We don't want to create a
///         separate CLAPIAnalyzer class for a rule -- simple API rules can be
///         defined by a callback function
//==============================================================================

#include "SimpleCLAPIRuleManager.h"
#include "../Common/Logger.h"
#include "../Common/StringUtils.h"

using namespace std;
using namespace GPULogger;

SimpleCLAPIRuleManager::SimpleCLAPIRuleManager(CLAPIAnalyzerManager* p) : CLAPIAnalyzer(p)
{
    m_strName = "CLAPIRules";
    m_pCmdQInfoList = new CmdQueueInfoList();
    SP_TODO("Add dependent APIs")
}

SimpleCLAPIRuleManager::~SimpleCLAPIRuleManager(void)
{
    delete m_pCmdQInfoList;
    m_pCmdQInfoList = NULL;
}

void SimpleCLAPIRuleManager::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void SimpleCLAPIRuleManager::Analyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLApiInfo)
    {
        if (pCLApiInfo->m_uiAPIID == CL_FUNC_TYPE_clCreateCommandQueue || pCLApiInfo->m_uiAPIID == CL_FUNC_TYPE_clCreateCommandQueueWithProperties)
        {
            GetCmdQueueInfoList()->AddToCmdQInfoList(pCLApiInfo);
        }
        else if (pCLApiInfo->m_uiAPIID == CL_FUNC_TYPE_clReleaseCommandQueue)
        {
            GetCmdQueueInfoList()->RemoveFromCmdQInfoList(pCLApiInfo);
        }
        else if (pCLApiInfo->m_uiAPIID == CL_FUNC_TYPE_clRetainCommandQueue)
        {
            GetCmdQueueInfoList()->SetRefCntInCmdQInfoList(pCLApiInfo);
        }

        for (vector<CLAPIRule>::iterator it = m_rules.begin(); it != m_rules.end(); it++)
        {
            if (it->enabled && pCLApiInfo->m_uiAPIID == (unsigned int)it->type)
            {

                if (it->callback != NULL)
                {
                    string strMsg;
                    APIAnalyzerMessageType type;

                    if (it->callback(pCLApiInfo, strMsg, type))
                    {
                        APIAnalyzerMessage msg;
                        msg.type = type;
                        msg.uiSeqID = pCLApiInfo->m_uiSeqID;
                        msg.uiDisplaySeqID = pCLApiInfo->m_uiDisplaySeqID;
                        msg.bHasDisplayableSeqId = pCLApiInfo->m_bHasDisplayableSeqId;
                        msg.uiTID = pCLApiInfo->m_tid;
                        msg.strMsg = strMsg;
                        m_msgList.push_back(msg);
                    }
                }
            }
        }
    }
}

void SimpleCLAPIRuleManager::SetEnable(const AnalyzeOps& op)
{
    bool flag = false;

    for (vector<CLAPIRule>::iterator it = m_rules.begin(); it != m_rules.end(); it++)
    {
        AnalyzerMap::const_iterator ait = op.analyzerMap.find(it->name);

        if (ait != op.analyzerMap.end())
        {
            it->enabled = ait->second;

            if (ait->second)
            {
                // as long as one rule is enabled, SimpleCLAPIRuleManager is enabled
                flag = true;
            }
        }
    }

    if (!flag)
    {
        // no sub-rules are enabled, disable SimpleCLAPIRuleManager
        m_bEnabled = false;
    }
}

//------------------------------------------------------------------------------------
/// Simple CL API Rule Manager class
//------------------------------------------------------------------------------------

CmdQueueInfoList::CmdQueueInfoList()
{
}

CmdQueueInfoList::~CmdQueueInfoList()
{
    CmdQueueMap::iterator it = m_CmdQueueMap.begin();

    while (it != m_CmdQueueMap.end())
    {
        delete it->second;
        it->second = NULL;
        it++;
    }

    m_CmdQueueMap.clear();
}

void CmdQueueInfoList::AddToCmdQInfoList(CLAPIInfo* pAPIInfo)
{
    std::vector<std::string> argList;
    StringUtils::Split(argList, pAPIInfo->m_ArgList, std::string(";"), true, true);

    if (argList.size() < 4)
    {
        return;
    }

    CmdQueueMap::iterator it = m_CmdQueueMap.find(pAPIInfo->m_strRet);

    CreateCmdQueueInfo* ptr = new CreateCmdQueueInfo();
    ptr->strContex = argList[0];
    ptr->strDevice = argList[1];
    ptr->strProp = argList[2];
    ptr->strRetVal = argList[3];
    ptr->ref_count = 0;

    if (it != m_CmdQueueMap.end())
    {
        Log(logWARNING, "ATP file has multiple command queues with same handle\n");
        it->second = ptr;
    }
    else
    {
        m_CmdQueueMap.insert(CmdQueueMapPair(pAPIInfo->m_strRet, ptr));
    }
}

void CmdQueueInfoList::RemoveFromCmdQInfoList(CLAPIInfo* pAPIInfo)
{
    std::vector<std::string> argList;
    StringUtils::Split(argList, pAPIInfo->m_ArgList, std::string(";"), true, true);
    CmdQueueMap::iterator it = m_CmdQueueMap.find(argList[0]);

    if (it == m_CmdQueueMap.end())
    {
        Log(logWARNING, "ATP file has extra release call for same command queue handle\n");
    }
    else if (it->second->ref_count)
    {
        it->second->ref_count -= 1;
    }
    else
    {
        delete it->second;
        it->second = NULL;
        m_CmdQueueMap.erase(argList[0]);
    }
}

void CmdQueueInfoList::SetRefCntInCmdQInfoList(CLAPIInfo* pAPIInfo)
{
    std::vector<std::string> argList;
    StringUtils::Split(argList, pAPIInfo->m_ArgList, std::string(";"), true, true);
    CmdQueueMap::iterator it = m_CmdQueueMap.find(argList[0]);

    if (it != m_CmdQueueMap.end())
    {
        it->second->ref_count ++;
    }
    else
    {
        Log(logWARNING, "ATP file has retain call for non-existing command queue\n");
    }
}

CreateCmdQueueInfo* CmdQueueInfoList::FindInCmdQInfoList(std::string cl_cmd_queue)
{
    CreateCmdQueueInfo* cmdQInfo = NULL;
    CmdQueueMap::iterator it = m_CmdQueueMap.find(cl_cmd_queue);

    if (it != m_CmdQueueMap.end())
    {
        cmdQInfo = it->second;
    }

    return cmdQInfo;
}
