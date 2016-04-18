//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfTranslatorIbs.h
/// \brief This is the IBS specific definitions for the CAPERF file translation.
///
//==================================================================================

#ifndef _CAPERFTRANSLATORIBS_H_
#define _CAPERFTRANSLATORIBS_H_

#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

/**
 * This struct represents the hardware-level IBS fetch information.
 * Each field corresponds to a model-specific register (MSR.) See the
 * BIOS and Kernel Developer's Guide for AMD Model Family 10h Processors
 * for further details.
 */
struct ibs_fetch_sample
{
    /* MSRC001_1030 IBS Fetch Control Register */
    gtUInt32 ibs_fetch_ctl_low;
    gtUInt32 ibs_fetch_ctl_high;
    /* MSRC001_1031 IBS Fetch Linear Address Register */
    gtUInt32 ibs_fetch_lin_addr_low;
    gtUInt32 ibs_fetch_lin_addr_high;
    /* MSRC001_1032 IBS Fetch Physical Address Register */
    gtUInt32 ibs_fetch_phys_addr_low;
    gtUInt32 ibs_fetch_phys_addr_high;
    gtUInt32 dummy_event;
};

/** This struct represents the hardware-level IBS op information. */
struct ibs_op_sample
{
    /* MSRC001_1033 IBS Execution Control Register */
    gtUInt32 ibs_op_ctrl_low;
    gtUInt32 ibs_op_ctrl_high;
    /* MSRC001_1034 IBS Op Logical Address Register */
    gtUInt32 ibs_op_lin_addr_low;
    gtUInt32 ibs_op_lin_addr_high;
    /* MSRC001_1035 IBS Op Data Register */
    gtUInt32 ibs_op_data1_low;
    gtUInt32 ibs_op_data1_high;
    /* MSRC001_1036 IBS Op Data 2 Register */
    gtUInt32 ibs_op_data2_low;
    gtUInt32 ibs_op_data2_high;
    /* MSRC001_1037 IBS Op Data 3 Register */
    gtUInt32 ibs_op_data3_low;
    gtUInt32 ibs_op_data3_high;
    /* MSRC001_1038 IBS DC Linear Address */
    gtUInt32 ibs_op_ldst_linaddr_low;
    gtUInt32 ibs_op_ldst_linaddr_high;
    /* MSRC001_1039 IBS DC Physical Address */
    gtUInt32 ibs_op_phys_addr_low;
    gtUInt32 ibs_op_phys_addr_high;
    /* MSRC001_103B IBS Branch Target Address */
    gtUInt64 ibs_op_brtgt_addr;
};

//
// These macro decodes IBS hardware-level event flags and fields.
// Translation results are either zero (false) or non-zero (true), except
// the fetch latency, which is a 16-bit cycle count, and the fetch page size
// field, which is a 2-bit unsigned integer.
//

// Bits 47:32 IbsFetchLat: instruction fetch latency
#define IBS_FETCH_FETCH_LATENCY(x)              ((gtUInt16)(x->ibs_fetch_ctl_high & FETCH_MASK_LATENCY))

// Bit 50 IbsFetchComp: instruction fetch complete.
#define IBS_FETCH_FETCH_COMPLETION(x)           ((x->ibs_fetch_ctl_high & FETCH_MASK_COMPLETE) != 0)

// Bit 51 IbsIcMiss: instruction cache miss.
#define IBS_FETCH_INST_CACHE_MISS(x)            ((x->ibs_fetch_ctl_high & FETCH_MASK_IC_MISS) != 0)

// Bit 52 IbsPhyAddrValid: instruction fetch physical address valid.
#define IBS_FETCH_PHYS_ADDR_VALID(x)            ((x->ibs_fetch_ctl_high & FETCH_MASK_PHY_ADDR) != 0)

// Bits 54:53 IbsL1TlbPgSz: instruction cache L1TLB page size.
#define IBS_FETCH_TLB_PAGE_SIZE(x)              ((gtUInt16)((x->ibs_fetch_ctl_high >> 21) & 0x3))
#define IBS_FETCH_TLB_PAGE_SIZE_4K(x)           (IBS_FETCH_TLB_PAGE_SIZE(x) == L1TLB4K)
#define IBS_FETCH_TLB_PAGE_SIZE_2M(x)           (IBS_FETCH_TLB_PAGE_SIZE(x) == L1TLB2M)
#define IBS_FETCH_TLB_PAGE_SIZE_1G(x)           (IBS_FETCH_TLB_PAGE_SIZE(x) == L1TLB1G)

// Bit 55 IbsL1TlbMiss: instruction cache L1TLB miss.//
#define IBS_FETCH_M_L1_TLB_MISS(x)              ((x->ibs_fetch_ctl_high & FETCH_MASK_L1_MISS) != 0)

// Bit 56 IbsL2TlbMiss: instruction cache L2TLB miss.//
#define IBS_FETCH_L2_TLB_MISS(x)                ((x->ibs_fetch_ctl_high & FETCH_MASK_L2_MISS) != 0)

