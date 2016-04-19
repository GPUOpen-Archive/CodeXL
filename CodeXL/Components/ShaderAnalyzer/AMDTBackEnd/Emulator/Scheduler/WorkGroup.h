//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __WORKGROUP_H
#define __WORKGROUP_H

#include "../Parser/Instruction.h"
#include <vector>
#include "WaveFront.h"
/// -----------------------------------------------------------------------------------------------
/// \class Name: WorkGroup
/// \brief Description: The work Group.
/// -----------------------------------------------------------------------------------------------

class WorkGroup
{
public:

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        WorkGroup
    /// \brief Description: c`tor
    /// \param[in]          wavefrontNum
    /// \param[in]          waveFrontSize):m_waveFrontNum(wavefrontNum)
    /// \param[in]          m_isScheduled(false
    /// \return
    /// -----------------------------------------------------------------------------------------------
    WorkGroup(size_t wavefrontNum, const std::vector<Instruction*>* pInstructions);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ~WorkGroup
    /// \brief Description: d`tor
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ~WorkGroup() {}

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsScheduled
    /// \brief Description: Get the indicator if work group was already scheduled.
    /// \return True :
    /// \return False:
    /// -----------------------------------------------------------------------------------------------
    bool IsScheduled() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsScheduleFinished
    /// \brief Description: Checks if scheduling of all instructions of the work group has completed
    /// \return True :
    /// \return False:
    /// -----------------------------------------------------------------------------------------------
    bool IsScheduleFinished() const;
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetCU
    /// \brief Description: Get compute unit to which WorkGroup is scheduled.
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetCU() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetWaveFrontNum
    /// \brief Description:
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetWaveFrontNum() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetWaveFront
    /// \brief Description: Get the wavefront #wf.
    /// \param[in]          wf
    /// \return WaveFront&
    /// -----------------------------------------------------------------------------------------------
    WaveFront& GetWaveFront(size_t wf);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetInstruction
    /// \brief Description: Get instruction which index in the instruction array is instIdx
    /// \param[in]          instIdx
    /// \return Instruction*
    /// -----------------------------------------------------------------------------------------------
    Instruction* GetInstruction(size_t instIdx);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        SetScheduled
    /// \brief Description: Set the indicator that the work group was  scheduled.
    /// \return void
    /// -----------------------------------------------------------------------------------------------
    void SetScheduled();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        SetScheduleFinished
    /// \brief Description: Set the indicator that the scheduling for all instructions for the work group was completed
    /// \return void
    /// -----------------------------------------------------------------------------------------------
    void SetScheduleFinished();
private:
    /// The number of wavefronts in the work group.
    size_t m_waveFrontNum;

    /// The Compute Unit to which work group was scheduled (Once the work group is cheduled to some compute unit
    /// it will execute only on this compute unit).
    size_t m_cu;

    /// Indicator if the work group was already scheduled ( to some compute unit).
    bool m_isScheduled;

    /// Indicator if he scheduling of the work group has completed.
    bool m_isScheduleFinished;

    /// Array of all wavefronts in the workgroup.
    std::vector<WaveFront> m_wavefront;

    /// The instruction [program kernel(s)] which work group executes.
    const std::vector<Instruction*>* m_pInstructions;
};

#endif //__WORKGROUP_H

