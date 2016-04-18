//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackWalkContext.h
///
//==================================================================================

#ifndef _STACKWALKCONTEXT_H_
#define _STACKWALKCONTEXT_H_

#include "StackFrameData.h"
#include <VirtualStack.h>
#include <cassert>

enum ReadMemoryType
{
    MEM_TYPE_ANY,
    MEM_TYPE_STACK,
    MEM_TYPE_CODE
};

#define INVALID_SECTION_INDEX ((unsigned)-1)

class BaseStackWalkContext
{
public:
    BaseStackWalkContext();

    int ReadMemory(ReadMemoryType type, gtVAddr virtualAddr, gtUByte* pBuffer, int len) const;
    gtVAddr GetImageLoadAddress(gtVAddr virtualAddr) const;
    gtVAddr GetImageLoadAddress(gtVAddr virtualAddr, gtVAddr& baseAddr) const;
    unsigned GetSectionInfo(gtVAddr virtualAddr, gtRVAddr& offset);

    HRESULT FindSymbolInterface(gtVAddr virtualAddr, struct IDiaSymbol** ppSymbol) const;

    void SetWorkingSet(ProcessWorkingSetQuery* pWorkingSet) { m_pWorkingSet = pWorkingSet; }
    void SetVirtualStack(VirtualStack* pStack) { m_pStack = pStack; }

protected:
    ProcessWorkingSetQuery* m_pWorkingSet;
    VirtualStack* m_pStack;
};


template <typename TReg, unsigned NRegs>
class StackWalkContext : public BaseStackWalkContext
{
public:
    StackWalkContext() : m_validRegistersNext(0U), m_validRegistersBase(0U)
    {
        memset(m_aRegistersNext, 0, sizeof(m_aRegistersNext));
        memset(m_aRegistersBase, 0, sizeof(m_aRegistersBase));
    }

    bool GetRegister(unsigned index, TReg& val) const
    {
        assert(index < NRegs);
        bool found;

        if (FindRegister(index))
        {
            found = true;
            val = m_aRegistersNext[index];
        }
        else
        {
            found = FindBaseRegister(index);
            val = m_aRegistersBase[index];
        }

        return found;
    }

    TReg GetRegister(unsigned index) const
    {
        assert(index < NRegs);
        return FindRegister(index) ? m_aRegistersNext[index] : m_aRegistersBase[index];
    }

    void SetRegister(unsigned index, TReg val)
    {
        assert(index < NRegs);
        m_aRegistersNext[index] = val;
        m_validRegistersNext |= (1U << index);
    }

    bool RecoverRegister(unsigned index)
    {
        assert(index < NRegs);
        bool found = FindRegister(index);

        if (!found)
        {
            if (FindBaseRegister(index))
            {
                SetRegister(index, m_aRegistersBase[index]);
                found = true;
            }
        }

        return found;
    }

    bool FindRegister(unsigned index) const
    {
        assert(index < NRegs);
        return (0U != (m_validRegistersNext & (1U << index)));
    }

    bool FindBaseRegister(unsigned index) const
    {
        assert(index < NRegs);
        return (0U != (m_validRegistersBase & (1U << index)));
    }

    bool RemoveBaseRegister(unsigned index)
    {
        bool found = FindBaseRegister(index);

        if (found)
        {
            m_aRegistersBase[index] = 0;
            m_validRegistersBase ^= (1U << index);
        }

        return found;
    }

    void Clear()
    {
        m_validRegistersNext = 0U;
        m_validRegistersBase = 0U;
        memset(m_aRegistersNext, 0, sizeof(m_aRegistersNext));
        memset(m_aRegistersBase, 0, sizeof(m_aRegistersBase));
    }

    void Reset()
    {
        m_validRegistersNext = 0U;
        memset(m_aRegistersNext, 0, sizeof(m_aRegistersNext));
    }

    void Propagate()
    {
        memcpy(m_aRegistersBase, m_aRegistersNext, sizeof(m_aRegistersNext));
        m_validRegistersBase = m_validRegistersNext;
        Reset();
    }


    int ReadMemory(ReadMemoryType memType, TReg virtualAddr, void* pBuffer, int len) const
    {
        return BaseStackWalkContext::ReadMemory(memType,
                                                static_cast<gtVAddr>(virtualAddr),
                                                static_cast<gtUByte*>(pBuffer),
                                                len);
    }

    bool ReadFullMemory(ReadMemoryType memType, TReg virtualAddr, void* pBuffer, int len) const
    {
        return ReadMemory(memType, virtualAddr, pBuffer, len) == len;
    }

    template <typename TBuf>
    int ReadMemory(ReadMemoryType memType, TReg virtualAddr, TBuf& buffer) const
    {
        return ReadMemory(memType, virtualAddr, &buffer, sizeof(TBuf));
    }

    template <typename TBuf>
    bool ReadFullMemory(ReadMemoryType memType, TReg virtualAddr, TBuf& buffer) const
    {
        return ReadFullMemory(memType, virtualAddr, &buffer, sizeof(TBuf));
    }

    TReg GetImageLoadAddress(TReg virtualAddr) const
    {
        return static_cast<TReg>(BaseStackWalkContext::GetImageLoadAddress(static_cast<gtVAddr>(virtualAddr)));
    }

    TReg GetImageLoadAddress(TReg virtualAddr, TReg& baseAddr) const
    {
        gtVAddr tempBaseAddr;
        TReg loadAddr = static_cast<TReg>(BaseStackWalkContext::GetImageLoadAddress(static_cast<gtVAddr>(virtualAddr), tempBaseAddr));
        baseAddr = static_cast<TReg>(tempBaseAddr);
        return loadAddr;
    }

    unsigned GetSectionInfo(TReg virtualAddr, gtRVAddr& offset)
    {
        return BaseStackWalkContext::GetSectionInfo(static_cast<gtVAddr>(virtualAddr), offset);
    }

    HRESULT FindSymbolInterface(TReg virtualAddr, struct IDiaSymbol** ppSymbol) const
    {
        return BaseStackWalkContext::FindSymbolInterface(static_cast<gtVAddr>(virtualAddr), ppSymbol);
    }

protected:
    gtUInt32 m_validRegistersNext;
    gtUInt32 m_validRegistersBase;

    TReg m_aRegistersNext[NRegs];
    TReg m_aRegistersBase[NRegs];
};

#endif // _STACKWALKCONTEXT_H_
