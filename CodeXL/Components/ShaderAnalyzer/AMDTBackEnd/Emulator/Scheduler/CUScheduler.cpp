//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   CUScheduler.cpp
/// \author GPU Developer Tools
/// \version $Revision: #2 $
/// \brief Description: Compute Unit Scheduler class.
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/KernelAnalyzer/AMDTKernelAnalyzer/src/Emulator/Scheduler/CUScheduler.cpp#2 $
// Last checkin:   $DateTime: 2013/11/20 07:16:06 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 482993 $
//=============================================================

/// Local:
#include "CUScheduler.h"

CUScheduler::CUScheduler(double branchRate, const std::vector<Instruction*>* pInstructions) : m_vectorUnitNextFreeClk(0), m_scalarUnitNextFreeClk(0), m_branchUnitNextFreeClk(0), m_branchUnitScheduler(branchRate)
{
    if (m_branchUnitScheduler.GetBranchInstructionsNum(pInstructions) > 0)
    {
        m_branchUnitScheduler.SetUpBranchUnitScheduler();
    }
}
size_t
CUScheduler::GetClkNum() const
{
    return std::max(std::max(m_vectorUnitNextFreeClk, m_scalarUnitNextFreeClk), m_branchUnitNextFreeClk);
}

size_t
CUScheduler::GetCU() const
{
    return m_cu;
}

CUScheduler::Status_ComputeUnitExe
CUScheduler::ScheduleWF(WorkGroup& workGroup, size_t wf, bool& isScheduleProgress)
{
    isScheduleProgress = true;
    WaveFront& waveFront = workGroup.GetWaveFront(wf);
    size_t instIdx = waveFront.GetNextInstIdx();
    Instruction* pInst = workGroup.GetInstruction(instIdx);

    if (pInst == NULL)
    {
        isScheduleProgress = false;
        return Status_CUExeSuccess;
    }

    Instruction::InstructionCategory instFormatKind = pInst->GetInstructionCategory();

    if (m_branchUnitScheduler.IsInstructionBranch(pInst))
    {
        size_t branchInstIdx = waveFront.GetNextBranchInstIdx();

        if (m_branchUnitScheduler.IsBranchTaken(branchInstIdx))
        {
            m_branchUnitNextFreeClk++;
        }
    }
    else if (instFormatKind == Instruction::ScalarMemoryRead || instFormatKind == Instruction::ScalarMemoryWrite || instFormatKind == Instruction::ScalarALU)
    {
        m_scalarUnitNextFreeClk++;
    }
    else
    {
        m_vectorUnitNextFreeClk++;
    }

    return Status_CUExeSuccess;
}

CUScheduler::Status_ComputeUnitExe
CUScheduler::ScheduleWG(WorkGroup& workGroup, bool& isScheduleProgress)
{
    isScheduleProgress = false;

    /// Once the work group was scheduled to be executed on specific compute unit, it should be executed only on it.
    if (workGroup.IsScheduled() && workGroup.GetCU() != m_cu)
    {
        return Status_CUExeInvalidWorkGroupCU;
    }

    if (!workGroup.IsScheduled())
    {
        workGroup.SetScheduled();
    }

    if (workGroup.IsScheduleFinished())
    {
        return Status_CUExeSuccess;
    }

    /// Excute an instruction from each wavefront
    size_t waveFrontNum = workGroup.GetWaveFrontNum();
    bool isScheduleWFProgress;

    for (size_t wf = 0; wf < waveFrontNum; ++wf)
    {
        ScheduleWF(workGroup, wf, isScheduleWFProgress);
        isScheduleProgress |= isScheduleWFProgress;
    }

    return Status_CUExeSuccess;
}
