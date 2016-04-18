//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the CLEvent object to interact with
///        CLEventHandler to retrieve the GPU timestamps.
//==============================================================================

#ifndef _CL_EVENT_MANAGER_H_
#define _CL_EVENT_MANAGER_H_

/// \defgroup CLEventManager CLEventManager
/// This module maintains CLEvent object, it interacts with CLEventHandler to retrieve GPU timestamp
///
/// \ingroup CLTraceAgent
// @{

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <CL/opencl.h>
#include "../Common/Defs.h"
#include "../Common/LocalTSingleton.h"
#include "AMDTMutex.h"
#include "CLAPIDefBase.h"
#include "TraceInfoManager.h"
#include "CLTraceAgent.h"
#include "StringUtils.h"

//------------------------------------------------------------------------------------
/// cl_event wrapper
//------------------------------------------------------------------------------------
struct CLEvent
{
    cl_event m_pEvent; ///< cl_event obj pointer
    bool m_bIsUserEvent; ///< is this cl_event obj created by user? if not, we have to release it.
    ULONGLONG m_ullQueued; ///< GPU timestamp that is mapped from CPU timestamp: command has been enqueued in the command-queue
    ULONGLONG m_ullSubmitted; ///< GPU timestamp that is mapped from CPU timestamp: enqueued command has been submitted by the host to the device associated with the command-queue
    ULONGLONG m_ullRunning; ///< GPU timestamp: device is currently executing this command
    ULONGLONG m_ullComplete; ///< GPU timestamp: the command has completed
    ULONGLONG m_ullCPUQueued; ///< CPU timestamp: got from callback func
    CLEnqueueAPIBase* m_pOwner; ///< Owner
    bool m_bIsReady; ///< Is this command finished
    cl_command_type m_clCommandType; ///< The OpenCL command type
    std::string m_clEventString; ///< The string representation of the OpenCL event

    /// Default constructor
    CLEvent()
    {
        SetClEvent(NULL);
        m_bIsUserEvent = false;
        m_bIsReady = false;
        m_ullQueued = m_ullSubmitted = m_ullRunning = m_ullComplete = m_ullCPUQueued = 0;
        m_pOwner = NULL;
    }

    /// For Queued and submitted event, ocl runtime queries CPU timestamp and maps to GPU timestamp so that all 4 timestamps are consistent
    /// This functions unmap all GPU timestamp back to CPU timestamp
    void Unmap();

    /// Helper function that used to map GPU timestamp to CPU timestamp
    /// \param ullBase   Base time in CPU time space
    /// \param ullQueued Queued time in GPU time space
    /// \param ullTime   Time to be mapped in GPU time space
    /// \return Time in CPU space
    static ULONGLONG UnmapHelper(ULONGLONG ullBase, ULONGLONG ullQueued, ULONGLONG ullTime);

    /// Copy constructor
    /// \param obj source
    CLEvent(const CLEvent& obj)
    {
        m_pEvent = obj.m_pEvent;
        m_bIsUserEvent = obj.m_bIsUserEvent;
        m_ullQueued = obj.m_ullQueued;
        m_ullSubmitted = obj.m_ullSubmitted;
        m_ullRunning = obj.m_ullRunning;
        m_ullComplete = obj.m_ullComplete;
        m_ullCPUQueued = obj.m_ullCPUQueued;
        m_pOwner = obj.m_pOwner;
        m_bIsReady = obj.m_bIsReady;
        m_clCommandType = obj.m_clCommandType;
        m_clEventString = obj.m_clEventString;
    }

    /// Assignment operator
    /// \param obj rhs
    /// \return rhs
    CLEvent& operator = (const CLEvent& obj)
    {
        if (this != &obj)
        {
            m_pEvent = obj.m_pEvent;
            m_bIsUserEvent = obj.m_bIsUserEvent;
            m_ullQueued = obj.m_ullQueued;
            m_ullSubmitted = obj.m_ullSubmitted;
            m_ullRunning = obj.m_ullRunning;
            m_ullComplete = obj.m_ullComplete;
            m_ullCPUQueued = obj.m_ullCPUQueued;
            m_pOwner = obj.m_pOwner;
            m_bIsReady = obj.m_bIsReady;
            m_clCommandType = obj.m_clCommandType;
            m_clEventString = obj.m_clEventString;
        }

        return *this;
    }

    void SetClEvent(cl_event clEvent)
    {
        m_pEvent = clEvent;

        if (NULL == m_pEvent)
        {
            m_clCommandType = 0;
        }
        else
        {
            GetRealDispatchTable()->GetEventInfo(
                clEvent,
                CL_EVENT_COMMAND_TYPE,
                sizeof(cl_command_type),
                &(m_clCommandType),
                NULL);
        }

        m_clEventString = StringUtils::ToHexString(m_pEvent);
    }
};

//------------------------------------------------------------------------------------
/// CL Event status change raw info
//------------------------------------------------------------------------------------
class CLEventRawInfo : public ITraceEntry
{
public:
    CLEventRawInfo() : ITraceEntry() {}
    std::string ToString() { return ""; }
    cl_event    m_event;          ///< cl event object
    cl_int      m_iStatus;        ///< cl event status
    ULONGLONG   m_ullTimestamp;   ///< timestamp
};


typedef std::unordered_map<cl_event, CLEventPtr> CLEventMap;
typedef CLEventMap::value_type CLEventMapPair;

//------------------------------------------------------------------------------------
/// This class keeps trace of OpenCL events. It maintains any event dependencies.
//------------------------------------------------------------------------------------
class CLEventManager : public TraceInfoManager, public TSingleton<CLEventManager>
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<CLEventManager>;
public:
    /// Destructor
    ~CLEventManager(void);

    /// Update event pointer to m_clEventMap
    /// \param event Event pointer
    /// \param isUserEvent Is the event created by user
    /// \param owner pointer to owner
    /// \return CLEvent - cl event wrapper
    CLEventPtr UpdateEvent(cl_event event, bool isUserEvent, CLEnqueueAPIBase* owner);

    /// Add cl_event
    /// \param event Event pointer
    /// \return CLEvent - cl event wrapper
    CLEventPtr AddEvent(cl_event event);

    /// Remove a cl_event
    /// \param[in] event The event removed from the manager
    void RemoveEvent(cl_event event);

    /// Get CLEvent from cl_event pointer
    /// \param event cl_event obj
    /// \return CLEvent obj
    CLEventPtr GetCLEvent(cl_event event);

    /// Add event status change raw info
    /// \param event  cl event object
    /// \param status event status
    /// \param ts     timestamp
    /// \return CLEventRawInfo object
    CLEventRawInfo* AddEventRawInfo(cl_event event, cl_int status, cl_long ts);

    /// Flush queued event raw info entries.
    void FlushTraceData(bool bForceFlush = false);

    /// Release cl_event created by CLTraceAgent
    void Release();

    /// Save gpu timestamp to file
    /// \param strFileName output file path+name
    void Debug(std::string& strFileName);

    /// Sets the m_bTimeOutMode member
    /// \param bTimeOutMode timeout mode value
    void SetTimeOutMode(bool bTimeOutMode);

private:
    // Private Constructor
    CLEventManager(void);

    /// Disable copy constructor
    /// \param obj obj
    CLEventManager(const CLEventManager& obj);

    /// Disable assignment operator
    /// \param obj obj
    /// \return lhs
    CLEventManager& operator = (const CLEventManager& obj);

private:
    CLEventMap m_clEventMap;                  ///< events map
    AMDTMutex* m_pMtx;                        ///< mutex for m_clEventMap
};

// @}

#endif //_CL_EVENT_MANAGER_H_
