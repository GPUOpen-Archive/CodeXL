//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IbsEvents.h
/// \brief IBS control values.
///
//==================================================================================

#ifndef _IBSEVENTS_H_
#define _IBSEVENTS_H_

//
// The following defines are bit masks that are used to select
// IBS fetch event flags and values at the
// MSRC001_1030 IBS Fetch Control Register (IbsFetchCtl)
//
#define FETCH_MASK_LATENCY  0X0000FFFF
#define FETCH_MASK_COMPLETE 0x00040000
#define FETCH_MASK_IC_MISS  0x00080000
#define FETCH_MASK_PHY_ADDR 0x00100000
#define FETCH_MASK_PG_SIZE  0x00600000
#define FETCH_MASK_L1_MISS  0x00800000
#define FETCH_MASK_L2_MISS  0x01000000
#define FETCH_MASK_L2_CACHE_MISS  0x04000000
#define FETCH_MASK_KILLED   (FETCH_MASK_L1_MISS|FETCH_MASK_L2_MISS|FETCH_MASK_PHY_ADDR|FETCH_MASK_COMPLETE|FETCH_MASK_IC_MISS|FETCH_MASK_L2_CACHE_MISS)


//
// The following defines are bit masks that are used to select
// IBS op event flags and values at the MSR level.
//

// MSRC001_1035 IBS Op Data Register (IbsOpData)
#define BR_MASK_RETIRE           0X0000FFFF
#define RIP_INVALID              0x00000040
#define BR_MASK_BRN_RET          0x00000020
#define BR_MASK_BRN_MISP         0x00000010
#define BR_MASK_BRN_TAKEN        0x00000008
#define BR_MASK_RETURN           0x00000004
#define BR_MASK_MISP_RETURN      0x00000002
#define BR_MASK_BRN_RESYNC       0x00000001

// MSRC001_1036 IBS Op Data Register (IbsOpData2)
#define NB_MASK_L3_STATE         0x00000020
#define NB_MASK_REQ_DST_PROC     0x00000010
#define NB_MASK_REQ_DATA_SRC     0x00000007

// MSRC001_1037 IBS Op Data Register (IbsOpData3)
#define DC_MASK_L2_HIT_1G        0x00080000
#define DC_MASK_PHY_ADDR_VALID   0x00040000
#define DC_MASK_LIN_ADDR_VALID   0x00020000
#define DC_MASK_MAB_HIT          0x00010000
#define DC_MASK_LOCKED_OP        0x00008000
#define DC_MASK_UC_MEM_ACCESS    0x00004000
#define DC_MASK_WC_MEM_ACCESS    0x00002000
#define DC_MASK_ST_TO_LD_CANCEL  0x00001000
#define DC_MASK_ST_TO_LD_FOR     0x00000800
#define DC_MASK_ST_BANK_CONFLICT 0x00000400
#define DC_MASK_LD_BANK_CONFLICT 0x00000200
#define DC_MASK_MISALIGN_ACCESS  0x00000100
#define DC_MASK_DC_MISS          0x00000080
#define DC_MASK_L2_HIT_2M        0x00000040
#define DC_MASK_L1_HIT_1G        0x00000020
#define DC_MASK_L1_HIT_2M        0x00000010
#define DC_MASK_L2_TLB_MISS      0x00000008
#define DC_MASK_L1_TLB_MISS      0x00000004
#define DC_MASK_STORE_OP         0x00000002
#define DC_MASK_LOAD_OP          0x00000001

// MSRC001_103D IBS Op Data Register (IbsOpData4)
#define DC_MASK_LD_RESYNC        0x00000001


//
// IBS derived events:
//
// IBS derived events are identified by event select values which are
// similar to the event select values that identify performance monitoring
// counter (PMC) events. Event select values for IBS derived events begin
// at 0xf000.
//
// The definitions in this file//must* match definitions
// of IBS derived events. More information
// about IBS derived events is given in the Software Oprimization
// Guide.
//

//
// The following defines associate a 16-bit select value with an IBS
// derived fetch event.
//
#define DE_IBS_FETCH_ALL         0XF000
#define DE_IBS_FETCH_KILLED      0XF001
#define DE_IBS_FETCH_ATTEMPTED   0XF002
#define DE_IBS_FETCH_COMPLETED   0XF003
#define DE_IBS_FETCH_ABORTED     0XF004
#define DE_IBS_L1_ITLB_HIT       0XF005
#define DE_IBS_ITLB_L1M_L2H      0XF006
#define DE_IBS_ITLB_L1M_L2M      0XF007
#define DE_IBS_IC_MISS           0XF008
#define DE_IBS_IC_HIT            0XF009
#define DE_IBS_FETCH_4K_PAGE     0XF00A
#define DE_IBS_FETCH_2M_PAGE     0XF00B
#define DE_IBS_FETCH_1G_PAGE     0XF00C
#define DE_IBS_FETCH_XX_PAGE     0XF00D
#define DE_IBS_FETCH_LATENCY     0XF00E
#define DE_IBS_FETCH_L2C_MISS    0XF00F
#define DE_IBS_ITLB_REFILL_LAT   0XF010

#define IBS_FETCH_BASE           0XF000
#define IBS_FETCH_END            0XF010
#define IBS_FETCH_MAX            0xF0FF
#define IBS_FETCH_OFFSET(x)      ((x) - IBS_FETCH_BASE)
#define CHECK_FETCH_SELECTED_FLAG(x)    if ( selected_flag & (1 << IBS_FETCH_OFFSET(x)))


