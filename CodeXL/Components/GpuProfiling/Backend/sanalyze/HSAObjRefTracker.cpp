//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class tracks HSA object create, release and generates warning messages
//==============================================================================

#include <sstream>
#include <vector>
#include "HSAObjRefTracker.h"
#include "../HSAFdnCommon/HSAFunctionDefs.h"
#include "../Common/Logger.h"
#include "../Common/StringUtils.h"

//#define DEBUG_REF_TRACKER
using namespace GPULogger;

const std::string HSAObjRefTracker::ms_HSA_RUNTIME_REF = "__HSA_INIT_SHUTDOWN_PSEUDO_HANDLE";
const std::string HSAObjRefTracker::ms_HSA_STATUS_SUCCESS = "HSA_STATUS_SUCCESS";

HSAObjRefTracker::HSAObjRefTracker()
{
    m_strName = "HSARefTracker";

    m_bRequireAPIFlattening = true;

    SP_TODO("Add support for tracking memory allocations/regsitrations (the commented out bits below)");

    m_dependentAPIs.insert(HSA_API_Type_hsa_init);
    m_dependentAPIs.insert(HSA_API_Type_hsa_shut_down);
    m_dependentAPIs.insert(HSA_API_Type_hsa_queue_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_soft_queue_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_queue_destroy);
    //m_dependentAPIs.insert(HSA_API_Type_hsa_memory_register);
    //m_dependentAPIs.insert(HSA_API_Type_hsa_memory_deregister);
    //m_dependentAPIs.insert(HSA_API_Type_hsa_memory_allocate);
    //m_dependentAPIs.insert(HSA_API_Type_hsa_memory_free);
    m_dependentAPIs.insert(HSA_API_Type_hsa_signal_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_signal_destroy);
    m_dependentAPIs.insert(HSA_API_Type_hsa_executable_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_executable_destroy);
    m_dependentAPIs.insert(HSA_API_Type_hsa_ext_program_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_ext_program_destroy);
    m_dependentAPIs.insert(HSA_API_Type_hsa_ext_image_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_ext_image_destroy);
    m_dependentAPIs.insert(HSA_API_Type_hsa_ext_sampler_create);
    m_dependentAPIs.insert(HSA_API_Type_hsa_ext_sampler_destroy);

    m_resourceArgMap[HSA_API_Type_hsa_queue_create] = 7;
    m_resourceArgMap[HSA_API_Type_hsa_soft_queue_create] = 5;
    m_resourceArgMap[HSA_API_Type_hsa_queue_destroy] = 0;
    m_resourceArgMap[HSA_API_Type_hsa_signal_create] = 3;
    m_resourceArgMap[HSA_API_Type_hsa_signal_destroy] = 0;
    m_resourceArgMap[HSA_API_Type_hsa_executable_create] = 3;
    m_resourceArgMap[HSA_API_Type_hsa_executable_destroy] = 0;
    m_resourceArgMap[HSA_API_Type_hsa_ext_program_create] = 4;
    m_resourceArgMap[HSA_API_Type_hsa_ext_program_destroy] = 0;
    m_resourceArgMap[HSA_API_Type_hsa_ext_image_create] = 4;
    m_resourceArgMap[HSA_API_Type_hsa_ext_image_destroy] = 1;
    m_resourceArgMap[HSA_API_Type_hsa_ext_sampler_create] = 2;
    m_resourceArgMap[HSA_API_Type_hsa_ext_sampler_destroy] = 1;
}

HSAObjRefTracker::~HSAObjRefTracker(void)
{
    Clear();
}

void HSAObjRefTracker::Clear()
{
    for (APITraceMap::iterator it = m_objRefHistoryMap.begin() ; it != m_objRefHistoryMap.end(); it++)
    {
        if (it->second != NULL)
        {
            delete it->second;
        }
    }

    m_objRefHistoryMap.clear();
    m_msgList.clear();
    m_bEndAnalyze = false;
}

void HSAObjRefTracker::Analyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void HSAObjRefTracker::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    HSAAPIInfo* pHSAApiInfo = dynamic_cast<HSAAPIInfo*>(pAPIInfo);

    if (nullptr != pHSAApiInfo)
    {
        switch (pHSAApiInfo->m_apiID)
        {
            case HSA_API_Type_hsa_init:
            case HSA_API_Type_hsa_queue_create:
            case HSA_API_Type_hsa_soft_queue_create:
            case HSA_API_Type_hsa_signal_create:
            case HSA_API_Type_hsa_executable_create:
            case HSA_API_Type_hsa_ext_program_create:
            case HSA_API_Type_hsa_ext_image_create:
            case HSA_API_Type_hsa_ext_sampler_create:
                RecordResourceCreate(pHSAApiInfo);
                break;

            case HSA_API_Type_hsa_shut_down:
            case HSA_API_Type_hsa_queue_destroy:
            case HSA_API_Type_hsa_signal_destroy:
            case HSA_API_Type_hsa_executable_destroy:
            case HSA_API_Type_hsa_ext_program_destroy:
            case HSA_API_Type_hsa_ext_image_destroy:
            case HSA_API_Type_hsa_ext_sampler_destroy:
                RecordResourceFree(pHSAApiInfo);
                break;

            default:
                break;
        }
    }
}

