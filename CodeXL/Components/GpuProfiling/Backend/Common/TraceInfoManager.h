//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages ITraceEntry, provides thread-safe API to add and
///        flush trace entries. It's the base class for APIInfoManager, PerfMarkerInfoManager
///        and EventInfoManager (and others)
//==============================================================================

#ifndef _TRACE_INFO_MANAGER_H_
#define _TRACE_INFO_MANAGER_H_

/// \defgroup TraceInfoManager TraceInfoManager
/// This module manages ITraceEntry
///
/// \ingroup CLTraceAgent
// @{

#include <string>
#include <list>
#include <map>
#include <set>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "Timer.h"
#include "Defs.h"
#include "OSUtils.h"
#include "AMDTMutex.h"
#include "LocaleSetting.h"
#include "StackTracer.h"

/// Timer function definition
typedef void (*TimerFunc)(void* param);


//------------------------------------------------------------------------------------
/// Trace entry base class
//------------------------------------------------------------------------------------
class ITraceEntry
{
public:
    /// Constructor
    ITraceEntry() : m_tid(0) {}

    /// Virtual destructor
    virtual ~ITraceEntry()
    {
    }

    /// return string representation of the marker
    virtual std::string ToString() = 0;
public:
    static std::string s_strParamSeparator; /// string separator

public:
    osThreadId m_tid;            ///< thread id

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    ITraceEntry(const ITraceEntry& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    ITraceEntry& operator=(const ITraceEntry& obj);
};

typedef std::map<osThreadId, std::list<ITraceEntry*> > TraceInfoMap;
typedef std::pair<osThreadId, std::list<ITraceEntry*> > TraceInfoMapPair;

//------------------------------------------------------------------------------------
/// TraceInfoManager manages captured apis, events and so on
//------------------------------------------------------------------------------------
class TraceInfoManager
{
public:

    /// Constructor
    TraceInfoManager();

    /// Destructor
    virtual ~TraceInfoManager();

    /// Add APIInfo to the list
    /// \param en APIInfo entry
    virtual void AddTraceInfoEntry(ITraceEntry* en);

    /// Release all entries in m_TraceInfoMap
    virtual void Release();

    /// Start timer-based output mode
    /// \return true is succeed
    bool StartTimer(TimerFunc timerFunc);

    /// Stop timer, call Join to make main thread wait till all I/O are finished
    void StopTimer();

    /// Recreates the timer thread after a call to StopTimer
    bool ResumeTimer();

    /// Is it in time out mode
    /// \return true if it is in timer mode
    bool IsTimeOutMode() const
    {
        return m_bTimeOutMode;
    }

    /// Save trace data to tmp file
    /// \param bForceFlush Force to write all data out no matter it's ready or not - used in Detach() only
    virtual void FlushTraceData(bool bForceFlush = false) = 0;

    /// Indicating when timer thread should stop
    /// \return true if app is still running
    bool IsRunning() const { return m_bIsRunning; }

    /// Timer interval
    /// \return timer interval
    unsigned int GetInterval() const  { return m_uiInterval; }

    /// Set timer interval
    /// \param val interval
    void SetInterval(unsigned int val) { m_uiInterval = val; }

    /// Try swap active map and non-active map
    /// Active map is the one api entry being stored into
    /// Non-active map is the one to be flushed to disk
    /// If non-active map has remaining entries, don't swap maps
    /// else, swap atomically
    void TrySwapBuffer();

    /// Stop tracing
    void StopTracing() { m_bStopped = true; }

    /// Resume tracing
    void ResumeTracing() { m_bStopped = false; }

    /// Check whether we are currently tracing
    /// \return true if we are tracing, false otehrwise
    bool IsTracing() const { return m_bStopped == false; }

protected:
    /// Disable copy constructor
    /// \param obj obj
    TraceInfoManager(const TraceInfoManager& obj);

    /// Disable assignment operator
    /// \param obj obj
    /// \return lhs
    TraceInfoManager& operator = (const TraceInfoManager& obj);

protected:
    TraceInfoMap m_TraceInfoMap[2];  ///< stl map that maintains all captured apis
    int m_iActiveMap;                ///< active map index
    AMDTMutex m_mtx;                 ///< mutex used to lock APIInfoMap
    AMDTMutex m_mtxFlush;            ///< mutex used to lock flush()
    AMDTMutex m_mtxTracemap;         ///< mutex used to lock AddTraceInfoEntry
    bool m_bTimeOutMode;             ///< Is time out mode
    bool m_bIsRunning;               ///< Is app running
    unsigned int m_uiInterval;       ///< Timer interval
    char m_cListSeparator;           ///< the list separator for the current locale
    THREADHANDLE m_tidTimer;         ///< ThreadID of timer thread
    bool m_bStopped;                 ///< A flag indicating whether trace is stopped or not
    TimerFunc m_timerFunc;           ///< The timer function used for flushing data
};

// @}

#endif //_TRACE_INFO_MANAGER_H_