// A fetch is a killed fetch if all the masked bits are clear//
#define IBS_FETCH_KILLED(x)                     ((x->ibs_fetch_ctl_high & FETCH_MASK_KILLED) == 0)

#define IBS_FETCH_INST_CACHE_HIT(x)             (IBS_FETCH_FETCH_COMPLETION(x) && !IBS_FETCH_INST_CACHE_MISS(x))

#define IBS_FETCH_L1_TLB_HIT(x)                 (!IBS_FETCH_M_L1_TLB_MISS(x) && IBS_FETCH_PHYS_ADDR_VALID(x))

#define IBS_FETCH_ITLB_L1M_L2H(x)               (IBS_FETCH_M_L1_TLB_MISS(x) && !IBS_FETCH_L2_TLB_MISS(x))

#define IBS_FETCH_ITLB_L1M_L2M(x)               (IBS_FETCH_M_L1_TLB_MISS(x) && IBS_FETCH_L2_TLB_MISS(x))


//
// These macros translates IBS op event data from its hardware-level
// representation .It hides the MSR layout of IBS op data.
//

//
// MSRC001_1035 IBS OP Data Register (IbsOpData)
//
// 15:0 IbsCompToRetCtr: macro-op completion to retire count
//
#define IBS_OP_COM_TO_RETIRE_CYCLES(x)          ((gtUInt16)(x->ibs_op_data1_low & BR_MASK_RETIRE))

// 31:16 tag_to_retire_cycles : macro-op tag to retire count.//
#define IBS_OP_TAG_TO_RETIRE_CYCLES(x)          ((gtUInt16)((x->ibs_op_data1_low >> 16) & BR_MASK_RETIRE))

// 32 op_branch_resync : resync macro-op.//
#define IBS_OP_BRANCH_RESYNC(x)                 ((x->ibs_op_data1_high & BR_MASK_BRN_RESYNC) != 0)

// 33 op_mispredict_return : mispredicted return macro-op.//
#define IBS_OP_MISPREDICT_RETURN(x)             ((x->ibs_op_data1_high & BR_MASK_MISP_RETURN) != 0)

// 34 IbsOpReturn: return macro-op.//
#define IBS_OP_RETURN(x)                        ((x->ibs_op_data1_high & BR_MASK_RETURN) != 0)

// 35 IbsOpBrnTaken: taken branch macro-op.//
#define IBS_OP_BRANCH_TAKEN(x)                  ((x->ibs_op_data1_high & BR_MASK_BRN_TAKEN) != 0)

// 36 IbsOpBrnMisp: mispredicted branch macro-op. //
#define IBS_OP_BRANCH_MISPREDICT(x)             ((x->ibs_op_data1_high & BR_MASK_BRN_MISP) != 0)

// 37 IbsOpBrnRet: branch macro-op retired.//
#define IBS_OP_BRANCH_RETIRED(x)                ((x->ibs_op_data1_high & BR_MASK_BRN_RET) != 0)

// 38 IbsRipInvalid: RIP invalid.//
#define IBS_OP_RIP_INVALID(x)                   ((x->ibs_op_data1_high & RIP_INVALID) != 0)

//
// MSRC001_1036 IBS Op Data 2 Register (IbsOpData2)
//
// 5 NbIbsReqCacheHitSt: IBS L3 cache state
//
#define IBS_OP_NB_IBS_CACHE_HIT_ST(x)           ((x->ibs_op_data2_low & NB_MASK_L3_STATE) != 0)

// 4 NbIbsReqDstProc: IBS request destination processor//
#define IBS_OP_NB_IBS_REQ_DST_PROC(x)           ((x->ibs_op_data2_low & NB_MASK_REQ_DST_PROC) != 0)

// 2:0 NbIbsReqSrc: Northbridge IBS request data source//
#define IBS_OP_NB_IBS_REQ_SRC(x)                ((unsigned char)(x->ibs_op_data2_low & NB_MASK_REQ_DATA_SRC))

#define IBS_OP_NB_IBS_REQ_SRC_01(x)             (IBS_OP_NB_IBS_REQ_SRC(x) == 0x01)

#define IBS_OP_NB_IBS_REQ_SRC_02(x)             (IBS_OP_NB_IBS_REQ_SRC(x) == 0x02)

#define IBS_OP_NB_IBS_REQ_SRC_03(x)             (IBS_OP_NB_IBS_REQ_SRC(x) == 0x03)

#define IBS_OP_NB_IBS_REQ_SRC_07(x)             (IBS_OP_NB_IBS_REQ_SRC(x) == 0x07)

//
// MSRC001_1037 IBS Op Data3 Register
//
// Bits 47:32   IbsDcMissLat (family10h)
//
#define IBS_OP_DC_MISS_LATENCY(x)               ((gtUInt16)(x->ibs_op_data3_high & 0XFFFF))

