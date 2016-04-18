//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfEventInternal.h
///
//==================================================================================

#ifndef _PERFEVENTINTERNAL_H_
#define _PERFEVENTINTERNAL_H_

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <wchar.h>

// C++ STL headers
#include <list>

// project headers
#include "PerfEvent.h"

// class PerfEventInternal
//
// Internal class for Event Configuration
//
class PerfEventInternal
{
public:
    // To create a counter Event
    // following fields does NoT matter
    //  - m_samplebyFreq
    //  - m_samplingValue
    //  - m_wakeupByEvents
    //  - m_waterMark
    PerfEventInternal(const PerfEvent& evt, bool gleader = true) : m_event(evt), m_groupLeader(gleader)
    {
        m_samplebyFreq    = false;
        m_samplingValue   = 0;
        m_wakeupByEvents  = false;
        m_waterMark       = 0;

        m_sampleAttr = 0;
        m_readFormat = 0;
    }

    // Ctor to create a Sampling Event
    // following fields does NoT matter
    //  - m_groupLeader;  groupleader has no meaning for sampling events
    PerfEventInternal(const PerfEvent& evt, uint64_t sampleValue, bool sampleByFreq = false) : m_event(evt),
        m_groupLeader(false),
        m_samplebyFreq(sampleByFreq),
        m_samplingValue(sampleValue)
    {
        m_wakeupByEvents = true;
        m_waterMark = 1;

        m_sampleAttr = 0;
        m_readFormat = 0;
    }

    PerfEventInternal(const PerfEventInternal& evt);
    PerfEventInternal& operator= (const PerfEventInternal& evt);

    void setGroupLeader(bool gleader = true)
    {
        m_groupLeader = gleader;
    }

    void setSamplingValue(uint64_t value, bool byFreq = false)
    {
        m_samplebyFreq = byFreq;
        m_samplingValue = value;
    }

    // isByEvent is false, then the value will be bytes
    // isByEvent is true, then the value will be the number of events
    // happen before an overflow signal happens.
    int setWakeupWaterMark(uint64_t watermark, bool byEvent = true)
    {
        m_wakeupByEvents = byEvent;
        m_waterMark = watermark;
        return 0;
    }

    // set Sample Attributes :-
    // specifies the information that needs to be gathered while sampling
    int setSampleAttr(uint64_t sampleattr)
    {
        m_sampleAttr = sampleattr;
        return 0;
    }

    // set read format
    int setReadFormat(uint64_t readformat)
    {
        m_readFormat = readformat;
        return 0;
    }

    // Enable or disable the events - don't need here..
    void enableEvent() { }
    void disableEvent() { }

    // Debug routines
    void print();

protected:
    PerfEvent   m_event;
    bool      m_groupLeader;

    // if m_samplebyFreq is
    //   "false", m_samplingValue is period
    //   "true", m_samplingValue is frequency
    bool      m_samplebyFreq;
    uint64_t  m_samplingValue;

    // Sample Attributes - specifies the information that needs to be gathered
    // while sampling
    uint64_t  m_sampleAttr;

    // format of the data returned by read() on a perf evebnt fd;
    uint64_t  m_readFormat;

    // how many events or bytes happen before an overflow signal happens..
    // if m_wakeupByEvents is
    //    "true" then the m_waterMark is number of events;
    //    "false" then the m_waterMark is number of bytes;
    bool      m_wakeupByEvents;   // notify counter overflow events
    uint32_t  m_waterMark;

    // TODO: Buffer for overflow samples..
};


//
// Typedefs
//
typedef std::list<PerfEventInternal> PerfEventList;
typedef std::list<PerfEventList> PerfEventGroupList;


#endif // _PERFEVENTINTERNAL_H_
