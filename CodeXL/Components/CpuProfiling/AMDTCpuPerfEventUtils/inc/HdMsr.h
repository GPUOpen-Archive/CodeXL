//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file HdMsr.h
/// \brief Hardware MSRs header.
///
//==================================================================================

#ifndef _HDMSR_H
#define _HDMSR_H

#include <stdlib.h>

/// \def FAKE_L2I_MASK_VALUE
/// The value of FakeL2IMask in PERF_CTL for L2I events
#define FAKE_L2I_MASK_VALUE        0xD
/// \def FAKE_L2I_EVENT_ID_PREFIX
/// The value of bits[15:12] in 16 bit L2I event
/// This is to distinguish L2I events having event id same as of some core event
#define FAKE_L2I_EVENT_ID_PREFIX   0xD
/// \def FAKE_L2I_MSR_CTL_BITS
/// Using L2I MSR_CTL reserved bits [39:36] to distinguish L2I events
/// having event id same as of some core event
#define FAKE_L2I_MSR_CTL_BITS 0xD000000000

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    // Disable C4201 : nameless struct/union
    #pragma warning(disable : 4201)
#endif

/// \union PERF_CTL A simple way to encode the bits for a performance
/// event select register (PERF_CTL)
typedef union
{
    struct
    {
        /// Bits [7:0] of the event to be monitored
        UCHAR ucEventSelect : 8;
        /// Bits[15:8]: Unit mask for the given event
        UCHAR ucUnitMask : 8;
        /// bit [16]: User mode (CPL > 0) events are counted.  Either this or
        /// \ref bitOsEvents need to be 1 for data
        UCHAR bitUsrEvents : 1;
        /// bit [17]: OS mode (CPL = 0) events are counted.  Either this or
        /// \ref bitUsrEvents need to be 1 for data
        UCHAR bitOsEvents : 1;
        /// bit [18]: Edge: The events are determined from edge detection
        UCHAR bitEdgeEvents : 1;
        /// bit [19]:  PC: pin control
        UCHAR bitPinControl : 1;
        /// bit [20]: The events should be sampled, do not include this for counting events
        /// Int: enable APIC interrupt
        UCHAR bitSampleEvents : 1;
        /// bit [21]: Reserved
        UCHAR bitReserved : 1;
        /// bit [22]: En: The configuration will be enabled.  This always needs to be present
        /// En: enable performance counter
        UCHAR bitEnabled : 1;
        /// bit [23]: Inv: invert counter mask
        UCHAR bitInvert : 1;
        /// Bit[31:24]: CntMask: Number of events for a single cycle count. CntMask: counter mask.
        /// 0 means the counter is incremented by the number of events in a clock
        /// cycle. 1-3 mean the counter is incremented by 1, if at least that
        /// number of events occur in one clock cycle
        UCHAR ucCounterMask : 8;
        /// Bit[35:32] : EventSelect[11:8] of the event to be monitored
        UCHAR ucEventSelectHigh : 4;
        /// Bit[39:36] Reserved
        /// The event ids of some of the L2I events and core events are same so
        /// using these reserved bit to distinguish the L2I events
        /// For L2I events - keeping the FakeL2IMask as FAKE_L2I_MASK_VALUE
        UCHAR FakeL2IMask : 4;
        /// Bit[40]: GuestOnly: Exclusively count only events that occur when the processor is in the
        /// virtual guest mode.  Exclusive with \ref hostOnly
        UCHAR guestOnly : 1;
        /// Bit[41] Exclusively count only events that occur when the processor is in the
        /// virtual host mode.  Exclusive with \ref guestOnly
        UCHAR hostOnly : 1;
        /// Reserved
        ULONG Reserved : 22;
    };
    /// The encoded control value for a performance event counter
    ULONG64 perf_ctl;
} PERF_CTL;


