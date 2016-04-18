//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ExecutableAnalyzer.h
/// \brief Memory map utility class.
///
//==================================================================================

#ifndef _EXECUTABLEANALYZER_H_
#define _EXECUTABLEANALYZER_H_

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osReadWriteLock.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTExecutableFormat/inc/ProcessWorkingSet.h>

#ifndef _DISASM_H_
    #include <AMDTDisassembler/inc/SpeciesEnum.h>
#endif

class CDisasmwrapper;

class ExecutableAnalyzer
{
public:
    ExecutableAnalyzer(ExecutableFile& exe);
    ~ExecutableAnalyzer();
    ExecutableAnalyzer& operator=(const ExecutableAnalyzer&) = delete;

    bool AnalyzeIsSystemCall(gtVAddr va);
    void AnalyzeCode(gtVAddr va);

    const FunctionSymbolInfo* FindAnalyzedFunction(gtVAddr va, bool handleInline = false);

private:
    enum CodeSymbolType
    {
        CODE_SYM_NONE,
        CODE_SYM_PARTIAL,
        CODE_SYM_FULL
    };

    bool IsSystemCall(gtRVAddr rva) const;
    gtSet<FunctionSymbolInfo>::const_iterator AnalyzeCodeRva(gtRVAddr rva);

    void DisassembleContainingFunction(gtRVAddr rva, FunctionSymbolInfo& func,
                                       gtSet<FunctionSymbolInfo>::const_iterator& itContainingFunc);
    void DisassembleContainingFunctionExcept(gtRVAddr rva, FunctionSymbolInfo& func,
                                             gtSet<FunctionSymbolInfo>::const_iterator& itContainingFunc);

    bool DisassembleFunction(const gtUByte* pCodeBegin, FunctionSymbolInfo& func, gtVector<int>& pendingOffsets);
    bool DisassembleBasicBlock(const gtUByte* pCode, const gtUByte* pCodeBegin, const gtUByte* pCodeEnd,
                               FunctionSymbolInfo& func,
                               gtVector<int>& pendingOffsets, gtMap<int, int>& analyzedBlocks);

    unsigned FindBoundingKnownCode(gtRVAddr rva, FunctionSymbolInfo& func, CodeSymbolType& codeSymType) const;
    const gtUByte* GetCodeBytes(unsigned sectionIndex, gtRVAddr rva) const;
    int Decode(const gtUByte* pCode, const class CInstr_Table*& pInst);

    bool AddFunctionEntry(gtRVAddr rva);
    bool AddFunction(FunctionSymbolInfo& func, gtSet<FunctionSymbolInfo>::const_iterator& itAddedFunc);

    ExecutableFile& m_exe;
    CDisasmwrapper* m_pDasm;

    gtSet<FunctionSymbolInfo> m_functions;
    gtSet<gtRVAddr> m_systemCalls;
    mutable osReadWriteLock m_mapLock;
    mutable osCriticalSection m_dasmLock;
};

#endif // _EXECUTABLEANALYZER_H_
