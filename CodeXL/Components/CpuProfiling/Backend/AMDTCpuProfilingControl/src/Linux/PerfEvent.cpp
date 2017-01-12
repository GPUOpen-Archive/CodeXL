//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfEvent.cpp
///
//==================================================================================

// Standard headers
#include <string.h>
#include <stdlib.h>

// Project headers
#include "PerfEvent.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

//
//  class PerfEvent
//

PerfEvent::PerfEvent()
{
    uint32_t eventid = UINT_MAX;   // dummy event id
    uint32_t unitmask = UINT_MAX;  // dummy unitmask

    init(PERF_PROFILE_TYPE_HW_CPU_CYCLES,
         eventid,
         unitmask,
         (uint32_t)(PERF_PMU_FLAG_EXCLUDE_IDLE | PERF_PMU_FLAG_EXCLUDE_HYPERVISOR),
         (uint32_t)(PERF_TASK_FLAG_INHERIT | PERF_TASK_FLAG_TRACE_TASK));
}

PerfEvent::PerfEvent(uint32_t type, uint32_t pmuFlags, uint32_t taskFlags)
{
    uint32_t eventid = UINT_MAX;   // dummy event id
    uint32_t unitmask = UINT_MAX;  // dummy unitmask

    init(type,
         eventid,
         unitmask,
         pmuFlags,
         taskFlags);
}

PerfEvent::PerfEvent(uint32_t eventid, uint32_t unitmask, uint32_t pmuFlags, uint32_t taskFlags)
{
    init(PERF_PROFILE_TYPE_RAW,
         eventid,
         unitmask,
         pmuFlags,
         taskFlags);
}

void PerfEvent::init(uint32_t type, uint32_t eventid, uint32_t unitmask, uint32_t pmuFlags, uint32_t taskFlags)
{
    m_eventId  = eventid;
    m_unitMask = unitmask;

    // for IBS, m_config will contain the fetch/op control msr values
    m_config = 0;

    // set the m_type and m_config fields
    m_type     = type;
    m_skidFactor = PERF_SAMPLE_ARBITRARY_SKID;

    if (PERF_PROFILE_TYPE_RAW == m_type)
    {
        m_config = ((uint64_t)(((unitmask & 0xff) << 8) | (eventid & 0xff))
                    | ((uint64_t)(eventid & 0xf00) << 24));
    }
    else if (PERF_PROFILE_TYPE_IBS_FETCH == m_type)
    {
        // IBS fetch counter is randomized, set IbsRandEn to 1
        if (pmuFlags & PerfEvent::PERF_PMU_FLAG_IBS_RAND_INST_FETCH_TAGGING)
        {
            m_config |= ((uint64_t)(1ULL << 57));  // IbsRandEn
        }

        m_skidFactor = PERF_SAMPLE_CONSTANT_SKID;
    }
    else if (PERF_PROFILE_TYPE_IBS_OP == m_type)
    {
        // dispatch count, set IbsOpCntCtl bit to 1
        if (pmuFlags & PerfEvent::PERF_PMU_FLAG_IBS_COUNT_DISPATCHED_OPS)
        {
            m_config |= ((uint64_t)(1ULL << 19));
        }

        m_skidFactor = PERF_SAMPLE_CONSTANT_SKID;
    }

    m_pEventName   = NULL;
    m_pEventAlias  = NULL;
    m_pmuFlags     = pmuFlags;
    m_taskFlags    = taskFlags;
    m_eventCounter = UINT_MAX;
    m_initialized  = true;
}

bool PerfEvent::initialize(uint32_t type, uint32_t pmuFlags, uint32_t taskFlags)
{
    uint32_t eventid  = UINT_MAX;   // dummy event id
    uint32_t unitmask = UINT_MAX;  // dummy unitmask

    init(type, eventid, unitmask, pmuFlags, taskFlags);

    m_initialized = validate();
    return m_initialized;
}

bool PerfEvent::initialize(uint32_t eventid, uint32_t unitmask, uint32_t pmuFlags, uint32_t taskFlags)
{
    init(PERF_PROFILE_TYPE_RAW, eventid, unitmask, pmuFlags, taskFlags);

    m_initialized = validate();
    return m_initialized;
}


// PerfEvent::validate
//
// check for the validity of all the fields..
//   - event-id, unitmask, profileflags, taslflags
//
// TODO: return proper errorcode
//
bool PerfEvent::validate()
{
    // Validate event & unitmask
    if (!validateEvent())
    {
        return false;
    }

    // validate PMU flags
    if (! validatePmuFlags())
    {
        return false;
    }

    // validate task flags
    if (! validateTaskFlags())
    {
        return false;
    }

    return true;
}


// copy ctor
PerfEvent::PerfEvent(const PerfEvent& evt)
{
    copy(evt);
}


// assignment operator
PerfEvent& PerfEvent::operator= (const PerfEvent& evt)
{
    if (this != &evt)
    {
        if (NULL != m_pEventName)
        {
            free(m_pEventName);
            m_pEventName = NULL;
        }

        if (NULL != m_pEventAlias)
        {
            free(m_pEventAlias);
            m_pEventAlias = NULL;
        }

        copy(evt);
    }

    return *this;
}


void PerfEvent::copy(const PerfEvent& evt)
{
    m_type     = evt.m_type;
    m_config   = evt.m_config;
    m_eventId  = evt.m_eventId;
    m_unitMask = evt.m_unitMask;

    m_pEventName = (NULL == evt.m_pEventName) ? NULL : strdup(evt.m_pEventName);

    m_pEventAlias = (NULL == evt.m_pEventAlias) ? NULL : strdup(evt.m_pEventAlias);

    m_pmuFlags      = evt.m_pmuFlags;
    m_taskFlags     = evt.m_taskFlags;
    m_eventCounter  = evt.m_eventCounter;
    m_initialized   = evt.m_initialized;
}

bool PerfEvent::validateEvent() const
{
    // TODO
    // Validate the event - use CEventsFile::findEventByValue
    // Validate the unitmask - use CEventsFile::UnitMaskList iter
    return true;
}


// dtor
PerfEvent::~PerfEvent()
{
    clear();
}

void PerfEvent::clear()
{
    m_initialized = false;
    // Re-set all the fields..

    if (m_pEventName)
    {
        free(m_pEventName);
        m_pEventName = NULL;
    }

    if (m_pEventAlias)
    {
        free(m_pEventAlias);
        m_pEventAlias = NULL;
    }
}

void PerfEvent::print()
{
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"type          : %d", m_type);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"config        : 0x%lx", m_config);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"eventId       : 0x%x", m_eventId);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"unitmask      : 0x%x", m_unitMask);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"profile flags : 0x%x", m_pmuFlags);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"task flags    : 0x%x", m_taskFlags);
}


PerfEventCountData::PerfEventCountData()
{
    m_type      = UINT_MAX;
    m_config    = UINT_MAX;
    m_pid       = -1;
    m_cpu       = -1;
    m_fd        = -1;
    m_sampleId  = ULONG_MAX;
    m_hasValidCountData = false;
    m_nbrValues = PERF_MAX_NBR_VALUES;
}

void PerfEventCountData::print()
{
    OS_OUTPUT_DEBUG_LOG(L"Counting Event Values", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_DEBUG_LOG(L"---------------------", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Event type : %d", m_type);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Config : 0x%lx", m_config);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"PID : %d", m_pid);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"CPU : %d", m_cpu);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"FD : %d", m_fd);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Sample ID : %ld", m_sampleId);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Values %ld", m_values[0]);
}