/// \union NB_PERF_CTL A simple way to encode the bits for a NB performance
/// event select register (NB_PERF_CTL)
typedef union
{
    struct
    {
        /// Bits [7:0] : EventSelect[7:0] of the event to be monitored
        UCHAR ucEventSelect : 8;
        /// Bits[15:8] : Unit mask: event qualification
        UCHAR ucUnitMask : 8;
        /// bit [18:16]: reserved
        UCHAR ucReserved1 : 3;
        /// bit [19]: PC: pin control
        UCHAR bitPinControl : 1;
        /// Bit [20]: Int: enable APIC interrupt
        // The events should be sampled, do not include this for counting events
        UCHAR bitSampleEvents : 1;
        /// Bit [21]: Reserved
        UCHAR bitReserved : 1;
        /// Bit [22]: En: enable performance counter
        /// The configuration will be enabled.  This always needs to be present
        UCHAR bitEnabled : 1;
        /// Bit [31:23] Reserved
        USHORT ReservedBit31_23 : 9;
        /// Bits [35:32] : EventSelect[11:8] of the event to be monitored
        UCHAR ucEventSelectHigh : 4;
        /// Bit [36] IntCoreEn: interrupt to core enable; 1 = interrupt to a single
        //           core specifiedby IntCoreSel. 0 = interrupt to all core.
        UCHAR bitIntCoreEn : 1;
        /// Bit [40:37] : IntCoreSel: interrupt to core select.
        UCHAR guestOnly : 4;
        /// bit [63:41] : Reserved
        ULONG Reserved : 23;
    };
    /// The encoded control value for a performance event counter
    ULONG64 nb_perf_ctl;
} NB_PERF_CTL;


/// \union IBSFetchCtl: A simple way to encode the bits for IBS Fetch
/// Control Register (IbsFetchCtl or IC_IBS_CTL)
typedef union
{
    struct
    {
        /// Bits [15:0] : IbsFetchMaxCnt specifies maximum count
        ///     value of periodic fetch count
        USHORT ibsFetchMaxCnt : 16;
        /// Bits[31:16] : IbsFetchCnt: returns current value of bits [19:4] of
        ///     the periodic fetch counter
        USHORT ibsFetchCnt : 16;
        /// bit [47:32]: IbsFetchLat: instruction fetch latency
        USHORT ibsFetchLat : 16;
        /// bit [48]: IbsFetchEn : instruciton fetch enable. 1 = IBS fetch enabled
        UCHAR ibsFetchEn : 1;
        /// Bit [49]: IbsFetchVal : instruction fetch valid. 1 = ibs fetch data available
        UCHAR ibsFetchVal : 1;
        /// Bit [50]: ibsFetchComp : instruction fetch complete
        UCHAR ibsFetchComp : 1;
        /// Bit [51]: IbsIcMiss : instruction cache miss
        UCHAR ibsIcMiss : 1;
        /// Bit [52] : IbsPhyAddrValid : instruction fetch physical address valid
        UCHAR ibsPhyAddrValid : 1;
        /// Bits [54:53] : ibsL1TlbPgSz; instruction cache L1TLB page size.
        UCHAR ibsL1TlbPgSz : 2;
        /// Bit [55] IbsL1TlbMiss : instruction cache L1TLB miss
        UCHAR ibsL1TlbMiss : 1;
        /// Bit [56] IbsL2TlbMiss :  instruction cahce L2TLB miss
        UCHAR ibsL2TlbMiss : 1;
        /// Bit [57] IbsRandEn : random instruction fetch tagging enable
        UCHAR ibsRandEn : 1;
        /// Bit [58] IbsFetchL2Miss : L2 cache miss
        UCHAR ibsFetchL2Miss : 1;
        /// bit [63:59] : Reserved
        UCHAR Reserved : 5;
    };
    /// The encoded control value for a performance event counter
    ULONG64 ibsFetchCtl;
} IBSFetchCtl;


/// \union IbsFetchCtlExtd: A simple way to encode the bits for IBS Fetch
/// Control Extended Register (IbsFetchCtlExtd or IC_IBS_EXTD_CTL)
typedef union
{
    struct
    {
        /// Bit [15:0] IbsItlbRefillLat : ITLB Refill Latency
        USHORT ibsItlbRefillLat : 16;
        /// Bit [63:16] : Reserved
        ULONG64 reserved_63_16 : 48;
    };
    /// The encoded extended control value
    ULONG64 ibsFetchCtlExtd;
} IbsFetchCtlExtd;


