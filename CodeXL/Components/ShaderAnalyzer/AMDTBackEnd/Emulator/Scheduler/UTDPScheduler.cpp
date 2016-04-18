//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   UTDPScheduler.cpp
/// \author GPU Developer Tools
/// \version $Revision: #3 $
/// \brief Description: The ultra-threaded dispatch processor scheduler class.
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/KernelAnalyzer/AMDTKernelAnalyzer/src/Emulator/Scheduler/UTDPScheduler.cpp#3 $
// Last checkin:   $DateTime: 2014/01/08 09:22:31 $
// Last edited by: $Author: delifaz $
// Change list:    $Change: 485956 $
//=============================================================

/// Local:

#include <numeric>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include "UTDPScheduler.h"

const size_t UTDPScheduler::NumCUTahiti = 32;
const size_t UTDPScheduler::NumCUCapeVerde = 10;
const size_t UTDPScheduler::NumCUPitcrain = 16;

const size_t UTDPScheduler::DeviceMaxWorkGoupSizeTahiti  = 256;
const size_t UTDPScheduler::DeviceMaxWorkGoupSizeCapeVerde  = 256;
const size_t UTDPScheduler::DeviceMaxWorkGoupSizePitcrain  = 256;

const size_t UTDPScheduler::WaveFrontWINumTahiti  = 64;
const size_t UTDPScheduler::WaveFrontWINumCapeVerde  = 64;
const size_t UTDPScheduler::WaveFrontWINumPitcrain  = 64;

const size_t UTDPScheduler::ClkTahiti  = 1;
const size_t UTDPScheduler::ClkCapeVerde  = 1;
const size_t UTDPScheduler::ClkPitcrain  = 1;

bool
UTDPScheduler::IsWorkDimValid()
{
    if (m_workDim <= 3 && m_workDim >= 1)
    {
        return true;
    }

    return false;
}

bool
UTDPScheduler::IsGlobalWorkSizeValid()
{
    if (m_workGroupSize > m_maxWorkGoupSize)
    {
        return false;
    }

    return true;
}

bool
UTDPScheduler::IsLocalWorkSizeValid()
{
    if (m_globalWorkSize.size() != m_workDim || m_localWorkSize.size() != m_workDim)
    {
        return false;
    }

    for (size_t dim = 0 ; dim < m_workDim ; ++dim)
    {
        if (m_globalWorkSize[dim] % (m_localWorkSize[dim] == 0 ? 1 : m_localWorkSize[dim]) != 0)
        {
            return false;
        }
    }

    return true;
}

size_t
UTDPScheduler::GetExeTime(size_t exeClkNum) const
{
    return exeClkNum * m_clkToSec;
}

UTDPScheduler::UTDPScheduler(const std::vector<Instruction*>& instructions, DeviceType deviceType, size_t workDim, const std::vector<size_t>& globalWorkSize, const std::vector<size_t>& localWorkSize, double branchRate)
    : m_deviceType(deviceType), m_workDim(workDim), m_globalWorkSize(globalWorkSize), m_localWorkSize(localWorkSize), m_branchRate(branchRate), m_wavefronts(0)
{
    switch (m_deviceType)
    {
        case Tahiti:
            m_numCU = NumCUTahiti;
            m_clkToSec = ClkTahiti;
            m_maxWorkGoupSize = DeviceMaxWorkGoupSizeTahiti;
            m_waveFrontSize = WaveFrontWINumTahiti;
            break;

        case CapeVerde:
            m_numCU = NumCUCapeVerde;
            m_clkToSec = ClkCapeVerde;
            m_maxWorkGoupSize = DeviceMaxWorkGoupSizeCapeVerde;
            m_waveFrontSize = WaveFrontWINumCapeVerde;
            break;

        case Pitcrain:
            m_numCU = NumCUPitcrain;
            m_clkToSec = ClkPitcrain;
            m_maxWorkGoupSize = DeviceMaxWorkGoupSizePitcrain;
            m_waveFrontSize = WaveFrontWINumPitcrain;
            break;
    }

    m_vCUScheduler.assign(m_numCU, CUScheduler(branchRate, &instructions));
    m_workGroupSize = std::accumulate(m_localWorkSize.begin(), m_localWorkSize.end(), 1, m_mul);
    size_t globalTotalWorkSize = std::accumulate(m_globalWorkSize.begin(), m_globalWorkSize.end(), 1, m_mul);
    m_workGroupNum = globalTotalWorkSize / m_workGroupSize;

    /// Always the number of executed work-tems in lock-step is equal to m_waveFrontSize
    /// if m_workGroupSize is not evenly devided by m_waveFrontSize pad the wavefront by "nop" work-items.
    m_waveFrontNum = m_workGroupSize / m_waveFrontSize + m_workGroupSize % m_waveFrontSize;
    m_workGroups.assign(m_workGroupNum, WorkGroup(m_waveFrontNum, &instructions));
}

