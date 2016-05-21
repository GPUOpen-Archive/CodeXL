//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ExecutableAnalyzer.cpp
///
//==================================================================================

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTDisassembler/inc/Disasmwrapper.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include "ExecutableAnalyzer.h"

static inline bool IsFunctionExitPoint(const CInstr_Table& inst)
{
    return ((evRetSpecies    == inst.InstSpecies)                                  ||
            (evSysretSpecies == inst.InstSpecies)                                  ||
            (evIretSpecies   == inst.InstSpecies)                                  ||
            (evIretdSpecies  == inst.InstSpecies));
}

ExecutableAnalyzer::ExecutableAnalyzer(ExecutableFile& exe) : m_exe(exe), m_pDasm(new CDisasmwrapper())
{
    FunctionSymbolInfo func;
    func.m_rva = m_exe.GetEntryPoint();
    func.m_size = 0;
    func.m_pName = const_cast<wchar_t*>(L"!EntryPoint");

    m_functions.insert(func);
}

ExecutableAnalyzer::~ExecutableAnalyzer()
{
    delete m_pDasm;
}

bool ExecutableAnalyzer::AnalyzeIsSystemCall(gtVAddr va)
{
    bool ret;

    gtRVAddr rva = m_exe.VaToRva(va);
    AnalyzeCodeRva(rva);

    m_mapLock.lockRead();
    ret = IsSystemCall(rva);
    m_mapLock.unlockRead();

    return ret;
}

void ExecutableAnalyzer::AnalyzeCode(gtVAddr va)
{
    gtRVAddr rva = m_exe.VaToRva(va);
    AnalyzeCodeRva(rva);
}

gtSet<FunctionSymbolInfo>::const_iterator ExecutableAnalyzer::AnalyzeCodeRva(gtRVAddr rva)
{
    gtSet<FunctionSymbolInfo>::const_iterator itContainingFunc;
    bool analyzed = false;

    FunctionSymbolInfo func;
    func.m_rva = rva;
    func.m_size = 0;
    func.m_pName = nullptr;

    m_mapLock.lockRead();
    itContainingFunc = m_functions.end();
    gtSet<FunctionSymbolInfo>::const_iterator itFuncUpper = m_functions.upper_bound(func);

    if (m_functions.begin() != itFuncUpper)
    {
        gtSet<FunctionSymbolInfo>::const_iterator itFuncLower = itFuncUpper;
        --itFuncLower;

        //
        // If the address is inside the function, then, clearly, we have already analyzed this function.
        // Otherwise we may start the analysis from the end bounding function.
        //

        gtRVAddr rvaEnd = itFuncLower->m_rva + itFuncLower->m_size;

        if (rva < rvaEnd)
        {
            itContainingFunc = itFuncLower;
            analyzed = true;
        }
        else
        {
            func.m_rva = rvaEnd;
        }
    }
    else
    {
        //
        // No bounding function has been analyzed yet.
        //

        func.m_rva = GT_INVALID_RVADDR;
    }


    if (!analyzed)
    {
        //
        // If we have an upper bound function, then use it as a limit for the function's size.
        //

        if (m_functions.end() != itFuncUpper)
        {
            func.m_size = itFuncUpper->m_rva - func.m_rva;
        }

        m_mapLock.unlockRead();

        DisassembleContainingFunctionExcept(rva, func, itContainingFunc);
    }
    else
    {
        m_mapLock.unlockRead();
    }

    return itContainingFunc;
}

bool ExecutableAnalyzer::IsSystemCall(gtRVAddr rva) const
{
    bool ret = false;

    gtSet<gtRVAddr>::const_iterator itSysCall = m_systemCalls.upper_bound(rva);

    if (m_systemCalls.begin() != itSysCall)
    {
        --itSysCall;

        if (rva < (*itSysCall + 2))
        {
            ret = true;
        }
    }

    return ret;
}