/// \union IbsOpCtl: A simple way to encode the bits for IBS execution
/// Control Register (IbsOpCtl or SC_IBS_CTL)
typedef union
{
    struct
    {
        /// Bits [15:0] : IbsOpMaxCnt specifies maximum count
        ///     value of periodic
        USHORT ibsOpMaxCnt : 16;
        /// Bits[16] : Reserved
        UCHAR reservedBit16 : 1;
        /// bit [17]: IbsOpEn : micro-op sampling enable
        UCHAR ibsOpEn : 1;
        /// bit [18]: IbsOpVal : micro-op sample valid
        UCHAR ibsOpVal : 1;
        /// Bit [19]: IbsOpCntCtl : periodic op counter count control
        ///     1 = dispatched op; 0 = clock cycles
        UCHAR ibsOpCntCtl : 1;
        /// Bit [26:20]: IbsOpMaxCntExt : periodic op counter maximum
        ///     only available after family 12h
        UCHAR ibsOpMaxCntExe : 7;
        /// Bit [31:27]: reserved
        UCHAR reserved_31_27: 5;
        /// Bit [58:32] : IbsOpCurCnt[26:0]: periodic op counter current count;
        ULONG ibsOpCurCnt : 27;
        /// Bits [63:59] : Reserved
        UCHAR reserved_63_59 : 5;
    };
    /// The encoded control value for a performance event counter
    ULONG64 ibsOpCtl;
} IBSOpCtl;

/// \union IbsOpData: A simple way to encode the bits for IBS Op Data
/// Register (IbsOpData or SC_IBS_DATA)
typedef union
{
    struct
    {
        /// Bits [15:0] : IbsCompToRetCtr : micro-op completion to retire count
        USHORT ibsCompToRetCtr : 16;
        /// Bits [31:15] : IbsTagToretCtr: Micro-op tag to retire count
        USHORT ibsTagToretCtr : 16;
        /// Bits[32] : IbsOpBrnResync : resync micro-op; 1 = targged operation is resync micro-op
        UCHAR ibsOpBrnResync : 1;
        /// bit [33]: IbsOpMispReturn: mispredicted return micro-op
        UCHAR ibsOpMispReturn : 1;
        /// bit [34] IbsOpReturn: return micro-op.
        UCHAR ibsOpReturn : 1;
        /// Bit [35] IbsOpBrnTaken: taken branch micro-op
        UCHAR ibsOpBrnTaken : 1;
        /// Bit [36]  IbsOpBrnMisp: mispredicted branch micro-op.
        UCHAR ibsOpBrnMisp : 1;
        /// Bit [37] : IbsOpBrnRet: branch micro-op retired
        UCHAR ibsOpBrnRet: 1;
        /// Bit [38] : IbsRipInvalid: RIP invalid
        ///         Only available after family 10h
        UCHAR ibsRipInvalid : 1;
        /// Bits [63:39] : Reserved
        ULONG reserved_63_39 : 25;
    };
    /// The encoded control value for a performance event counter
    ULONG64 ibsOpData;
} IBSOpData;


/// \union IbsOpData2: A simple way to encode the bits for IBS Op Data 2
/// Register (IbsOpData2 )
typedef union
{
    struct
    {
        /// Bits [2:0] : NbIbsReqSrc: northbridge IBS request data source
        UCHAR nbIbsReqSrc : 3;
        /// Bits[3] : Reserved
        UCHAR reservedBit3 : 1;
        /// Bits [4] : NbIbsReqDstProc: IBS request destination processor
        ///         Only available for 10h, 15h, not for 12h 14h
        UCHAR nbIbsReqDstProc : 1;
        /// Bits[5] : NbIbsReqCacheHitSt: IBS L3 cache state
        UCHAR nbIbsReqCacheHitSt : 1;
        /// Bits [63:6] : Reserved
        ULONG64 reserved_63_39 : 58;
    };
    /// The encoded control value for a performance event counter
    ULONG64 ibsOpData2;
} IBSOpData2;