void
UTDPScheduler::Schedule(size_t workGroup, bool& isScheduleProgress)
{
    size_t cu = workGroup % m_numCU;
    m_vCUScheduler[cu].ScheduleWG(m_workGroups[workGroup], isScheduleProgress);
}

UTDPScheduler::StatusSchedule
UTDPScheduler::Schedule(size_t& exeClkNum)
{
    if (!IsWorkDimValid())
    {
        return Status_ScheduleInvalidWorkDim;
    }
    else if (!IsGlobalWorkSizeValid() || !IsLocalWorkSizeValid())
    {
        return Status_ScheduleInvalidWorkGroupSize;
    }

    bool isScheduleProgress = true;

    while (isScheduleProgress)
    {
        isScheduleProgress = false;
        bool isWorkGroupScheduleProgress = true;

        /// Execute an instruction from each work-group.
        for (size_t workGroup = 0; workGroup < m_workGroupNum; ++workGroup)
        {
            Schedule(workGroup, isWorkGroupScheduleProgress);

            if (isWorkGroupScheduleProgress)
            {
                isScheduleProgress = true;
            }
        }
    }

    exeClkNum = 0;

    /// The program execution time is a miximum execution time of compute units
    for (size_t numCU = 0; numCU < m_numCU; ++numCU)
    {
        size_t cuClkNum = m_vCUScheduler[numCU].GetClkNum();

        if (exeClkNum == 0 || cuClkNum > exeClkNum)
        {
            exeClkNum = cuClkNum;
        }
    }

    return Status_ScheduleSuccess;
}

bool UTDPScheduler::IsBranchTaken(unsigned int itNumber, double branchRate, bool isForward)
{
    if (isForward)
    {
        branchRate /= 5;
    }

    branchRate /= std::sqrt((double)itNumber);
    double pRand = (double)rand() / (RAND_MAX);
    return pRand > (1 - branchRate);
}

void UTDPScheduler::ResetInstructionsCounters()
{
    m_instructionCounters.m_scalarMemoryReadInstCount = 0;
    m_instructionCounters.m_scalarALUInstCount = 0;
    m_instructionCounters.m_vectorMemoryReadInstCount = 0;
    m_instructionCounters.m_vectorMemoryWriteInstCount = 0;
    m_instructionCounters.m_vectorALUInstCount = 0;
    m_instructionCounters.m_LDSInstCount = 0;
    m_instructionCounters.m_GDSInstCount = 0;
    m_instructionCounters.m_exportInstCount = 0;
    m_instructionCounters.m_atomicsInstCount = 0;
    m_instructionCounters.m_branchInstCount = 0;
}

