//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __WAVEFRONT_H
#define __WAVEFRONT_H

#include <vector>
#include "../Parser/Instruction.h"
class WaveFront
{
public:
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        WaveFront
    /// \brief Description: c`tor.
    /// \param[in]          pInstructions):m_nextInstIdx(0)
    /// \param[in]          m_pInstructions(pInstructions
    /// \return
    /// -----------------------------------------------------------------------------------------------
    explicit WaveFront(const std::vector<Instruction*>* pInstructions): m_nextInstIdx(0), m_nextBranchInstIdx(0), m_pInstructions(pInstructions) {}
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ~WaveFront
    /// \brief Description: d`tor.
    /// \param[in]          void
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ~WaveFront(void) {}
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetNextInstIdx
    /// \brief Description: Get next instruction index for scheduling and increase it.
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetNextInstIdx() { return m_nextInstIdx++; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetNextBranchInstIdx
    /// \brief Description: Get next branch instruction index for scheduling and increase it.
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetNextBranchInstIdx() { return m_nextBranchInstIdx++; }
private:
    /// The index of the next instruction to schedule.
    size_t m_nextInstIdx;

    /// The index of the next branch instruction to schedule.
    size_t m_nextBranchInstIdx;

    /// A vector of all instructions ,which should be scheduled and executed.
    const std::vector<Instruction*>* m_pInstructions;
};

#endif //__WAVEFRONT_H