// 0 IbsLdOp: Load op//
#define IBS_OP_IBS_LD_OP(x)                     ((x->ibs_op_data3_low & DC_MASK_LOAD_OP) != 0)

// 1 IbsStOp: Store op//
#define IBS_OP_IBS_ST_OP(x)                     ((x->ibs_op_data3_low & DC_MASK_STORE_OP) != 0)

// 2 ibs_dc_l1_tlb_miss: Data cache L1TLB miss//
#define IBS_OP_IBS_DC_L1_TLB_MISS(x)            ((x->ibs_op_data3_low & DC_MASK_L1_TLB_MISS) != 0)

// 3 ibs_dc_l2_tlb_miss: Data cache L2TLB miss//
#define IBS_OP_IBS_DC_L2_TLB_MISS(x)            ((x->ibs_op_data3_low & DC_MASK_L2_TLB_MISS) != 0)

// 4 IbsDcL1tlbHit2M: Data cache L1TLB hit in 2M page//
#define IBS_OP_IBS_DC_L1_TLB_HIT_2MB(x)         ((x->ibs_op_data3_low & DC_MASK_L1_HIT_2M) != 0)

// 5 ibs_dc_l1_tlb_hit_1gb: Data cache L1TLB hit in 1G page//
#define IBS_OP_IBS_DC_L1_TLB_HIT_1GB(x)         ((x->ibs_op_data3_low & DC_MASK_L1_HIT_1G) != 0)

// 6 ibs_dc_l2_tlb_hit_2mb: Data cache L2TLB hit in 2M page//
#define IBS_OP_IBS_DC_L2_TLB_HIT_2MB(x)         ((x->ibs_op_data3_low & DC_MASK_L2_HIT_2M) != 0)

// 7 ibs_dc_miss: Data cache miss//
#define IBS_OP_IBS_DC_MISS(x)                   ((x->ibs_op_data3_low & DC_MASK_DC_MISS) != 0)

// 8 ibs_dc_miss_acc: Misaligned access//
#define IBS_OP_IBS_DC_MISS_ACC(x)               ((x->ibs_op_data3_low & DC_MASK_MISALIGN_ACCESS) != 0)

// 9 ibs_dc_ld_bnk_con: Bank conflict on load operation//
#define IBS_OP_IBS_DC_LD_BNK_CON(x)             ((x->ibs_op_data3_low & DC_MASK_LD_BANK_CONFLICT) != 0)

// 10 ibs_dc_st_bnk_con: Bank conflict on store operation//
#define IBS_OP_IBS_DC_ST_BNK_CON(x)             ((x->ibs_op_data3_low & DC_MASK_ST_BANK_CONFLICT) != 0)

// 11 ibs_dc_st_to_ld_fwd : Data forwarded from store to load operation//
#define IBS_OP_IBS_DC_ST_TO_LD_FWD(x)           ((x->ibs_op_data3_low & DC_MASK_ST_TO_LD_FOR) != 0)

// 12 ibs_dc_st_to_ld_can: Data forwarding from store to load operation cancelled//
#define IBS_OP_IBS_DC_ST_TO_LD_CAN(x)           ((x->ibs_op_data3_low & DC_MASK_ST_TO_LD_CANCEL) != 0)

// 13 ibs_dc_wc_mem_acc : WC memory access//
#define IBS_OP_IBS_DC_WC_MEM_ACC(x)             ((x->ibs_op_data3_low & DC_MASK_WC_MEM_ACCESS) != 0)

// 14 ibs_dc_uc_mem_acc: UC memory access//
#define IBS_OP_IBS_DC_UC_MEM_ACC(x)             ((x->ibs_op_data3_low & DC_MASK_UC_MEM_ACCESS) != 0)

// 15 ibs_locked_op: Locked operation//
#define IBS_OP_IBS_LOCKED_OP(x)                 ((x->ibs_op_data3_low & DC_MASK_LOCKED_OP) != 0)

// 16 ibs_dc_mab_hit : MAB hit//
#define IBS_OP_IBS_DC_MAB_HIT(x)                ((x->ibs_op_data3_low & DC_MASK_MAB_HIT) != 0)

// 17 IbsDcLinAddrValid: Data cache linear address valid//
#define IBS_OP_IBS_DC_LIN_ADDR_VALID(x)         ((x->ibs_op_data3_low & DC_MASK_LIN_ADDR_VALID) != 0)

// 18 ibs_dc_phy_addr_valid: Data cache physical address valid//
#define IBS_OP_IBS_DC_PHY_ADDR_VALID(x)         ((x->ibs_op_data3_low & DC_MASK_PHY_ADDR_VALID) != 0)

// 19 ibs_dc_l2_tlb_hit_1gb: Data cache L2TLB hit in 1G page//
#define IBS_OP_IBS_DC_L2_TLB_HIT_1GB(x)         ((x->ibs_op_data3_low & DC_MASK_L2_HIT_1G) != 0)

#endif // _CAPERFTRANSLATORIBS_H_