void
UTDPScheduler::UpdateInstructionCounters(const std::vector<Instruction*>& instructions, double branchRate)
{
    ResetInstructionsCounters();
    srand((unsigned int)time(NULL));

    std::vector<Instruction*> updInstructions;

    // Insert the "32 bit representation" for each 32bit instruction "as is"
    // For each 64 bit instruction insert its "32 bit representation" and NULL (for the seconds 32 bit)
    for (std::vector<Instruction*>::const_iterator itInstruction = instructions.begin(); itInstruction != instructions.end(); ++itInstruction)
    {
        if ((*itInstruction)->GetInstructionWidth() == 32)
        {
            updInstructions.push_back(*itInstruction);
        }
        else
        {
            updInstructions.push_back(*itInstruction);
            updInstructions.push_back(NULL);
        }
    }

    int pc = 0;
    std::vector<unsigned int> vBranchIteration(updInstructions.size(), 1);

    int updIstructionSize = (int)updInstructions.size();

    for (int nInstruction = 0 ; nInstruction < updIstructionSize ; nInstruction++)
    {
        Instruction* pCurrentInstruction = updInstructions[nInstruction];

        if (NULL == pCurrentInstruction)
        {
            pc++;
            continue;
        }

        Instruction::InstructionCategory instructionFormatKind = (pCurrentInstruction)->GetInstructionCategory();

        switch (instructionFormatKind)
        {
            case Instruction::ScalarMemoryRead:
                m_instructionCounters.m_scalarMemoryReadInstCount++;
                break;

            case Instruction::ScalarMemoryWrite:
                m_instructionCounters.m_scalarMemoryWriteInstCount++;
                break;

            case Instruction::VectorMemoryRead:
                m_instructionCounters.m_vectorMemoryReadInstCount++;
                break;

            case Instruction::VectorMemoryWrite:
                m_instructionCounters.m_vectorMemoryWriteInstCount++;
                break;

            case Instruction::VectorALU:
                m_instructionCounters.m_vectorALUInstCount++;
                break;

            case Instruction::LDS:
                m_instructionCounters.m_LDSInstCount++;
                break;

            case Instruction::GDS:
                m_instructionCounters.m_GDSInstCount++;
                break;

            case Instruction::Export:
                m_instructionCounters.m_exportInstCount++;
                break;

            case Instruction::Atomics:
                m_instructionCounters.m_atomicsInstCount++;
                break;

            case Instruction::ScalarALU:
            {
                Instruction::InstructionSet instructionFormat = pCurrentInstruction->GetInstructionFormat();

                if (instructionFormat == Instruction::InstructionSet_SOPP)
                {
                    SOPPInstruction* pSoppInstruction = static_cast<SOPPInstruction*>(pCurrentInstruction);
                    SOPPInstruction::OP op = pSoppInstruction->GetOp();
                    SOPPInstruction::SIMM16 simm16 = pSoppInstruction->GetSIMM16();

                    if (((op >= SOPPInstruction::S_CBRANCH_SCC0 && op <= SOPPInstruction::S_CBRANCH_EXECNZ) && IsBranchTaken(vBranchIteration[pc], branchRate, simm16 > 0)) ||
                        op == SOPPInstruction::S_BRANCH)
                    {
                        vBranchIteration[pc]++;
                        m_instructionCounters.m_branchInstCount++;
                        int newInstructionPos = nInstruction + simm16;

                        // Do not allow going backward. might point to some problem, in that case move one instruction forward.
                        if (newInstructionPos < nInstruction)
                        {
                            nInstruction++;
                            pc++;
                        }
                        else
                        {
                            nInstruction = newInstructionPos;
                            pc += simm16;
                        }
                    }
                }
                else
                {
                    m_instructionCounters.m_scalarALUInstCount++;
                }
            }
            break;

            default:
                break;
        }

        pc++;
    }
}

void UTDPScheduler::UpdateWavefrontsNum(unsigned int waveFrontWINum, int* globalWorkSize, int* localWorkSize)
{
    double globalSize = 1, localSize = 1;

    for (unsigned int i = 0; i < 3; ++i)
    {
        globalSize *= (double)(globalWorkSize[i] == 0 ? 1 : globalWorkSize[i]);
        localSize *= (double)(localWorkSize[i] == 0 ? 1 : localWorkSize[i]);
    }

    // The total number of wavefronts executing a kernel =
    // total number of complete workgroups * the number of wavefronts executing each workgroup +
    // the number of wavefronts executing the rest of workitems.
    m_wavefronts = (unsigned int)(std::floor(globalSize / localSize) * std::ceil(localSize / double(waveFrontWINum)) +
                                  std::ceil((double)((unsigned int)globalSize % (unsigned int)localSize) / (double)waveFrontWINum));

}