const FunctionSymbolInfo* ExecutableAnalyzer::FindAnalyzedFunction(gtVAddr va, bool handleInline)
{
    const FunctionSymbolInfo* pFunc = nullptr;
    gtRVAddr rva = m_exe.VaToRva(va);

    if (handleInline)
    {
        SymbolEngine* pSymbolEngine = m_exe.GetSymbolEngine();

        if (nullptr != pSymbolEngine)
        {
            pFunc = pSymbolEngine->LookupFunction(rva, nullptr, handleInline);
        }
    }

    if (nullptr == pFunc)
    {
        gtSet<FunctionSymbolInfo>::const_iterator itFunc = AnalyzeCodeRva(rva);

        m_mapLock.lockRead();

        if (m_functions.end() != itFunc)
        {
            pFunc = &(*itFunc);

            //
            // If this is an internal function, then it may not truly contain the address.
            // We need to search for the parent function.
            //

            if ((pFunc->m_rva + pFunc->m_size) <= rva)
            {
                gtSet<FunctionSymbolInfo>::const_iterator itFuncBegin = m_functions.begin();

                while (itFuncBegin != itFunc)
                {
                    //
                    // The functions list is sorted (in an ascending order).
                    //

                    const FunctionSymbolInfo& parentFunc = *(--itFunc);

                    // If we have gone too far, then stop.
                    // Though, we should never reach this point!
                    if (parentFunc.m_rva > rva)
                    {
                        pFunc = nullptr;
                        break;
                    }

                    if ((parentFunc.m_rva + parentFunc.m_size) > rva)
                    {
                        pFunc = &parentFunc;
                        break;
                    }
                }
            }
        }

        m_mapLock.unlockRead();
    }

    return pFunc;
}

bool ExecutableAnalyzer::AddFunctionEntry(gtRVAddr rva)
{
    bool ret;
    FunctionSymbolInfo func;
    func.m_rva = rva;
    func.m_size = 0;
    func.m_pName = nullptr;

    m_mapLock.lockWrite();
    ret = m_functions.insert(func).second;
    m_mapLock.unlockWrite();
    return ret;
}

bool ExecutableAnalyzer::AddFunction(FunctionSymbolInfo& func, gtSet<FunctionSymbolInfo>::const_iterator& itAddedFunc)
{
    bool ret = true;
    m_mapLock.lockWrite();

    std::pair<gtSet<FunctionSymbolInfo>::iterator, bool> pairib = m_functions.insert(func);
    itAddedFunc = pairib.first;

    if (!pairib.second)
    {
        FunctionSymbolInfo& funcSet = const_cast<FunctionSymbolInfo&>(*pairib.first);

        if (0 == funcSet.m_size)
        {
            funcSet.m_size  = func.m_size;

            // This prevents us from overriding preallocated names.
            if (nullptr != func.m_pName)
            {
                funcSet.m_pName = func.m_pName;
            }
        }
        else
        {
            ret = false;
        }
    }

    m_mapLock.unlockWrite();
    return ret;
}

void ExecutableAnalyzer::DisassembleContainingFunctionExcept(gtRVAddr rva, FunctionSymbolInfo& func,
                                                             gtSet<FunctionSymbolInfo>::const_iterator& itContainingFunc)
{
    //
    // AMDTDisassembler is not that good right now, and sometimes throws system exceptions, like "Access Violation".
    // For now, just to prevent a rare, but possible, crash, we need to be able to catch those exceptions.
    //

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    __try
    {
#endif

        DisassembleContainingFunction(rva, func, itContainingFunc);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        GT_ASSERT(0);
    }

#endif
}

