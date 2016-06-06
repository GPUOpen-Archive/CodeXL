//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModularSymbolEngine.cpp
/// \brief This file contains the base class for a modular symbol engine.
///
//==================================================================================

#include "ModularSymbolEngine.h"
#include <AMDTBaseTools/Include/gtAlgorithms.h>

ModularSymbolEngine::ModularSymbolEngine() : m_pFuncsInfoVec(NULL)
{
}

ModularSymbolEngine::~ModularSymbolEngine()
{
    if (NULL != m_pFuncsInfoVec)
    {
        ClearFunctionsInfo();
        delete m_pFuncsInfoVec;
    }
}

const FunctionSymbolInfo* ModularSymbolEngine::LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    const FunctionSymbolInfo* const pFuncBegin = m_pFuncsInfoVec->data();
    const FunctionSymbolInfo* const pFuncEnd = pFuncBegin + m_pFuncsInfoVec->size();
    const FunctionSymbolInfo* pFunc = gtBinarySearch(pFuncBegin, pFuncEnd, rva);

    if (NULL != pNextRva)
    {
        if ((pFunc + 1) < pFuncEnd)
        {
            *pNextRva = (pFunc + 1)->m_rva;
        }
        else
        {
            *pNextRva = GT_INVALID_RVADDR;
        }
    }

    return pFunc;
}

const FunctionSymbolInfo* ModularSymbolEngine::LookupBoundingFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    const FunctionSymbolInfo* pFunc = NULL;

    if (true == handleInline)
    {
        pFunc = LookupInlinedFunction(rva);

        if (NULL != pFunc && NULL != pNextRva)
        {
            *pNextRva = GT_INVALID_RVADDR;
        }
    }

    if (NULL == pFunc)
    {
        const FunctionSymbolInfo* const pFuncBegin = m_pFuncsInfoVec->data();
        const FunctionSymbolInfo* const pFuncEnd = pFuncBegin + m_pFuncsInfoVec->size();
        pFunc = gtUpperBound(pFuncBegin, pFuncEnd, rva);

        if (pFuncBegin != pFunc)
        {
            if (NULL != pNextRva)
            {
                if (pFuncEnd != pFunc)
                {
                    *pNextRva = pFunc->m_rva;
                }
                else
                {
                    *pNextRva = GT_INVALID_RVADDR;
                }
            }

            pFunc--;
        }
        else
        {
            if (NULL != pNextRva)
            {
                *pNextRva = GT_INVALID_RVADDR;
            }

            pFunc = NULL;
        }
    }

    return pFunc;
}

const FunctionSymbolInfo* ModularSymbolEngine::LookupInlinedFunction(gtRVAddr rva) const
{
    GT_UNREFERENCED_PARAMETER(rva);
    return NULL;
}

gtRVAddr ModularSymbolEngine::LoadSymbol(const wchar_t* pName, gtUInt32 size)
{
    gtRVAddr rva = GT_INVALID_RVADDR;

    if (NULL != pName)
    {
        for (gtVector<FunctionSymbolInfo>::const_iterator it = m_pFuncsInfoVec->begin(), itEnd = m_pFuncsInfoVec->end(); it != itEnd; ++it)
        {
            if (it->m_size == size && 0 == wcscmp(it->m_pName, pName))
            {
                rva = it->m_rva;
                break;
            }
        }
    }

    return rva;
}

bool ModularSymbolEngine::HasSourceLineInfo() const
{
    return true;
}

void ModularSymbolEngine::Splice(ModularSymbolEngine& targetSymEngine)
{
    if (!m_pFuncsInfoVec->empty())
    {
        // Assume the local is more complete.
        std::swap(targetSymEngine.m_pFuncsInfoVec, m_pFuncsInfoVec);

        gtVector<FunctionSymbolInfo>& targetFuncsInfo = *targetSymEngine.m_pFuncsInfoVec;

        targetFuncsInfo.reserve(m_pFuncsInfoVec->size());

        gtVector<FunctionSymbolInfo>::iterator itSrc = m_pFuncsInfoVec->begin(), itSrcEnd = m_pFuncsInfoVec->end(),
                                               itDst = targetFuncsInfo.begin(), itDstEnd = targetFuncsInfo.end();

        while (itSrc != itSrcEnd)
        {
            if (itDst == itDstEnd)
            {
                targetFuncsInfo.insert(itDstEnd, itSrc, itSrcEnd);
                break;
            }

            if (itSrc->m_rva == itDst->m_rva)
            {
                // Assume the local (itDst after the swap) name is the correct full one (this is due to non-mangled names in DWARF).
                // Delete the name as we will not transfer it.
                delete [] itSrc->m_pName;

                FUNCSYM_OFFSET_SUPPORT(itDst->m_offset = itSrc->m_offset;)
                itDst->m_rva = itSrc->m_rva;
                itDst->m_size = itSrc->m_size;

                ++itSrc;
                ++itDst;
            }
            else if (itSrc->m_rva < itDst->m_rva)
            {
                gtVector<FunctionSymbolInfo>::iterator itBegin = itSrc;

                while (++itSrc != itSrcEnd)
                {
                    if (itSrc->m_rva >= itDst->m_rva)
                    {
                        size_t newPos = (itDst - targetFuncsInfo.begin()) + (itSrc - itBegin);
                        targetFuncsInfo.insert(itDst, itBegin, itSrc);
                        itDst = targetFuncsInfo.begin() + newPos;
                        itDstEnd = targetFuncsInfo.end();
                        break;
                    }
                }
            }
            else // (itSrc->m_rva > itDst->m_rva)
            {
                while (++itDst != itDstEnd)
                {
                    if (itSrc->m_rva <= itDst->m_rva)
                    {
                        break;
                    }
                }
            }
        }
    }

    delete m_pFuncsInfoVec;
    m_pFuncsInfoVec = NULL;
}

void ModularSymbolEngine::InitializeFunctionsInfo()
{
    if (NULL == m_pFuncsInfoVec)
    {
        m_pFuncsInfoVec = new gtVector<FunctionSymbolInfo>();
        m_pFuncsInfoVec->reserve(512);
    }
    else
    {
        ClearFunctionsInfo();
    }
}

void ModularSymbolEngine::ClearFunctionsInfo()
{
    for (gtVector<FunctionSymbolInfo>::iterator it = m_pFuncsInfoVec->begin(), itEnd = m_pFuncsInfoVec->end(); it != itEnd; ++it)
    {
        if (NULL != it->m_pName)
        {
            delete [] it->m_pName;
        }
    }

    m_pFuncsInfoVec->clear();
}