//
// The following defines associate a 16-bit select value with an IBS
// derived branch/return macro-op event.
//
#define DE_IBS_OP_ALL             0XF100
#define DE_IBS_OP_TAG_TO_RETIRE   0XF101
#define DE_IBS_OP_COMP_TO_RETIRE  0XF102
#define DE_IBS_BRANCH_RETIRED     0XF103
#define DE_IBS_BRANCH_MISP        0XF104
#define DE_IBS_BRANCH_TAKEN       0XF105
#define DE_IBS_BRANCH_MISP_TAKEN  0XF106
#define DE_IBS_RETURN             0XF107
#define DE_IBS_RETURN_MISP        0XF108
#define DE_IBS_RESYNC             0XF109
#define DE_IBS_BR_ADDR            0XF10A

#define IBS_OP_BASE               0XF100
#define IBS_OP_END                0XF10A
#define IBS_OP_MAX                0xF2FF
#define IBS_OP_OFFSET(x)          (x - IBS_OP_BASE)
#define CHECK_OP_SELECTED_FLAG(x)   if ( selected_flag & (1 << IBS_OP_OFFSET(x)))


//
// The following defines associate a 16-bit select value with an IBS
// derived load/store event.
//
#define DE_IBS_LS_ALL_OP         0XF200
#define DE_IBS_LS_LOAD_OP        0XF201
#define DE_IBS_LS_STORE_OP       0XF202
#define DE_IBS_LS_DTLB_L1H       0XF203
#define DE_IBS_LS_DTLB_L1M_L2H   0XF204
#define DE_IBS_LS_DTLB_L1M_L2M   0XF205
#define DE_IBS_LS_DC_MISS        0XF206
#define DE_IBS_LS_DC_HIT         0XF207
#define DE_IBS_LS_MISALIGNED     0XF208
#define DE_IBS_LS_BNK_CONF_LOAD  0XF209
#define DE_IBS_LS_BNK_CONF_STORE 0XF20A
#define DE_IBS_LS_STL_FORWARDED  0XF20B
#define DE_IBS_LS_STL_CANCELLED  0XF20C
#define DE_IBS_LS_UC_MEM_ACCESS  0XF20D
#define DE_IBS_LS_WC_MEM_ACCESS  0XF20E
#define DE_IBS_LS_LOCKED_OP      0XF20F
#define DE_IBS_LS_MAB_HIT        0XF210
#define DE_IBS_LS_L1_DTLB_4K     0XF211
#define DE_IBS_LS_L1_DTLB_2M     0XF212
#define DE_IBS_LS_L1_DTLB_1G     0XF213
#define DE_IBS_LS_L1_DTLB_RES    0XF214
#define DE_IBS_LS_L2_DTLB_4K     0XF215
#define DE_IBS_LS_L2_DTLB_2M     0XF216
#define DE_IBS_LS_L2_DTLB_1G     0XF217
#define DE_IBS_LS_L2_DTLB_RES2   0XF218
#define DE_IBS_LS_DC_LOAD_LAT    0XF219
#define DE_IBS_LS_DC_LIN_ADDR    0XF21A
#define DE_IBS_LS_DC_PHY_ADDR    0XF21B
#define DE_IBS_LS_DC_LD_RESYNC   0XF21C

#define IBS_OP_LS_BASE           0XF200
#define IBS_OP_LS_END            0XF21C
#define IBS_OP_LS_OFFSET(x)      (x - IBS_OP_LS_BASE)
#define CHECK_OP_LS_SELECTED_FLAG(x)    if ( selected_flag & (1 << IBS_OP_LS_OFFSET(x)))


//
// The following defines associate a 16-bit select value with an IBS
// derived Northbridge (NB) event.
//
#define DE_IBS_NB_LOCAL          0XF240
#define DE_IBS_NB_REMOTE         0XF241
#define DE_IBS_NB_LOCAL_L3       0XF242
#define DE_IBS_NB_LOCAL_CACHE    0XF243
#define DE_IBS_NB_REMOTE_CACHE   0XF244
#define DE_IBS_NB_LOCAL_DRAM     0XF245
#define DE_IBS_NB_REMOTE_DRAM    0XF246
#define DE_IBS_NB_LOCAL_OTHER    0XF247
#define DE_IBS_NB_REMOTE_OTHER   0XF248
#define DE_IBS_NB_CACHE_STATE_M  0XF249
#define DE_IBS_NB_CACHE_STATE_O  0XF24A
#define DE_IBS_NB_LOCAL_LATENCY  0XF24B
#define DE_IBS_NB_REMOTE_LATENCY 0XF24C

#define IBS_OP_NB_BASE           0XF240
#define IBS_OP_NB_END            0XF24C
#define IBS_OP_NB_OFFSET(x)      (x - IBS_OP_NB_BASE)
#define CHECK_OP_NB_SELECTED_FLAG(x)    if ( selected_flag & (1 << IBS_OP_NB_OFFSET(x)))


//
// The following defines associate a 16-bit select value with an IBS
// derived CLU event.
//
#define DE_IBS_CLU_PERCENTAGE       0XFF00
#define DE_IBS_CLU_SPANNING         0XFF01
#define DE_IBS_CLU_BYTE_PER_EVICT   0XFF02
#define DE_IBS_CLU_ACCESS_PER_EVICT 0XFF03
#define DE_IBS_CLU_EVICT_COUNT      0XFF04
#define DE_IBS_CLU_ACCESS_COUNT     0XFF05
#define DE_IBS_CLU_BYTE_COUNT       0XFF06

#define IBS_CLU_BASE                0XFF00
#define IBS_CLU_END                 0XFF06
#define IBS_CLU_MAX                 0XFF06
#define IBS_CLU_OFFSET(x)           (x - IBS_CLU_BASE)

enum IBSL1PAGESIZE
{
    L1TLB4K = 0,
    L1TLB2M,
    L1TLB1G,
    L1TLB_INVALID
};

#endif // _IBSEVENTS_H_