void ExecutableAnalyzer::DisassembleContainingFunction(gtRVAddr rva, FunctionSymbolInfo& func,
                                                       gtSet<FunctionSymbolInfo>::const_iterator& itContainingFunc)
{
    gtSet<FunctionSymbolInfo>::const_iterator itFuncEnd = itContainingFunc;

    CodeSymbolType codeSymType;
    unsigned sectionIndex = FindBoundingKnownCode(rva, func, codeSymType);

    if (m_exe.GetSectionsCount() > sectionIndex)
    {
        const gtUByte* pCode = GetCodeBytes(sectionIndex, func.m_rva);

        if (nullptr != pCode)
        {
            gtVector<int> workingVec;
            workingVec.reserve(32);

            if (CODE_SYM_FULL == codeSymType)
            {
                //
                // We have found a function in the symbol engine that contains the given address.
                // No need to search any further. Just analyze this function, and get it done with.
                //

                DisassembleFunction(pCode, func, workingVec);
                AddFunction(func, itContainingFunc);
            }
            else
            {
                //
                // Continue analyzing functions until we have successfully analyzed the address' containing function.
                //

                while (func.m_rva <= rva)
                {
                    if (CODE_SYM_NONE == codeSymType)
                    {
                        //
                        // Padding between functions is usually done with a series
                        // of "0xCC" ("int 3" instruction) or "0x90" ("nop" instruction) bytes.
                        // We can just skip them.
                        //
                        // Note that this is relevant only if we don't have the real function's entry point (CODE_SYM_NONE == codeSymType).
                        //

                        gtUByte pad = 0xCC;

                        if (pad == *pCode || (pad = 0x90) == *pCode)
                        {
                            do
                            {
                                //
                                // We consider the given address as part of an actual code, so we might have landed on a function that
                                // starts with a "int 3" or a "nop". Very rare, but possible.
                                // In this case, we would like to stop skipping byte codes.
                                //

                                if (func.m_rva >= rva)
                                {
                                    break;
                                }

                                ++pCode;
                                ++func.m_rva;
                                //TODO: Why func.m_size is not decreased here?
                            }
                            while (pad == *pCode);
                        }
                    }

                    gtUInt32 sizeLeft = func.m_size;
                    DisassembleFunction(pCode, func, workingVec);

                    if (0 == func.m_size)
                    {
                        itContainingFunc = itFuncEnd;
                        break;
                    }

                    AddFunction(func, itContainingFunc);

                    pCode += func.m_size;
                    func.m_rva += func.m_size;
                    func.m_size = sizeLeft - func.m_size;
                    func.m_pName = nullptr;
                    codeSymType = CODE_SYM_NONE;
                }
            }
        }
    }
}

bool ExecutableAnalyzer::DisassembleFunction(const gtUByte* pCodeBegin, FunctionSymbolInfo& func, gtVector<int>& pendingOffsets)
{
    bool foundExit = false;
    gtMap<int, int> analyzedBlocks;

    // Check if this is a Thunk function.
    // - A Thunk function always starts with a JMP instruction.
    if (0x25FF == *reinterpret_cast<const gtUInt16*>(pCodeBegin) || 0xE9 == *pCodeBegin)
    {
        int last = 4 + ((0xE9 == *pCodeBegin) ? 1 : 2) - 1;
        analyzedBlocks.insert(std::pair<int, int>(0, last));
    }
    else
    {
        pendingOffsets.clear();
        pendingOffsets.push_back(0);

        do
        {
            int offset = pendingOffsets.back();
            pendingOffsets.pop_back();

            // Try to analyze down to the next block, or to the supposed end of the function.
            const gtUByte* pCode = pCodeBegin + offset;
            const gtUByte* pCodeEnd = pCodeBegin + func.m_size;

            foundExit |= DisassembleBasicBlock(pCode, pCodeBegin, pCodeEnd, func, pendingOffsets, analyzedBlocks);
        }
        while (!pendingOffsets.empty());
    }

    if (!analyzedBlocks.empty())
    {
        gtMap<int, int>::const_iterator itBlock = analyzedBlocks.end();
        --itBlock;

        func.m_size = static_cast<gtUInt32>(itBlock->second + 1);
    }
    else
    {
        func.m_size = 0;
    }

    return foundExit;
}

