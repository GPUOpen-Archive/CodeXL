//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventEncoding.h
/// \brief This file contains all the simple type definitions, and some cross-compiler definitions.
///
//==================================================================================

#ifndef _EVENTENCODING_H_
#define _EVENTENCODING_H_

#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include "CpuPerfEventUtilsDLLBuild.h"

#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    #define UCHAR    gtUByte
    #define USHORT   gtUInt16
    #define ULONG    gtUInt32
    #define ULONG64  gtUInt64
#endif
#include "HdMsr.h"
#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    #undef UCHAR
    #undef USHORT
    #undef ULONG
    #undef ULONG64
#endif

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    // Disable C4201 : nameless struct/union
    #pragma warning(disable : 4201)
#endif

// The EventMaskType is EventUnitMask + EventType. The EventType is 16 bits
// and the EventUnitMask is 8 bits. This is used as a key in SampleKey class.
typedef gtUInt32 EventMaskType;

union EventMaskTypeEnc
{
    struct
    {
        ///  [15:0] Event select
        gtUInt16 ucEventSelect : 16;
        /// Bits[23:16]: Unit mask for the given event
        gtUByte ucUnitMask : 8;
        /// bit [24]: User mode (CPL > 0) events are counted.  Either this or
        /// \ref bitOsEvents need to be 1 for data
        gtUByte bitUsrEvents : 1;
        /// bit [25]: OS mode (CPL = 0) events are counted.  Either this or
        /// \ref bitUsrEvents need to be 1 for data
        gtUByte bitOsEvents : 1;
        /// bit [31:26]: Reserved
        gtUByte bitReserved : 6;
    };
    /// The encoded control value for a performance event counter
    gtUInt32 encodedEvent;
};

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif

//*********************************************************
// struct EventEncodeType
//
// Description:
// This class represent each event setting including
// - Index in TBP/EBP file
// - Event Select/Mask encoding
// - Count
//
struct EventEncodeType
{
    EventMaskType eventMask;
    gtUInt64 eventCount;
    unsigned int sortedIndex;
};

//
// This vector is used to store information
// of available events, and will be written to
// the TBP/EBP file
//
typedef gtVector<EventEncodeType> EventEncodeVec;


CP_EVENT_API EventMaskType EncodeEvent(gtUInt16 event, gtUByte unitMask, bool bitOs, bool bitUsr);
CP_EVENT_API EventMaskType EncodeEvent(PERF_CTL ctl);

CP_EVENT_API void DecodeEvent(EventMaskType encoded, gtUInt16* pEvent, gtUByte* pUnitMask, bool* pBitOs, bool* pBitUsr);
CP_EVENT_API void DecodeEvent(EventMaskType encoded, PERF_CTL* pCtl);

CP_EVENT_API unsigned int GetEvent12BitSelect(PERF_CTL ctl);

#endif // _EVENTENCODING_H_