/// \union IbsOpData3: A simple way to encode the bits for IBS Op Data 3
/// Register (IbsOpData3 )
typedef union
{
    struct
    {
        /// Bits [0] : IbsLdOp: load op.
        UCHAR ibsLdOp : 1;
        /// Bits [1] : IbsStOp: load op.
        UCHAR ibsStOp : 1;
        /// Bits [2] : IbsDcL1tlbMiss: data cache L1TLB miss
        UCHAR ibsDcL1tlbMiss : 1;
        /// Bits [3] : IbsDcL2tlbMiss: data cache L2TLB miss
        UCHAR ibsDcL2tlbMiss : 1;
        /// Bits [4] : IbsDcL1tlbHit2M: data cache L1TLB hit in 2M page
        UCHAR ibsDcL1tlbHit2M : 1;
        /// Bits [5] : IbsDcL1tlbHit1G: data cache L1TLB hit in 1G page
        UCHAR ibsDcL1tlbHit1G : 1;
        /// Bits [6] : IbsDcL2tlbHit2M: data cache L2TLB hit in 2M page
        UCHAR ibsDcL2tlbHit2M : 1;
        /// Bits [7] : IbsDcMiss: data cache miss
        UCHAR ibsDcMiss : 1;
        /// Bits [8] : IbsDcMisAcc: misaligned access
        UCHAR ibsDcMisAcc : 1;
        /// Bits [9] : IbsDcLdBnkCon: bank conflict on load operation
        ///     Reserved for 14h
        UCHAR ibsDcLdBnkCon : 1;
        /// Bits [10] : IbsDcStBnkCon: bank conflict on store operation
        //      Reserved for 14h and 15h.
        UCHAR ibsDcStBnkCon : 1;
        /// Bits [11] : IbsDcStToLdFwd: data forwarded from store to load operation
        UCHAR ibsDcStToLdFwd : 1;
        /// Bits [12] : IbsDcStToLdCan: data forwarding from store to load operation canceled
        ///     Reserved for 14h
        UCHAR ibsDcStToLdCan : 1;
        /// Bits [13] : IbsDcUcMemAcc: UC memory access
        UCHAR ibsDcUcMemAcc : 1;
        /// Bits [14] : IbsDcWcMemAcc: WC memory access
        UCHAR ibsDcWcMemAcc : 1;
        /// Bits [15] : IbsDcLockedOp: locked operation
        UCHAR ibsDcLockedOp : 1;
        /// Bits [16] : IbsDcMabHit: MAB hit
        UCHAR ibsDcMabHit : 1;
        /// Bits [17] : IbsDcLinAddrValid: data cache linear address valid.
        UCHAR ibsDcLinAddrValid : 1;
        /// Bits [18] : IbsDcPhyAddrValid: data cache physical address valid
        UCHAR ibsDcPhyAddrValid : 1;
        /// Bits [19] : IbsDcL2tlbHit1G: data cache L2TLB hit in 1G page
        ///     Reserved for 14h and 15h
        UCHAR ibsDcL2tlbHit1G : 1;
        /// Bits [31:20] : Reserved
        USHORT reserved_31_20 : 12;
        /// Bits [47:32] : IbsDcMissLat: data cache miss latency
        USHORT ibsDcMissLat : 16;
        /// Bits [63:48] : Reserved
        ULONG64 reserved_63_48 : 16;
    };
    /// The encoded control value for a performance event counter
    ULONG64 ibsOpData3;
} IBSOpData3;


/// \union IbsOpData4: A simple way to encode the bits for IBS Op Data 4
/// Register (IbsOpData4)
typedef union
{
    struct
    {
        /// Bit [0] IbsOpLdResync : Load Resync
        UCHAR ibsOpLdResync : 1;
        /// Bit [63:1] : Reserved
        ULONG64 reserved_63_1 : 63;
    };
    /// The encoded control value
    ULONG64 ibsOpData4;
} IBSOpData4;

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif

#endif //_HDMSR_H