bool ExecutableAnalyzer::DisassembleBasicBlock(const gtUByte* pCode, const gtUByte* pCodeBegin, const gtUByte* pCodeEnd,
                                               FunctionSymbolInfo& func,
                                               gtVector<int>& pendingOffsets, gtMap<int, int>& analyzedBlocks)
{
    bool foundExit = false;
    const gtUByte* pCodeFirst = pCode;

    while (pCode < pCodeEnd)
    {
        const CInstr_Table* pInst;
        int instLength = Decode(pCode, pInst);

        if (0 >= instLength)
        {
            pCode++;
            continue;
        }

        if (nullptr != pInst)
        {
            if (IsFunctionExitPoint(*pInst))
            {
                pCode += instLength;
                foundExit = true;
                break;
            }

            if (evCallSpecies == pInst->InstSpecies)
            {
                if (J == pInst->OpField1.AddrMethod)
                {
                    int offset = static_cast<int>(m_pDasm->GetDisplacement()) + instLength + static_cast<int>(pCode - pCodeBegin);
                    gtRVAddr targetRva = func.m_rva + static_cast<gtRVAddr>(static_cast<gtInt32>(offset));
                    AddFunctionEntry(targetRva);

                    //
                    // Check if the target function is within our range. If so, then we need to shrink the function.
                    //

                    if (0 < offset && static_cast<gtUInt32>(offset) < func.m_size)
                    {
                        //
                        // Remove from the pending list all the offsets beyond the target.
                        //

                        gtVector<int>::iterator itOffsetLast = pendingOffsets.begin(), itOffsetEnd = pendingOffsets.end();

                        for (gtVector<int>::iterator itOffset = itOffsetLast; itOffset != itOffsetEnd; ++itOffset)
                        {
                            if (!(*itOffset <= offset))
                            {
                                *itOffsetLast = *itOffset;
                                ++itOffsetLast;
                            }
                        }

                        pendingOffsets.erase(itOffsetLast, itOffsetEnd);


                        //
                        // Remove all the analyzed blocks beyond the target offset.
                        //

                        gtMap<int, int>::iterator itBlock = analyzedBlocks.lower_bound(offset);

                        if (analyzedBlocks.end() != itBlock && (itBlock->first <= offset && offset <= itBlock->second))
                        {
                            // Try to shrink the block.
                            if (itBlock->first < offset)
                            {
                                itBlock->second = offset - 1;
                                ++itBlock;
                            }

                            // Remove all the blocks outside the function's limits.
                            analyzedBlocks.erase(itBlock, analyzedBlocks.end());
                        }

                        func.m_size = static_cast<gtUInt32>(offset);
                        pCodeEnd = pCodeBegin + func.m_size;
                    }
                }
            }
            else if ((evJaSpecies   <= pInst->InstSpecies && pInst->InstSpecies <= evJzSpecies) ||
                     (evLoopSpecies <= pInst->InstSpecies && pInst->InstSpecies <= evLoopzSpecies))
            {
                //
                // We, currently, do not support parsing of absolute jump instructions.
                // This includes "switch jumps", which most probably will lead to false positives.
                //

                if (J == pInst->OpField1.AddrMethod)
                {
                    int offset = static_cast<int>(m_pDasm->GetDisplacement()) + instLength + static_cast<int>(pCode - pCodeBegin);

                    // Verify that the offset is within our range.
                    if (0 < offset && static_cast<gtUInt32>(offset) < func.m_size)
                    {
                        gtMap<int, int>::const_iterator itBlock = analyzedBlocks.lower_bound(offset);

                        // If the offset is not within an already processed block.
                        if (analyzedBlocks.end() == itBlock || offset < itBlock->first)
                        {
                            // If the offset is not already in the pending list.
                            if (pendingOffsets.end() == gtFind(pendingOffsets.begin(), pendingOffsets.end(), offset))
                            {
                                pendingOffsets.push_back(offset);
                            }
                        }
                    }
                }


                //
                // A jump instruction definitely exits the block.
                //

                if (evJmpSpecies == pInst->InstSpecies)
                {
                    pCode += instLength;
                    break;
                }
            }
            else if (evSyscallSpecies == pInst->InstSpecies || evSysenterSpecies == pInst->InstSpecies)
            {
                m_mapLock.lockWrite();
                m_systemCalls.insert(func.m_rva + static_cast<gtRVAddr>(pCode - pCodeBegin));
                m_mapLock.unlockWrite();
            }
        }

        pCode += instLength;
    }

    if (pCodeFirst < pCode)
    {
        analyzedBlocks.insert(std::pair<int, int>(static_cast<int>(pCodeFirst - pCodeBegin),
                                                  static_cast<int>(pCode - pCodeBegin) - 1));
    }

    return foundExit;
}

