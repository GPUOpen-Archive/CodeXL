//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the CLEvent object to interact with
///        CLEventHandler to retrieve the GPU timestamps.
//==============================================================================

#include <fstream>
#include <utility>

#include <AMDTOSWrappers/Include/osProcess.h>

#include "CLEventManager.h"
#include "CLAPIInfoManager.h"
#include "../Common/Logger.h"
#include "../Common/GlobalSettings.h"
#include "../Common/FileUtils.h"
#include "../Common/StringUtils.h"
#include "CLTraceAgent.h"

using namespace std;
using namespace GPULogger;

ULONGLONG CLEvent::UnmapHelper(ULONGLONG ullBase, ULONGLONG ullQueued, ULONGLONG ullTime)
{
    SpAssert(ullTime >= ullQueued);
    return ullBase + (ullTime - ullQueued);
}

void CLEvent::Unmap()
{
    //SpAssert( m_pOwner != NULL );
    //if( m_pOwner == NULL )
    //{
    //   Log( logERROR, "CLEvent::Unmap() CLEvent::m_pOwner = NULL\n" );
    //   return;
    //}

    m_ullSubmitted = UnmapHelper(m_ullCPUQueued, m_ullQueued, m_ullSubmitted);
    m_ullRunning = UnmapHelper(m_ullCPUQueued, m_ullQueued, m_ullRunning);
    m_ullComplete = UnmapHelper(m_ullCPUQueued, m_ullQueued, m_ullComplete);
    m_ullQueued = m_ullCPUQueued;
}

CLEventManager::CLEventManager(void)
{
    m_pMtx = new(nothrow) AMDTMutex("CLEventManagerMutex");
    SpAssert(m_pMtx != NULL);
}

CLEventManager::~CLEventManager(void)
{
    SAFE_DELETE(m_pMtx);
}

void CLEventManager::FlushTraceData(bool bForceFlush)
{
    SP_UNREFERENCED_PARAMETER(bForceFlush);
    m_mtxFlush.Lock();
    osProcessId pid = osGetCurrentProcessId();
    TraceInfoMap& nonActiveMap = m_TraceInfoMap[ 1 - m_iActiveMap ];

    stringstream ss;
    string path;

    if (GlobalSettings::GetInstance()->m_params.m_strOutputFile.empty())
    {
        path = FileUtils::GetDefaultOutputPath();
    }
    else
    {
        path = FileUtils::GetTempFragFilePath();
    }

    // File name: pid.tstamp
    ss << path << pid << TMP_GPU_TIME_STAMP_RAW_EXT;
    string strFile = ss.str();
    ofstream fout(strFile.c_str(), fstream::out | fstream::app);

    for (TraceInfoMap::iterator mapIt = nonActiveMap.begin(); mapIt != nonActiveMap.end(); mapIt++)
    {
        while (!mapIt->second.empty())
        {
            CLEventRawInfo* pInfo = dynamic_cast<CLEventRawInfo*>(mapIt->second.front());
            fout << StringUtils::ToHexString(pInfo->m_event) << " " << pInfo->m_iStatus << " " << pInfo->m_ullTimestamp << endl;

            mapIt->second.pop_front();
            delete pInfo;
        }
    }

    fout.close();
    m_mtxFlush.Unlock();
}

CLEventRawInfo* CLEventManager::AddEventRawInfo(cl_event event, cl_int status, cl_long ts)
{
    if (CLAPIInfoManager::Instance()->IsCapReached())
    {
        // Do not allocate more memory if max number of APIs are traced.
        return NULL;
    }

    CLEventRawInfo* pInfo = new(nothrow) CLEventRawInfo();
    SpAssertRet(pInfo != NULL) NULL;

    pInfo->m_event = event;
    pInfo->m_iStatus = status;
    pInfo->m_ullTimestamp = ts;
    TraceInfoManager::AddTraceInfoEntry(pInfo);

    return pInfo;
}

CLEventPtr CLEventManager::AddEvent(cl_event event)
{
    AMDTScopeLock lock(m_pMtx);

    if (event != NULL)
    {
        CLEventMap::iterator it = m_clEventMap.find(event);

        if (it != m_clEventMap.end())
        {
            cl_uint refC;
            GetRealDispatchTable()->GetEventInfo(event, CL_EVENT_REFERENCE_COUNT, sizeof(cl_uint), &refC, NULL);
            Log(logWARNING, "Event(0x%p) is already in EventManager. Ref = %d. IsUserEvent = %s. \n", event, refC, (it->second->m_bIsUserEvent ? "True" : "False"));
            return it->second;
        }
        else
        {
            CLEventPtr en(new(std::nothrow) CLEvent);
            en->SetClEvent(event);
            m_clEventMap.insert(CLEventMapPair(event, en));

            // User events need to be retained.
            // Non user events are over incremented here, they are released in UpdateEvent
            GetRealDispatchTable()->RetainEvent(event);
        }

        return m_clEventMap[event];
    }
    else
    {
        Log(logERROR, "NULL event obj\n");
        return NULL;
    }
}

