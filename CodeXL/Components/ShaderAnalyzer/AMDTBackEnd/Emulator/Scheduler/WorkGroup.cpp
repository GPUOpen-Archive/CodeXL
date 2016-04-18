//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   WorkGroup.cpp
/// \author GPU Developer Tools
/// \version $Revision: #2 $
/// \brief Description: The work group class.
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/KernelAnalyzer/AMDTKernelAnalyzer/src/Emulator/Scheduler/WorkGroup.cpp#2 $
// Last checkin:   $DateTime: 2013/11/20 07:16:06 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 482993 $
//=============================================================

/// Local:
#include "WorkGroup.h"

WorkGroup::WorkGroup(size_t wavefrontNum, const std::vector<Instruction*>* pInstructions): m_waveFrontNum(wavefrontNum), m_isScheduled(false), m_isScheduleFinished(false), m_pInstructions(pInstructions)
{
    m_wavefront.assign(wavefrontNum, WaveFront(pInstructions));
}
bool
WorkGroup::IsScheduled() const
{
    return m_isScheduled;
}

bool
WorkGroup::IsScheduleFinished() const
{
    return m_isScheduleFinished;
}

size_t
WorkGroup::GetCU() const
{
    return m_cu;
}

size_t
WorkGroup::GetWaveFrontNum() const
{
    return m_waveFrontNum;
}

WaveFront&
WorkGroup::GetWaveFront(size_t wf)
{
    return m_wavefront[wf];
}

Instruction*
WorkGroup::GetInstruction(size_t instIdx)
{
    if (m_pInstructions->size() <= instIdx)
    {
        return NULL;
    }

    return (*m_pInstructions)[instIdx];
}

void
WorkGroup::SetScheduled()
{
    m_isScheduled = true;
}

void
WorkGroup::SetScheduleFinished()
{
    m_isScheduleFinished = true;
}
