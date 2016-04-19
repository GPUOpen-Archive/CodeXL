//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfEventInternal.cpp
///
//==================================================================================

// Standard headers
#include <string.h>
#include <stdlib.h>

// Project headers
#include "PerfEventInternal.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

//
//  class PerfEventInternal
//

PerfEventInternal::PerfEventInternal(const PerfEventInternal& evt)
{
    m_event           = evt.m_event;
    m_groupLeader     = evt.m_groupLeader;

    m_samplebyFreq    = evt.m_samplebyFreq;
    m_samplingValue   = evt.m_samplingValue;
    m_sampleAttr      = evt.m_sampleAttr;

    m_readFormat      = evt.m_readFormat;

    m_wakeupByEvents  = evt.m_wakeupByEvents;
    m_waterMark       = evt.m_waterMark;
}

PerfEventInternal& PerfEventInternal::operator=(const PerfEventInternal& evt)
{
    m_event           = evt.m_event;
    m_groupLeader     = evt.m_groupLeader;

    m_samplebyFreq    = evt.m_samplebyFreq;
    m_samplingValue   = evt.m_samplingValue;
    m_sampleAttr      = evt.m_sampleAttr;

    m_readFormat      = evt.m_readFormat;

    m_wakeupByEvents  = evt.m_wakeupByEvents;
    m_waterMark       = evt.m_waterMark;

    return *this;
}

void PerfEventInternal::print()
{
    OS_OUTPUT_DEBUG_LOG(L"PerfEventInternal Details :-", OS_DEBUG_LOG_INFO);
    m_event.print();

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Sampling %ls : %ld", (m_samplebyFreq ? L"Frequency" : L"Period"), m_samplingValue);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Sample attributes : %lx", m_sampleAttr);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"read  format : %lx", m_readFormat);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Wakeup by %ls", (m_wakeupByEvents ? L"Events" : L"Bytes"));
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Wakeup Watermark : %d\n", m_waterMark);
}
