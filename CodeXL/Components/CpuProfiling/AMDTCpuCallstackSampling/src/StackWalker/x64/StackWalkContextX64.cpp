//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackWalkContextX64.cpp
///
//==================================================================================

#include "StackWalkContextX64.h"
#include <AMDTExecutableFormat/inc/PeFile.h>

StackWalkContextX64::StackWalkContextX64()
{
}

void StackWalkContextX64::Clear(VAddrX64 controlPc, VAddrX64 stackPtr, VAddrX64 framePtr)
{
    Parent::Clear();

    m_aRegistersBase[REG_INDEX_RIP] = controlPc;
    m_validRegistersBase |= (1U << REG_INDEX_RIP);

    m_aRegistersBase[REG_INDEX_RSP] = stackPtr;
    m_validRegistersBase |= (1U << REG_INDEX_RSP);

    m_aRegistersBase[REG_INDEX_RBP] = framePtr;
    m_validRegistersBase |= (1U << REG_INDEX_RBP);
}

void StackWalkContextX64::Extract(StackFrameControlData& frameData)
{
    frameData.m_programCounter = FindBaseRegister(REG_INDEX_RIP) ? m_aRegistersBase[REG_INDEX_RIP] : 0;
    frameData.m_stackPtr = FindBaseRegister(REG_INDEX_RSP) ? m_aRegistersBase[REG_INDEX_RSP] : 0;
    frameData.m_framePtr = FindBaseRegister(REG_INDEX_RBP) ? m_aRegistersBase[REG_INDEX_RBP] : 0;
}

HRESULT StackWalkContextX64::LookupPdataEntry(VAddrX64 virtualAddr, AMD64_RELOCATED_PDATA_ENTRY& pdataEntry) const
{
    HRESULT hr = E_FAIL;
    PeFile* pPe = static_cast<PeFile*>(m_pWorkingSet->FindModule(virtualAddr));

    if (NULL != pPe)
    {
        const RUNTIME_FUNCTION* pFuncEntry = pPe->LookupFunctionEntry64(pPe->VaToRva(virtualAddr));

        //
        // If the specified function entry is not NULL and specifies indirection,
        // then compute the address of the master function table entry.
        //
        if (NULL != pFuncEntry && 0 != (pFuncEntry->UnwindData & RUNTIME_FUNCTION_INDIRECT))
        {
            gtUInt32 sizeFuncEntry = sizeof(RUNTIME_FUNCTION);
            pFuncEntry = reinterpret_cast<const RUNTIME_FUNCTION*>(pPe->GetMemoryBlock(pFuncEntry->UnwindData - 1, sizeFuncEntry));

            if (sizeof(RUNTIME_FUNCTION) != sizeFuncEntry)
            {
                pFuncEntry = NULL;
            }
        }

        if (NULL != pFuncEntry)
        {
            pdataEntry.BeginAddress      = pPe->RvaToVa(pFuncEntry->BeginAddress);
            pdataEntry.EndAddress        = pPe->RvaToVa(pFuncEntry->EndAddress);
            pdataEntry.UnwindInfoAddress = pPe->RvaToVa(pFuncEntry->UnwindData);

            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}