int ExecutableAnalyzer::Decode(const gtUByte* pCode, const CInstr_Table*& pInst)
{
    int length = -1;
    pInst = nullptr;

    m_dasmLock.enter();

    if (m_pDasm->Decode(pCode))
    {
        length = m_pDasm->GetLength();

        if (0 < length)
        {
            const CInstr_ExtraCodes* pExtraInfo = static_cast<CInstr_ExtraCodes*>(m_pDasm->GetExtraInfoPtr());

            if (nullptr != pExtraInfo)
            {
                pInst = &pExtraInfo->instr_table;
            }
        }
    }

    m_dasmLock.leave();

    return length;
}

const gtUByte* ExecutableAnalyzer::GetCodeBytes(unsigned sectionIndex, gtRVAddr rva) const
{
    const gtUByte* pBytes = m_exe.GetSectionBytes(sectionIndex);

    gtRVAddr startRva, endRva;

    if (nullptr != pBytes && m_exe.GetSectionRvaLimits(sectionIndex, startRva, endRva))
    {
        pBytes += rva - startRva;
    }

    return pBytes;
}

unsigned ExecutableAnalyzer::FindBoundingKnownCode(gtRVAddr rva, FunctionSymbolInfo& func, CodeSymbolType& codeSymType) const
{
    codeSymType = CODE_SYM_NONE;
    unsigned sectionIndex = m_exe.LookupSectionIndex(rva);

    gtRVAddr startRva, endRva;

    if (m_exe.GetSectionRvaLimits(sectionIndex, startRva, endRva))
    {
        gtUInt32 size = endRva - startRva;
        wchar_t* pName = nullptr;


        SymbolEngine* pSymbolEngine = m_exe.GetSymbolEngine();

        if (nullptr != pSymbolEngine)
        {
            gtRVAddr nextRva = GT_INVALID_RVADDR;
            const FunctionSymbolInfo* pFuncInfo = pSymbolEngine->LookupBoundingFunction(rva, &nextRva);

            if (nullptr != pFuncInfo)
            {
                if (startRva <= pFuncInfo->m_rva)
                {
                    if (0 == pFuncInfo->m_size)
                    {
                        if (rva < nextRva && nextRva < endRva)
                        {
                            endRva = nextRva;
                        }

                        startRva = pFuncInfo->m_rva;
                        size = endRva - startRva;
                        pName = pFuncInfo->m_pName;
                        codeSymType = CODE_SYM_PARTIAL;
                    }
                    // If we are inside the function, then save the function's information.
                    else if ((pFuncInfo->m_rva + pFuncInfo->m_size) > rva)
                    {
                        startRva = pFuncInfo->m_rva;
                        size = pFuncInfo->m_size;
                        pName = pFuncInfo->m_pName;
                        codeSymType = CODE_SYM_FULL;
                    }
                    else
                    {
                        if (rva < nextRva && nextRva < endRva)
                        {
                            endRva = nextRva;
                        }

                        startRva = pFuncInfo->m_rva + pFuncInfo->m_size;

                        // startRva should not exceed endRva
                        if (startRva > endRva)
                        {
                            startRva = endRva;
                        }

                        size = endRva - startRva;
                        pName = nullptr;
                    }
                }
                else
                {
                    if (nextRva < endRva)
                    {
                        endRva = nextRva;
                        size = endRva - startRva;
                    }
                }
            }
        }

        if (static_cast<gtInt32>(startRva) >= static_cast<gtInt32>(func.m_rva))
        {
            if (0 != func.m_size)
            {
                gtUInt32 adjustedSize = func.m_size - (startRva - func.m_rva);

                if (adjustedSize < size)
                {
                    size = adjustedSize;
                }
            }

            // function size should not exceed endRva
            if (startRva + size > endRva)
            {
                size = endRva - startRva;
            }

            func.m_rva = startRva;
            func.m_size = size;
            func.m_pName = pName;
        }
        else
        {
            if (GT_INVALID_RVADDR == func.m_rva)
            {
                func.m_rva = startRva;
            }

            if (0 == func.m_size)
            {
                if (size >= (func.m_rva - startRva))
                {
                    func.m_size = size - (func.m_rva - startRva);
                }
                else
                {
                    func.m_size = size;
                }

                codeSymType = CODE_SYM_PARTIAL;
            }
            else if (size < func.m_size)
            {
                func.m_size = (size < (func.m_rva - startRva)) ? size : (size - (func.m_rva - startRva));
            }
        }
    }

    return sectionIndex;
}
