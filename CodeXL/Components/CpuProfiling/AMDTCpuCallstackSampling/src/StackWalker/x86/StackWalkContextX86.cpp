//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackWalkContextX86.cpp
///
//==================================================================================

#include "StackWalkContextX86.h"
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

StackWalkContextX86::StackWalkContextX86() : m_vframe(0),
    m_vframePrev(0),
    m_params(0),
    m_paramsPrev(0),
    m_locals(0),
    m_localsPrev(0),
    m_validExtra(0),
    m_validExtraPrev(0),
    m_registerNotFound(false)
{
    memset(m_aVariables, 0, sizeof(m_aVariables));
}


gtUInt32 StackWalkContextX86::GetRegister(unsigned index) const
{
    gtUInt32 val;
    GetRegister(index, val);
    return val;
}

bool StackWalkContextX86::GetRegister(unsigned index, gtUInt32& val) const
{
    bool found = Parent::GetRegister(index, val);

    if (!found)
    {
        m_registerNotFound = true;
    }

    return found;
}

gtUInt32 StackWalkContextX86::GetVariable(unsigned index) const
{
    return m_validVariables.test(index) ? m_aVariables[index] : 0;
}

bool StackWalkContextX86::GetVariable(unsigned index, gtUInt32& val) const
{
    bool found = m_validVariables.test(index);
    val = found ? m_aVariables[index] : 0;
    return found;
}

void StackWalkContextX86::SetVariable(unsigned index, gtUInt32 val)
{
    m_aVariables[index] = val;
    m_validVariables.set(index);
}


void StackWalkContextX86::SetVFrame(gtUInt32 val)
{
    m_vframe = val;
    m_validExtra |= VALID_MASK_VFRAME;
}

gtUInt32 StackWalkContextX86::GetVFrame() const
{
    return (0 != (m_validExtra & VALID_MASK_VFRAME)) ? m_vframe : m_vframePrev;
}

bool StackWalkContextX86::GetVFrame(gtUInt32& val) const
{
    bool found = (0 != (m_validExtra & VALID_MASK_VFRAME));

    if (found)
    {
        val = m_vframe;
    }
    else
    {
        found = (0 != (m_validExtraPrev & VALID_MASK_VFRAME));
        val = m_vframePrev;
    }

    return found;
}


void StackWalkContextX86::SetParams(gtUInt32 val)
{
    m_params = val;
    m_validExtra |= VALID_MASK_PARAMS;
}

gtUInt32 StackWalkContextX86::GetParams() const
{
    return (0 != (m_validExtra & VALID_MASK_PARAMS)) ? m_params : m_paramsPrev;
}

bool StackWalkContextX86::GetParams(gtUInt32& val) const
{
    bool found = (0 != (m_validExtra & VALID_MASK_PARAMS));

    if (found)
    {
        val = m_params;
    }
    else
    {
        found = (0 != (m_validExtraPrev & VALID_MASK_PARAMS));
        val = m_paramsPrev;
    }

    return found;
}


void StackWalkContextX86::SetLocals(gtUInt32 val)
{
    m_locals = val;
    m_validExtra |= VALID_MASK_LOCALS;
}

gtUInt32 StackWalkContextX86::GetLocals() const
{
    return (0 != (m_validExtra & VALID_MASK_LOCALS)) ? m_locals : m_localsPrev;
}

bool StackWalkContextX86::GetLocals(gtUInt32& val) const
{
    bool found = (0 != (m_validExtra & VALID_MASK_LOCALS));

    if (found)
    {
        val = m_locals;
    }
    else
    {
        found = (0 != (m_validExtraPrev & VALID_MASK_LOCALS));
        val = m_localsPrev;
    }

    return found;
}

void StackWalkContextX86::Clear(VAddrX86 controlPc, VAddrX86 stackPtr, VAddrX86 framePtr)
{
    Parent::Clear();

    m_vframe = 0;
    m_vframePrev = 0;

    m_params = 0;
    m_paramsPrev = 0;

    m_locals = 0;
    m_localsPrev = 0;

    m_validExtra = 0;
    m_validExtraPrev = 0;

    m_registerNotFound = false;
    m_validVariables.reset();

    m_aRegistersBase[REG_INDEX_EIP] = controlPc;
    m_validRegistersBase |= (1U << REG_INDEX_EIP);

    m_aRegistersBase[REG_INDEX_ESP] = stackPtr;
    m_validRegistersBase |= (1U << REG_INDEX_ESP);

    m_aRegistersBase[REG_INDEX_EBP] = framePtr;
    m_validRegistersBase |= (1U << REG_INDEX_EBP);
}

void StackWalkContextX86::Reset()
{
    Parent::Reset();
    m_vframe = 0;
    m_params = 0;
    m_locals = 0;
    m_validExtra = 0;
}

void StackWalkContextX86::Propagate()
{
    Parent::Propagate();

    m_vframePrev = m_vframe;
    m_paramsPrev = m_params;
    m_localsPrev = m_locals;
    m_validExtraPrev = m_validExtra;

    m_vframe = 0;
    m_params = 0;
    m_locals = 0;
    m_validExtra = 0;

    m_registerNotFound = false;
    m_validVariables.reset();
}

void StackWalkContextX86::Extract(StackFrameControlData& frameData)
{
    frameData.m_programCounter = FindBaseRegister(REG_INDEX_EIP) ? m_aRegistersBase[REG_INDEX_EIP] : 0;
    frameData.m_stackPtr = FindBaseRegister(REG_INDEX_ESP) ? m_aRegistersBase[REG_INDEX_ESP] : 0;
    frameData.m_framePtr = FindBaseRegister(REG_INDEX_EBP) ? m_aRegistersBase[REG_INDEX_EBP] : 0;
}

HRESULT StackWalkContextX86::FindFrameInterface(gtUInt32 virtualAddr, IDiaFrameData** ppFrame) const
{
    HRESULT hr = E_FAIL;
    gtVAddr vaddr = static_cast<gtVAddr>(virtualAddr);
    ExecutableFile* pExe = m_pWorkingSet->FindModule(vaddr);

    if (NULL != pExe)
    {
        SymbolEngine* pSymbolEngine = pExe->GetSymbolEngine();

        if (NULL != pSymbolEngine)
        {
            hr = pSymbolEngine->FindFrameInterface(vaddr, ppFrame);
        }
    }

    return hr;
}