void CLEventManager::RemoveEvent(cl_event event)
{
    AMDTScopeLock lock(m_pMtx);

    CLEventMap::iterator eventIter = m_clEventMap.find(event);

    if (m_clEventMap.end() != eventIter)
    {
        GetRealDispatchTable()->ReleaseEvent(event);
        m_clEventMap.erase(eventIter);
    }
}

CLEventPtr CLEventManager::UpdateEvent(cl_event event, bool isUserEvent, CLEnqueueAPIBase* owner)
{
    AMDTScopeLock lock(m_pMtx);

    CLEventPtr clEvent(NULL);

    if (event != NULL)
    {
        CLEventMap::iterator it = m_clEventMap.find(event);

        if (it != m_clEventMap.end())
        {
            // Update CLEvent
            clEvent = it->second;
            clEvent->m_pOwner = owner;
            clEvent->m_bIsUserEvent = isUserEvent;

            if (false == isUserEvent)
            {
                // Non user events are over incremented in AddEVent, release it here
                GetRealDispatchTable()->ReleaseEvent(event);
            }

            // If the event is ready, it's a blocking event, it needs to be removed from the event manager
            if (clEvent->m_bIsReady)
            {
                RemoveEvent(event);
            }
        }
        else // event hasn't been added using AddEvent
        {
            // Event should be added to event managed in cl callback
            Log(logWARNING, "Event(0x%p) not managed by EventManager.\n Added to EventManager now\n", event);
            clEvent = std::move(CLEventPtr(new(std::nothrow) CLEvent));
            clEvent->m_bIsUserEvent = isUserEvent;
            clEvent->SetClEvent(event);
            clEvent->m_pOwner = owner;

            if (isUserEvent)
            {
                // add ref counter by one so that user can't release it
                GetRealDispatchTable()->RetainEvent(event);
            }

            // If the event is ready, it's a blocking event, no need to track it in the event manager
            if (false == clEvent->m_bIsReady)
            {
                m_clEventMap.insert(CLEventMapPair(event, clEvent));
            }
        }
    }
    else
    {
        Log(logERROR, "NULL event obj\n");
    }

    return clEvent;
}

CLEventPtr CLEventManager::GetCLEvent(cl_event event)
{
    AMDTScopeLock lock(m_pMtx);

    CLEventMap::iterator it = m_clEventMap.find(event);

    if (it != m_clEventMap.end())
    {
        return it->second;
    }
    else
    {
        CLEventPtr nullEvent;
        return std::move(nullEvent);
    }
}

void CLEventManager::Release()
{
    for (CLEventMap::iterator it = m_clEventMap.begin(); it != m_clEventMap.end(); it++)
    {
        if ((NULL != it->second->m_pEvent) && (NULL != it->second->m_pOwner))
        {
            // If pOwner == NULL, we never retain it or this event is not created by us
            cl_int res = GetRealDispatchTable()->ReleaseEvent(it->second->m_pEvent);

            if (res != CL_SUCCESS)
            {
                Log(logWARNING, "CLEventManager::Release() failed\n");
            }
        }
    }

    TraceInfoManager::Release();
}

void CLEventManager::Debug(std::string& strFileName)
{
    ofstream fout(strFileName.c_str());

    for (CLEventMap::iterator it = m_clEventMap.begin(); it != m_clEventMap.end(); it++)
    {
        if (!it->second->m_ullRunning == 0)
        {
            //fout << it->second.
            cl_command_type cmd_type;
            GetRealDispatchTable()->GetEventInfo(it->second->m_pEvent, CL_EVENT_COMMAND_TYPE, sizeof(cl_command_type), &cmd_type, NULL);
            fout << cmd_type << "   ";
            fout << CLStringUtils::GetCommandTypeString(cmd_type) << "   ";
            fout << it->second->m_ullQueued << "   ";
            fout << it->second->m_ullSubmitted << "   ";
            fout << it->second->m_ullRunning << "   ";
            fout << it->second->m_ullComplete << "   " << endl;
        }
        else
        {
            Log(logWARNING, "Event(0x%p) callback never triggered. API Type: %s\n",
                it->second->m_pEvent,
                CLStringUtils::GetCLAPINameString(it->second->m_pOwner->m_type).c_str());
        }
    }

    fout.close();
}

void CLEventManager::SetTimeOutMode(bool bTimeOutMode)
{
    m_bTimeOutMode = bTimeOutMode;
}
