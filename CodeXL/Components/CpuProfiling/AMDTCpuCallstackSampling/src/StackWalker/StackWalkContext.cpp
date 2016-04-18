//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackWalkContext.cpp
///
//==================================================================================

#include "StackWalkContext.h"
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

BaseStackWalkContext::BaseStackWalkContext() : m_pWorkingSet(NULL), m_pStack(NULL)
{
}

int BaseStackWalkContext::ReadMemory(ReadMemoryType type, gtVAddr virtualAddr, gtUByte* pBuffer, int len) const
{
    int lenRead = -1;

    if (MEM_TYPE_CODE != type)
    {
        assert(NULL != m_pStack);

        if (m_pStack->IsStackAddress(virtualAddr))
        {
            lenRead = len;

            if (NULL != pBuffer)
            {
                m_pStack->ReadMemory(virtualAddr, pBuffer, static_cast<gtUInt32>(len));
            }
        }
    }

    if (0 > lenRead && MEM_TYPE_STACK != type)
    {
        assert(NULL != m_pWorkingSet);
        ExecutableFile* pExe = m_pWorkingSet->FindModule(virtualAddr);

        if (NULL != pExe)
        {
            gtUInt32 size = static_cast<gtUInt32>(len);
            const gtUByte* pMem = pExe->GetMemoryBlock(pExe->VaToRva(virtualAddr), size);

            if (NULL != pMem)
            {
                if (NULL != pBuffer && 0 != size)
                {
                    memcpy(pBuffer, pMem, size);
                }

                lenRead = size;
            }
        }
    }

    return lenRead;
}

gtVAddr BaseStackWalkContext::GetImageLoadAddress(gtVAddr virtualAddr) const
{
    assert(NULL != m_pWorkingSet);
    ExecutableFile* pExe = m_pWorkingSet->FindModule(virtualAddr);
    return (NULL != pExe) ? pExe->GetLoadAddress() : 0ULL;
}

gtVAddr BaseStackWalkContext::GetImageLoadAddress(gtVAddr virtualAddr, gtVAddr& baseAddr) const
{
    assert(NULL != m_pWorkingSet);
    gtVAddr loadAddr;
    ExecutableFile* pExe = m_pWorkingSet->FindModule(virtualAddr);

    if (NULL != pExe)
    {
        baseAddr = pExe->GetImageBase();
        loadAddr = pExe->GetLoadAddress();
    }
    else
    {
        baseAddr = 0ULL;
        loadAddr = 0ULL;
    }

    return loadAddr;
}

unsigned BaseStackWalkContext::GetSectionInfo(gtVAddr virtualAddr, gtRVAddr& offset)
{
    assert(NULL != m_pWorkingSet);
    unsigned sectionIndex = INVALID_SECTION_INDEX;
    ExecutableFile* pExe = m_pWorkingSet->FindModule(virtualAddr);

    if (NULL != pExe)
    {
        gtRVAddr rva = pExe->VaToRva(virtualAddr);
        unsigned index = pExe->LookupSectionIndex(rva);

        gtRVAddr startRva, endRva;

        if (pExe->GetSectionRvaLimits(index, startRva, endRva))
        {
            sectionIndex = index;
            offset = rva - startRva;
        }
    }

    return sectionIndex;
}

HRESULT BaseStackWalkContext::FindSymbolInterface(gtVAddr virtualAddr, IDiaSymbol** ppSymbol) const
{
    assert(NULL != m_pWorkingSet);
    HRESULT hr = E_FAIL;
    ExecutableFile* pExe = m_pWorkingSet->FindModule(virtualAddr);

    if (NULL != pExe)
    {
        SymbolEngine* pSymbolEngine = pExe->GetSymbolEngine();

        if (NULL != pSymbolEngine)
        {
            hr = pSymbolEngine->FindSymbolInterface(virtualAddr, ppSymbol);
        }
    }

    return hr;
}