std::string HSAObjRefTracker::GetResourceHandle(HSAAPIInfo* pAPIInfo)
{
    std::string retVal;

    SpAssertRet(nullptr != pAPIInfo) retVal;

    if (m_resourceArgMap.find(pAPIInfo->m_apiID) != m_resourceArgMap.end())
    {
        std::vector<std::string> apiArgs;
        StringUtils::Split(apiArgs, pAPIInfo->m_ArgList, std::string(";"));

        unsigned int argIndex = m_resourceArgMap[pAPIInfo->m_apiID];

        if (argIndex < apiArgs.size())
        {
            retVal = StringUtils::StripBrackets(apiArgs[argIndex]);
        }
    }
    else if (pAPIInfo->m_apiID == HSA_API_Type_hsa_init || pAPIInfo->m_apiID == HSA_API_Type_hsa_shut_down)
    {
        retVal = ms_HSA_RUNTIME_REF;
    }

    return retVal;
}

void HSAObjRefTracker::RecordResourceFree(HSAAPIInfo* pAPIInfo, bool shouldCheckRetVal)
{
    if (nullptr == pAPIInfo || (shouldCheckRetVal && pAPIInfo->m_strRet != ms_HSA_STATUS_SUCCESS))
    {
        // passed in pointer was null, or API was not successful -- do not record
        return;
    }

    APIObjectHistory his;
    his.m_action = API_OBJECT_ACTION_Release;
    his.m_pAPIInfoObj = pAPIInfo;

    std::string strHandle = GetResourceHandle(pAPIInfo);
    APITraceMap::iterator it = m_objRefHistoryMap.find(strHandle);

    if (it != m_objRefHistoryMap.end())
    {
        APIObjHistoryList* list = it->second;
        int iRef = list->back().m_iCurrentRef;
        his.m_iCurrentRef = iRef - 1;

        if (his.m_iCurrentRef < 0)
        {
            Log(logERROR, "HSAObjRefTracker::RecordResourceFree: HSA object ref < 0\n", strHandle.c_str());
        }

        list->push_back(his);
    }
    else
    {
        Log(logERROR, "HSAObjRefTracker::RecordResourceFree: HSA object handle invalid - %p\n", strHandle.c_str());
    }
}

void HSAObjRefTracker::RecordResourceCreate(HSAAPIInfo* pAPIInfo, bool shouldCheckRetVal)
{
    if (nullptr == pAPIInfo || (shouldCheckRetVal && pAPIInfo->m_strRet != ms_HSA_STATUS_SUCCESS))
    {
        // passed in pointer was null, or API was not successful -- do not record
        return;
    }

    APIObjectHistory his;
    his.m_action = API_OBJECT_ACTION_Create;
    his.m_pAPIInfoObj = pAPIInfo;
    his.m_iCurrentRef = 1;

    std::string strHandle = GetResourceHandle(pAPIInfo);
    APITraceMap::iterator it = m_objRefHistoryMap.find(strHandle);

    if (it != m_objRefHistoryMap.end())
    {
        APIObjHistoryList* list = it->second;

        if (list->back().m_iCurrentRef != 0)
        {
            Log(logERROR, "HSAObjRefTracker::RecordResourceCreate: HSA object handle conflict - %p\n", strHandle.c_str());
        }
        else
        {
            // pointer reuse
            // just append to it
            list->push_back(his);
        }
    }
    else
    {
        APIObjHistoryList* list = new APIObjHistoryList();
        list->push_back(his);
        m_objRefHistoryMap.insert(std::pair<std::string, APIObjHistoryList*>(strHandle, list));
    }
}

std::string HSAObjRefTracker::APIObjHistoryListToString(APIObjHistoryList* list)
{
    std::stringstream ss;

    for (APIObjHistoryList::iterator it = list->begin(); it != list->end(); it++)
    {
        HSAAPIInfo* pInfo = dynamic_cast<HSAAPIInfo*>(it->m_pAPIInfoObj);

        if (nullptr != pInfo)
        {
            ss << "[ ThreadID: " << pInfo->m_tid << ", SeqID: " << pInfo->m_uiSeqID << " ]" << pInfo->m_strName << " - Ref = " << it->m_iCurrentRef << std::endl;
        }
    }

    return ss.str();
}

void HSAObjRefTracker::EndAnalyze()
{
    if (m_bEndAnalyze)
    {
        return;
    }

    for (APITraceMap::iterator it = m_objRefHistoryMap.begin() ; it != m_objRefHistoryMap.end(); it++)
    {
#ifdef DEBUG_REF_TRACKER
        m_msgList.push_back(APIObjHistoryListToString(it->second));
#endif

        if (it->second->back().m_iCurrentRef != 0)
        {
            HSAAPIInfo* pInfo = dynamic_cast<HSAAPIInfo*>(it->second->front().m_pAPIInfoObj);

            if (nullptr != pInfo)
            {
                std::stringstream ss;
                ss << "Memory leak detected [Ref = " << it->second->back().m_iCurrentRef;

                // don't display a handle if the leak is due to mismatched hsa_init/hsa_shut_down calls
                if (ms_HSA_RUNTIME_REF != it->first)
                {
                    ss << ", Object = " << it->first;
                }

                ss << "]: ";
                ss << "Object created by " << pInfo->m_strName << std::endl;
                APIAnalyzerMessage msg;
                msg.type = MSGTYPE_Warning;
                msg.uiSeqID = pInfo->m_uiSeqID;
                msg.uiDisplaySeqID = pInfo->m_uiDisplaySeqID;
                msg.bHasDisplayableSeqId = pInfo->m_bHasDisplayableSeqId;
                msg.uiTID = pInfo->m_tid;
                msg.strMsg = ss.str();
                m_msgList.push_back(msg);
            }
        }
    }

    m_bEndAnalyze = true;
}
