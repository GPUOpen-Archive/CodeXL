//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfTranslatorIbs.cpp
///
//==================================================================================

#include "CaPerfTranslator.h"
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

void CaPerfTranslator::_log_ibs(CpuProfileProcess* pProc,
                                gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                gtUInt32 event, gtUInt32 umask, gtUInt32 os, gtUInt32 usr,
                                gtUInt32 count,
                                const FunctionSymbolInfo* pFuncInfo)
{
    if (m_ibsEventMap.end() != m_ibsEventMap.find(event))
    {
        m_ibsEventMap[event] += 1;
    }
    else
    {
        m_ibsEventMap.insert(gtMap<gtUInt32, gtUInt32>::value_type(event, 1));
    }

    _addSampleToProcessAndModule(
        pProc, ldAddr, funcSize, pMod,
        ip, pid, tid, cpu,
        event, umask, os, usr,
        count,
        pFuncInfo);
}


/*
 * --------------------- FETCH DERIVED FUNCTION
 */
void CaPerfTranslator::trans_ibs_fetch(struct ibs_fetch_sample* trans_fetch,
                                       gtUInt32 selected_flag,
                                       CpuProfileProcess* pProc,
                                       gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                       gtUInt64 ip,
                                       gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                       gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo)
{

    // In per-process mode, ignore this sample if it does not belong to the target pid
    if (! _isTargetPid(pid))
    {
        return;
    }

    if ((selected_flag) == 0)
    {
        return;
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_ALL)
    {
        /* IBS all fetch samples (kills + attempts) */
        AGG_IBS_COUNT(DE_IBS_FETCH_ALL, count);
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_KILLED)
    {
        /* IBS killed fetches ("case 0") -- All interesting event
         * flags are clear */
        if (IBS_FETCH_KILLED(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_KILLED, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_ATTEMPTED)
    {
        /* Any non-killed fetch is an attempted fetch */
        if (!IBS_FETCH_KILLED(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_ATTEMPTED, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_COMPLETED)
    {
        if (!IBS_FETCH_KILLED(trans_fetch)
            &&   IBS_FETCH_FETCH_COMPLETION(trans_fetch))
            /* IBS Fetch Completed */
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_COMPLETED, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_ABORTED)
    {
        if (!IBS_FETCH_KILLED(trans_fetch)
            &&  !IBS_FETCH_FETCH_COMPLETION(trans_fetch))
            /* IBS Fetch Aborted */
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_ABORTED, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_L1_ITLB_HIT)
    {
        /* IBS L1 ITLB hit */
        if (IBS_FETCH_L1_TLB_HIT(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_L1_ITLB_HIT, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_ITLB_L1M_L2H)
    {
        /* IBS L1 ITLB miss and L2 ITLB hit */
        if (IBS_FETCH_ITLB_L1M_L2H(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_ITLB_L1M_L2H, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_ITLB_L1M_L2M)
    {
        /* IBS L1 & L2 ITLB miss; complete ITLB miss */
        if (IBS_FETCH_ITLB_L1M_L2M(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_ITLB_L1M_L2M, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_IC_MISS)
    {
        /* IBS instruction cache miss */
        if (IBS_FETCH_INST_CACHE_MISS(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_IC_MISS, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_IC_HIT)
    {
        /* IBS instruction cache hit */
        if (IBS_FETCH_INST_CACHE_HIT(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_IC_HIT, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_4K_PAGE)
    {
        if (IBS_FETCH_PHYS_ADDR_VALID(trans_fetch)
            && IBS_FETCH_TLB_PAGE_SIZE_4K(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_4K_PAGE, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_2M_PAGE)
    {
        if (IBS_FETCH_PHYS_ADDR_VALID(trans_fetch)
            && IBS_FETCH_TLB_PAGE_SIZE_2M(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_2M_PAGE, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_1G_PAGE)
    {
        if (IBS_FETCH_PHYS_ADDR_VALID(trans_fetch)
            && IBS_FETCH_TLB_PAGE_SIZE_1G(trans_fetch))
        {
            AGG_IBS_COUNT(DE_IBS_FETCH_1G_PAGE, count);
        }
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_XX_PAGE)
    {
    }

    CHECK_FETCH_SELECTED_FLAG(DE_IBS_FETCH_LATENCY)
    {
        if (IBS_FETCH_FETCH_LATENCY(trans_fetch))
            AGG_IBS_COUNT(DE_IBS_FETCH_LATENCY,
                          IBS_FETCH_FETCH_LATENCY(trans_fetch));
    }
}


/*
 * --------------------- OP DERIVED FUNCTION
 */
void CaPerfTranslator::trans_ibs_op(struct ibs_op_sample* trans_op,
                                    gtUInt32 selected_flag,
                                    CpuProfileProcess* pProc,
                                    gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                    gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                    gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo)
{
    // In per-process mode, ignore this sample if it does not belong to the target pid
    if (! _isTargetPid(pid))
    {
        return;
    }

    if ((selected_flag) == 0)
    {
        return;
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_OP_ALL)
    {
        /* All IBS op samples */
        AGG_IBS_COUNT(DE_IBS_OP_ALL, count);
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_OP_TAG_TO_RETIRE)
    {
        /* Tally retire cycle counts for all sampled macro-ops
         * IBS tag to retire cycles */
        if (IBS_OP_TAG_TO_RETIRE_CYCLES(trans_op))
            AGG_IBS_COUNT(DE_IBS_OP_TAG_TO_RETIRE,
                          IBS_OP_TAG_TO_RETIRE_CYCLES(trans_op));
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_OP_COMP_TO_RETIRE)
    {
        /* IBS completion to retire cycles */
        if (IBS_OP_COM_TO_RETIRE_CYCLES(trans_op))
            AGG_IBS_COUNT(DE_IBS_OP_COMP_TO_RETIRE,
                          IBS_OP_COM_TO_RETIRE_CYCLES(trans_op));
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_BRANCH_RETIRED)
    {
        if (IBS_OP_BRANCH_RETIRED(trans_op))
            /* IBS Branch retired op */
        {
            AGG_IBS_COUNT(DE_IBS_BRANCH_RETIRED, count) ;
        }
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_BRANCH_MISP)
    {
        if (IBS_OP_BRANCH_RETIRED(trans_op)
            /* Test branch-specific event flags */
            /* IBS mispredicted Branch op */
            && IBS_OP_BRANCH_MISPREDICT(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_BRANCH_MISP, count) ;
        }
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_BRANCH_TAKEN)
    {
        if (IBS_OP_BRANCH_RETIRED(trans_op)
            /* IBS taken Branch op */
            && IBS_OP_BRANCH_TAKEN(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_BRANCH_TAKEN, count);
        }
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_BRANCH_MISP_TAKEN)
    {
        if (IBS_OP_BRANCH_RETIRED(trans_op)
            /* IBS mispredicted taken branch op */
            && IBS_OP_BRANCH_TAKEN(trans_op)
            && IBS_OP_BRANCH_MISPREDICT(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_BRANCH_MISP_TAKEN, count);
        }
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_RETURN)
    {
        if (IBS_OP_BRANCH_RETIRED(trans_op)
            /* IBS return op */
            && IBS_OP_RETURN(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_RETURN, count);
        }
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_RETURN_MISP)
    {
        if (IBS_OP_BRANCH_RETIRED(trans_op)
            /* IBS mispredicted return op */
            && IBS_OP_RETURN(trans_op)
            && IBS_OP_BRANCH_MISPREDICT(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_RETURN_MISP, count);
        }
    }

    CHECK_OP_SELECTED_FLAG(DE_IBS_RESYNC)
    {
        /* Test for a resync macro-op */
        if (IBS_OP_BRANCH_RESYNC(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_RESYNC, count);
        }
    }
}


/*
 * --------------------- OP LS DERIVED FUNCTION
 */
void CaPerfTranslator::trans_ibs_op_ls(struct ibs_op_sample* trans_op,
                                       gtUInt32 selected_flag,
                                       CpuProfileProcess* pProc,
                                       gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                       gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                       gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo)
{
    // In per-process mode, ignore this sample if it does not belong to the target pid
    if (! _isTargetPid(pid))
    {
        return;
    }

    /* Preliminary check */
    if (!IBS_OP_IBS_LD_OP(trans_op) && !IBS_OP_IBS_ST_OP(trans_op))
    {
        return;
    }

    if ((selected_flag) == 0)
    {
        return;
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_ALL_OP)
    {
        /* Count the number of LS op samples */
        AGG_IBS_COUNT(DE_IBS_LS_ALL_OP, count) ;
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_LOAD_OP)
    {
        if (IBS_OP_IBS_LD_OP(trans_op))
            /* TALLy an IBS load derived event */
        {
            AGG_IBS_COUNT(DE_IBS_LS_LOAD_OP, count) ;
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_STORE_OP)
    {
        if (IBS_OP_IBS_ST_OP(trans_op))
            /* Count and handle store operations */
        {
            AGG_IBS_COUNT(DE_IBS_LS_STORE_OP, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_DTLB_L1H)
    {
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op))
            /* L1 DTLB hit -- This is the most frequent case */
        {
            AGG_IBS_COUNT(DE_IBS_LS_DTLB_L1H, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_DTLB_L1M_L2H)
    {
        /* l2_translation_size = 1 */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
            && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op))
            /* L1 DTLB miss, L2 DTLB hit */
        {
            AGG_IBS_COUNT(DE_IBS_LS_DTLB_L1M_L2H, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_DTLB_L1M_L2M)
    {
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
            && IBS_OP_IBS_DC_L2_TLB_MISS(trans_op))
            /* L1 DTLB miss, L2 DTLB miss */
        {
            AGG_IBS_COUNT(DE_IBS_LS_DTLB_L1M_L2M, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_DC_MISS)
    {
        if (IBS_OP_IBS_DC_MISS(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_DC_MISS, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_DC_HIT)
    {
        if (!IBS_OP_IBS_DC_MISS(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_DC_HIT, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_MISALIGNED)
    {
        if (IBS_OP_IBS_DC_MISS_ACC(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_MISALIGNED, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_BNK_CONF_LOAD)
    {
        if (IBS_OP_IBS_DC_LD_BNK_CON(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_BNK_CONF_LOAD, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_BNK_CONF_STORE)
    {
        if (IBS_OP_IBS_DC_ST_BNK_CON(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_BNK_CONF_STORE, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_STL_FORWARDED)
    {
        if (IBS_OP_IBS_LD_OP(trans_op)
            /* Data forwarding info are valid only for load ops */
            && IBS_OP_IBS_DC_ST_TO_LD_FWD(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_STL_FORWARDED, count) ;
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_STL_CANCELLED)
    {
        if (IBS_OP_IBS_LD_OP(trans_op))
            if (IBS_OP_IBS_DC_ST_TO_LD_CAN(trans_op))
            {
                AGG_IBS_COUNT(DE_IBS_LS_STL_CANCELLED, count) ;
            }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_UC_MEM_ACCESS)
    {
        if (IBS_OP_IBS_DC_UC_MEM_ACC(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_UC_MEM_ACCESS, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_WC_MEM_ACCESS)
    {
        if (IBS_OP_IBS_DC_WC_MEM_ACC(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_WC_MEM_ACCESS, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_LOCKED_OP)
    {
        if (IBS_OP_IBS_LOCKED_OP(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_LOCKED_OP, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_MAB_HIT)
    {
        if (IBS_OP_IBS_DC_MAB_HIT(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_LS_MAB_HIT, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L1_DTLB_4K)
    {
        /* l1_translation */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)

            && !IBS_OP_IBS_DC_L1_TLB_HIT_2MB(trans_op)
            && !IBS_OP_IBS_DC_L1_TLB_HIT_1GB(trans_op))
            /* This is the most common case, unfortunately */
        {
            AGG_IBS_COUNT(DE_IBS_LS_L1_DTLB_4K, count) ;
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L1_DTLB_2M)
    {
        /* l1_translation */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)

            && IBS_OP_IBS_DC_L1_TLB_HIT_2MB(trans_op))
            /* 2M L1 DTLB page translation */
        {
            AGG_IBS_COUNT(DE_IBS_LS_L1_DTLB_2M, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L1_DTLB_1G)
    {
        /* l1_translation */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)

            && !IBS_OP_IBS_DC_L1_TLB_HIT_2MB(trans_op)
            && IBS_OP_IBS_DC_L1_TLB_HIT_1GB(trans_op))
            /* 1G L1 DTLB page translation */
        {
            AGG_IBS_COUNT(DE_IBS_LS_L1_DTLB_1G, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L1_DTLB_RES)
    {
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L2_DTLB_4K)
    {
        /* l2_translation_size = 1 */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
            && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op)

            /* L2 DTLB page translation */
            && !IBS_OP_IBS_DC_L2_TLB_HIT_2MB(trans_op)
            && !IBS_OP_IBS_DC_L2_TLB_HIT_1GB(trans_op))
            /* 4K L2 DTLB page translation */
        {
            AGG_IBS_COUNT(DE_IBS_LS_L2_DTLB_4K, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L2_DTLB_2M)
    {
        /* l2_translation_size = 1 */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
            && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op)

            /* L2 DTLB page translation */
            && IBS_OP_IBS_DC_L2_TLB_HIT_2MB(trans_op)
            && !IBS_OP_IBS_DC_L2_TLB_HIT_1GB(trans_op))
            /* 2M L2 DTLB page translation */
        {
            AGG_IBS_COUNT(DE_IBS_LS_L2_DTLB_2M, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L2_DTLB_1G)
    {
        /* l2_translation_size = 1 */
        if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
            && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
            && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op)

            /* L2 DTLB page translation */
            && !IBS_OP_IBS_DC_L2_TLB_HIT_2MB(trans_op)
            && IBS_OP_IBS_DC_L2_TLB_HIT_1GB(trans_op))
            /* 2M L2 DTLB page translation */
        {
            AGG_IBS_COUNT(DE_IBS_LS_L2_DTLB_1G, count);
        }
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_L2_DTLB_RES2)
    {
    }

    CHECK_OP_LS_SELECTED_FLAG(DE_IBS_LS_DC_LOAD_LAT)
    {
        if (IBS_OP_IBS_LD_OP(trans_op)
            /* If the load missed in DC, tally the DC load miss latency */
            && IBS_OP_IBS_DC_MISS(trans_op))
            /* DC load miss latency is only reliable for load ops */
            AGG_IBS_COUNT(DE_IBS_LS_DC_LOAD_LAT,
                          IBS_OP_DC_MISS_LATENCY(trans_op)) ;
    }
}

/*
 * --------------------- OP NB DERIVED FUNCTION
 *
 * NB data is only guaranteed reliable for load operations
 * that miss in L1 and L2 cache. NB data arrives too late
 * to be reliable for store operations
 */
void CaPerfTranslator::trans_ibs_op_nb(struct ibs_op_sample* trans_op,
                                       gtUInt32 selected_flag,
                                       CpuProfileProcess* pProc,
                                       gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                       gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                       gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo)
{
    // In per-process mode, ignore this sample if it does not belong to the target pid
    if (! _isTargetPid(pid))
    {
        return;
    }

    /* Preliminary check */
    if ((selected_flag) == 0)
    {
        return;
    }

    if (!IBS_OP_IBS_LD_OP(trans_op))
    {
        return;
    }

    if (!IBS_OP_IBS_DC_MISS(trans_op))
    {
        return;
    }

    if (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0)
    {
        return;
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_LOCAL)
    {
        if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
            /* Request was serviced by local processor */
        {
            AGG_IBS_COUNT(DE_IBS_NB_LOCAL, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_REMOTE)
    {
        if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
            /* Request was serviced by remote processor */
        {
            AGG_IBS_COUNT(DE_IBS_NB_REMOTE, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_LOCAL_L3)
    {
        if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_01(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_LOCAL_L3, count);
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_LOCAL_CACHE)
    {
        if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_02(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_LOCAL_CACHE, count);
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_REMOTE_CACHE)
    {
        if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_02(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_REMOTE_CACHE, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_LOCAL_DRAM)
    {
        if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_03(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_LOCAL_DRAM, count);
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_REMOTE_DRAM)
    {
        if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_03(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_REMOTE_DRAM, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_LOCAL_OTHER)
    {
        if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_07(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_LOCAL_OTHER, count);
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_REMOTE_OTHER)
    {
        if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
            &&  IBS_OP_NB_IBS_REQ_SRC_07(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_REMOTE_OTHER, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_CACHE_STATE_M)
    {
        if (IBS_OP_NB_IBS_REQ_SRC_02(trans_op)
            && !IBS_OP_NB_IBS_CACHE_HIT_ST(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_CACHE_STATE_M, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_CACHE_STATE_O)
    {
        if (IBS_OP_NB_IBS_REQ_SRC_02(trans_op)
            && IBS_OP_NB_IBS_CACHE_HIT_ST(trans_op))
        {
            AGG_IBS_COUNT(DE_IBS_NB_CACHE_STATE_O, count) ;
        }
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_LOCAL_LATENCY)
    {
        if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
            /* Request was serviced by local processor */
            AGG_IBS_COUNT(DE_IBS_NB_LOCAL_LATENCY,
                          IBS_OP_DC_MISS_LATENCY(trans_op));
    }

    CHECK_OP_NB_SELECTED_FLAG(DE_IBS_NB_REMOTE_LATENCY)
    {
        if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
            /* Request was serviced by remote processor */
            AGG_IBS_COUNT(DE_IBS_NB_REMOTE_LATENCY,
                          IBS_OP_DC_MISS_LATENCY(trans_op));
    }
}


int CaPerfTranslator::trans_ibs_op_rip_invalid(struct ibs_op_sample* trans_op)
{
    if (IBS_OP_RIP_INVALID(trans_op))
    {
        return 1;
    }

    return 0;
}


void CaPerfTranslator::trans_ibs_op_mask_reserved(gtUInt32 family, struct ibs_op_sample* trans_op)
{
    switch (family)
    {
        case 0x10:
            /* Reserved IbsRipInvalid (MSRC001_1035[38])*/
            trans_op->ibs_op_data1_high &= ~RIP_INVALID;
            break;

        case 0x12:
            /* Reserved NbIbsReqDstProc (MSRCC001_1036[4]) */
            trans_op->ibs_op_data2_low &= ~NB_MASK_REQ_DST_PROC;
            /* Reserved NbIbsReqCacheHitSt (MSRCC001_1036[5]) */
            trans_op->ibs_op_data2_low &= ~NB_MASK_L3_STATE;
            break;

        case 0x14:
            /* Reserved NbIbsReqDstProc (MSRCC001_1036[4]) */
            trans_op->ibs_op_data2_low &= ~NB_MASK_REQ_DST_PROC;
            /* Reserved NbIbsReqCacheHitSt (MSRCC001_1036[5]) */
            trans_op->ibs_op_data2_low &= ~NB_MASK_L3_STATE;
            /* Reserved IbsDcL1tlbHit1G (MSRC001_1037[5]) */
            trans_op->ibs_op_data3_low &= ~DC_MASK_L1_HIT_1G;
            /* Reserved IbsDcLdBnkCon (MSRC001_1037[9]) */
            trans_op->ibs_op_data3_low &= ~DC_MASK_LD_BANK_CONFLICT;
            /* Reserved IbsDcStBnkCon (MSRC001_1037[10]) */
            trans_op->ibs_op_data3_low &= ~DC_MASK_ST_BANK_CONFLICT;
            /* Reserved IbsDcStToLdCan (MSRC001_1037[12]) */
            trans_op->ibs_op_data3_low &= ~DC_MASK_ST_TO_LD_CANCEL;
            /* Reserved IbsDcL2tlbHit1G (MSRC001_1037[19]) */
            trans_op->ibs_op_data3_low &= ~DC_MASK_L2_HIT_1G;

            break;

        case 0x15:
        default:
            break;

    }
}

#ifdef  HAS_BTA
// EXPERIMENT: BTA
extern struct calog* bta_log;
void CaPerfTranslator::trans_ibs_op_bta(struct transient* trans)
{
    calog_data entry;
    bta_data data;
    static cookie_t old_cookie     = NO_COOKIE;
    static cookie_t old_app_cookie = NO_COOKIE;
    static char const* mod        = NULL;
    static char const* app        = NULL;
    const char vmlinux[10]         = "vmlinux";
    struct ibs_op_sample* trans_op = ((struct ibs_sample*)(trans->ext))->op;

    if (!bta_log)
    {
        return;
    }

    if (!trans_op->ibs_op_brtgt_addr)
    {
        return;
    }

    if (old_app_cookie == INVALID_COOKIE || old_app_cookie == NO_COOKIE || old_app_cookie != trans->app_cookie)
    {
        app = find_cookie(trans->app_cookie);
        old_app_cookie = trans->cookie;
    }

    if (trans->in_kernel == 1)
    {
        mod = vmlinux;
        old_cookie = NO_COOKIE;
    }
    else
    {
        if (old_cookie == INVALID_COOKIE || old_cookie == NO_COOKIE || old_cookie != trans->cookie)
        {
            mod = find_cookie(trans->cookie);
            old_cookie = trans->cookie;
        }
    }

    /* Setup entry */
    entry.cookie     = trans->cookie;
    entry.app_cookie = trans->app_cookie;
    entry.cpu        = trans->cpu;
    entry.tgid       = trans->tgid;
    entry.tid        = trans->tid;
    entry.offset     = trans->pc;
    entry.cnt        = 1;

    /* Setup data */
    data.bta = trans_op->ibs_op_brtgt_addr;

    calog_add_data(bta_log, &entry, &data, sizeof(bta_data), mod, app);
}
#endif // HAS_BTA


#ifdef  HAS_DCMISS
// EXPERIMENT: DCMISS
extern struct calog* dcmiss_log;
void CaPerfTranslator::trans_ibs_op_ls_dcmiss(struct transient* trans)
{
    calog_data entry;
    dcmiss_data data;
    static cookie_t old_cookie     = NO_COOKIE;
    static cookie_t old_app_cookie = NO_COOKIE;
    static char const* mod        = NULL;
    static char const* app        = NULL;
    const char vmlinux[10]         = "vmlinux";
    struct ibs_op_sample* trans_op = ((struct ibs_sample*)(trans->ext))->op;

    if (!dcmiss_log)
    {
        return;
    }

    if (old_app_cookie == INVALID_COOKIE || old_app_cookie == NO_COOKIE || old_app_cookie != trans->app_cookie)
    {
        app = find_cookie(trans->app_cookie);
        old_app_cookie = trans->cookie;
    }

    if (trans->in_kernel == 1)
    {
        mod = vmlinux;
        old_cookie = NO_COOKIE;
    }
    else
    {
        if (old_cookie == INVALID_COOKIE || old_cookie == NO_COOKIE || old_cookie != trans->cookie)
        {
            mod = find_cookie(trans->cookie);
            old_cookie = trans->cookie;
        }
    }

    /* Setup entry */
    entry.cookie     = trans->cookie;
    entry.app_cookie = trans->app_cookie;
    entry.cpu        = trans->cpu;
    entry.tgid       = trans->tgid;
    entry.tid        = trans->tid;
    entry.offset     = trans->pc;
    entry.cnt        = 1;

    /* Setup data */
    data.phy = trans_op->ibs_op_phys_addr_high;
    data.phy = (data.phy << 32) | trans_op->ibs_op_phys_addr_low;
    data.lin = trans_op->ibs_op_ldst_linaddr_high;
    data.lin = (data.lin << 32) | trans_op->ibs_op_ldst_linaddr_low;
    data.ld  = IBS_OP_IBS_LD_OP(trans_op);
    data.lat = (data.ld) ? IBS_OP_DC_MISS_LATENCY(trans_op) : 0;

    calog_add_data(dcmiss_log, &entry, &data, sizeof(dcmiss_data), mod, app);
}
#endif // HAS_DCMISS
