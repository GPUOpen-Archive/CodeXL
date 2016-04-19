//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __CUSCHEDULER_H
#define __CUSCHEDULER_H

/// -----------------------------------------------------------------------------------------------
/// \class Name:
/// \brief Description:  Compute Unit Scheduler class.
/// -----------------------------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include "../Parser/Instruction.h"
#include "BranchUnitScheduler.h"
#include "WorkGroup.h"
#include "WaveFront.h"


/// -----------------------------------------------------------------------------------------------
/// \class Name:
/// \brief Description:  Compute Unit Scheduler
/// -----------------------------------------------------------------------------------------------

class CUScheduler
{
public:
    /// Compute Unit execution status
    enum Status_ComputeUnitExe
    {
        Status_CUExeInvalidWorkGroupCU,
        Status_CUExeInvalidInst,
        Status_CUExeSuccess
    };
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        CUScheduler
    /// \brief Description:
    /// \param[in]          m_vectorUnitNextFreeClk(0)
    /// \param[in]          m_scalarUnitNextFreeClk(0)
    /// \param[in]          m_branchUnitNextFreeClk(0
    /// \return
    /// -----------------------------------------------------------------------------------------------
    CUScheduler(double branchRate, const std::vector<Instruction*>* pInstructions);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ~CUScheduler
    /// \brief Description:
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ~CUScheduler() {}
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ScheduleWF
    /// \brief Description: schedule wavefront insife work group
    /// \param[in]          workGroup
    /// \param[in]          wf
    /// \param[in]          isScheduleProgress
    /// \return Status_ComputeUnitExe
    /// -----------------------------------------------------------------------------------------------
    Status_ComputeUnitExe ScheduleWF(WorkGroup& workGroup, size_t wf, bool& isScheduleProgress);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ScheduleWG
    /// \brief Description: Schedule work group
    /// \param[in]          workGroup
    /// \param[in]          isScheduleProgress
    /// \return Status_ComputeUnitExe
    /// -----------------------------------------------------------------------------------------------
    Status_ComputeUnitExe ScheduleWG(WorkGroup& workGroup, bool& isScheduleProgress);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetClkNum
    /// \brief Description: Get the clock number for program execution
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetClkNum() const;
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetCU
    /// \brief Description: Get the compute unit index
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetCU() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ResetCUScheduler
    /// \brief Description: Reset compute unit execution. (clocks m_vectorUnitNextFreeClk,m_scalarUnitNextFreeClk and m_branchUnitNextFreeClk are updated to be zero)
    /// \return void
    /// -----------------------------------------------------------------------------------------------
    void ResetCUScheduler();
private:
    /// The compute unit index
    size_t m_cu;

    /// The next free clock for vector instruction execution.
    /// (The amount of clock during which vector instructions were executed)
    size_t m_vectorUnitNextFreeClk;

    /// The next free clock for scalar instruction execution.
    /// (The amount of clock during which vector instructions were executed)
    size_t m_scalarUnitNextFreeClk;

    /// The next free clock for branch instruction execution.
    /// (The amount of clock during which scalar instructions were executed)
    size_t m_branchUnitNextFreeClk;

    /// The branch unit scheduler
    BranchUnitScheduler m_branchUnitScheduler;
};


#endif //__CUSCHEDULER_H

