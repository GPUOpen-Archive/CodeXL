//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   BranchUnitScheduler.cpp
/// \author GPU Developer Tools
/// \version $Revision: #3 $
/// \brief Description: Branch Unit Scheduler class
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/KernelAnalyzer/AMDTKernelAnalyzer/src/Emulator/Scheduler/BranchUnitScheduler.cpp#3 $
// Last checkin:   $DateTime: 2014/01/08 09:22:31 $
// Last edited by: $Author: delifaz $
// Change list:    $Change: 485956 $
//=============================================================

/// Local:
#include <algorithm>
#include <math.h>
#include "BranchUnitScheduler.h"

size_t BranchUnitScheduler::GetBranchInstructionsNum(const std::vector<Instruction*>& instructions)
{
    for (const Instruction*& iter : instructions)
    {
        if (IsInstructionBranch(iter))
        {
            m_branchInstNum++;
        }
    }

    return m_branchInstNum;
}

bool BranchUnitScheduler::IsInstructionBranch(const Instruction* inst) const
{
    bool isInstruction = false;

    if (inst != nullptr)
    {
        if (inst->GetInstructionCategory() == Instruction::ScalarALU)
        {
            if (inst->GetInstructionFormat() == Instruction::InstructionSet_SOPP)
            {
                SOPPInstruction* pSOPPInstruction = dynamic_cast<SOPPInstruction*>(inst);

                if (pSOPPInstruction != nullptr)
                {
                    SOPPInstruction::OP opSOPP = pSOPPInstruction->GetOp();

                    switch (opSOPP)
                    {
                        case SOPPInstruction::S_BRANCH:
                        case SOPPInstruction::S_CBRANCH_SCC0:
                        case SOPPInstruction::S_CBRANCH_SCC1:
                        case SOPPInstruction::S_CBRANCH_VCCZ:
                        case SOPPInstruction::S_CBRANCH_VCCNZ:
                        case SOPPInstruction::S_CBRANCH_EXECZ:
                        case SOPPInstruction::S_CBRANCH_EXECNZ:
                            isInstruction = true;
                            break;
                    }
                }
            }
            else if (inst->GetInstructionFormat() == Instruction::InstructionSet_SOP1)
            {
                SOP1Instruction* pSOP1Instruction = dynamic_cast<SOP1Instruction*>(inst);

                if (pSOP1Instruction != nullptr)
                {
                    SOP1Instruction::OP opSOP1 = pSOP1Instruction->GetOp();

                    switch (opSOP1)
                    {
                        case SOP1Instruction::S_CBRANCH_JOIN:
                        case SOP1Instruction::S_SETPC_B64:
                        case SOP1Instruction::S_SWAPPC_B64:
                        case SOP1Instruction::S_GETPC_B64:
                        {
                            isInstruction = true;
                        }
                        break;
                    }
                }
                else if (inst->GetInstructionFormat() == Instruction::InstructionSet_SOP2)
                {
                    SOP2Instruction* pSOP2Instruction = dynamic_cast<SOP2Instruction*>(inst);

                    if (pSOP2Instruction != nullptr)
                    {
                        SOP2Instruction::OP opSOP2 = pSOP2Instruction->GetOp();

                        if (opSOP2 == SOP2Instruction::S_CBRANCH_G_FORK)
                        {
                            isInstruction = true;
                        }
                    }
                }
                else if (inst->GetInstructionFormat() == Instruction::InstructionSet_SOPK)
                {
                    SOPKInstruction* pSOPKInstruction = dynamic_cast<SOPKInstruction*>(inst);

                    if (pSOPKInstruction != nullptr)
                    {
                        SOPKInstruction::OP opSOPK = pSOPKInstruction->GetOp();

                        if (opSOPK == SOPKInstruction::S_CBRANCH_I_FORK)
                        {
                            isInstruction = true;
                        }
                    }
                }
                else if (inst->GetInstructionFormat() == Instruction::InstructionSet_SOPC)
                {
                    SOPCInstruction* pSOPCInstruction = dynamic_cast<SOPCInstruction*>(inst);

                    if (pSOPCInstruction != nullptr)
                    {
                        SOPCInstruction::OP opSOPC = pSOPCInstruction->GetOp();

                        if (opSOPC == OPCInstruction::S_SETVSKIP)
                        {
                            isInstruction = true;
                        }
                    }
                }
            }
        }

        return isInstruction;
    }

    bool BranchUnitScheduler::IsBranchTaken(size_t branchInstIdx)const
    {
        /// If the branch instruction index is in range of brach instructions return its branch prediction.
        if (branchInstIdx < m_branchPredictor.size())
        {
            return m_branchPredictor[branchInstIdx];
        }

        /// If the branch instruction index is not in range of brach instructions the instruction should be taken (as non-branch)
        /// (Should not get here)
        return true;
    }

    void BranchUnitScheduler::SetUpBranchUnitScheduler()
    {
        /// Predicted as taken branches number
        size_t predictedBranches = (size_t)(m_branchInstNum * m_branchTakenRate);
        m_branchPredictor.resize(predictedBranches, true);
        m_branchPredictor.resize(m_branchInstNum, false);
        /// Shuffle in random order taken and non-taken branch instructions predicators
        std::random_shuffle(m_branchPredictor.begin(), m_branchPredictor.end());
    }
