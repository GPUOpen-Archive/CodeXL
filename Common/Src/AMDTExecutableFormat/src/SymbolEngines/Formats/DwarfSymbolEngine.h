//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DwarfSymbolEngine.h
/// \brief This file contains the class for querying DWARF symbols.
///
//==================================================================================

#ifndef _DWARFSYMBOLENGINE_H_
#define _DWARFSYMBOLENGINE_H_

#include "../Generics/ModularSymbolEngine.h"
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTOSWrappers/Include/osReadWriteLock.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>

extern "C"
{
#include <libdwarf.h>
}

class ExecutableFile;

class DwarfSymbolEngine : public ModularSymbolEngine
{
public:
    DwarfSymbolEngine();
    virtual ~DwarfSymbolEngine();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Initializes the functions information of the image file.
    ///
    /// \param[in] exe The executable to load the symbols for.
    /// \param[in] pAuxSymEngine An auxiliary symbol engine.
    /// \param[in] pElf An optional associated ELF file.
    ///
    /// \note If the function is successful then it takes ownership of \a pAuxSymEngine and is responsible for its destruction.
    ///
    /// \return A boolean value indicating success or failure of the initialization.
    /// -----------------------------------------------------------------------------------------------
    bool Initialize(const ExecutableFile& exe, ModularSymbolEngine* pAuxSymEngine = NULL, struct _Elf* pElf = NULL);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function containing the RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the inlined function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    ///
    /// \return The FunctionSymbolInfo of the inlined function containing the RVA, if such exists.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupInlinedFunction(gtRVAddr rva) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Enumerates and associate a given source file line numbers with the corresponding image RVAs.
    ///
    /// \param[in] pSourceFilePath The full name of the source file's lines to enumerate.
    /// \param[out] srcLineInstanceMap The map of the image RVAs to line numbers.
    /// \param[in] handleInline Include inlined function (true) or caller function (false).
    ///
    /// \return A boolean value indicating success or failure of the enumeration.
    /// -----------------------------------------------------------------------------------------------
    virtual bool EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline = false);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the source line information of a RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] sourceLine The found source line information.
    /// \param[in] handleInline Include inlined function (true) or caller function (false).
    ///
    /// \return A boolean value indicating whether the a source line information was found.
    /// -----------------------------------------------------------------------------------------------
    virtual bool FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline = false);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Check if the symbols are complete (or partial).
    ///
    /// \return A boolean value indicating whether the symbols are complete.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsComplete() const { return true; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Convert the inlined RVA in the calling function to the corresponding RVA of inline function.
    ///
    /// \param[in] rva The RVA to be converted.
    ///
    /// \return The coverted RVA. If RVA is not the inlined code then return same RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr TranslateToInlineeRVA(gtRVAddr rva) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Find all the nested inlined caller function RVAs for given inlined callee function RVA.
    ///
    /// \param[in] rva The RVA to be searched.
    ///
    /// \return The list of RVAs. If input RVA is not inlined then return same RVA in the list.
    /// -----------------------------------------------------------------------------------------------
    virtual gtVector<gtRVAddr> FindNestedInlineFunctions(gtRVAddr rva) const;

    const ExecutableFile* GetExecutable() const { return m_pExe; }
    gtByte* AllocateBuffer(unsigned size);

private:
    bool TraverseSubprograms(struct _Dwarf_Die* cuDie, void* pData);
    bool TraverseSourceLines(struct _Dwarf_Die* cuDie, void* pData);
    bool TraverseSourceLineInfo(struct _Dwarf_Die* cuDie, void* pData);
    bool ForeachCompilationUnit(bool (DwarfSymbolEngine::*pfnProcess)(struct _Dwarf_Die*, void*), void* pData = NULL);
    bool IsElf() const;
    void Clear(bool deinit = true);
    void ClearInlinedFunctionInfo();

    bool ProcessInlinedFunctionByRva(gtRVAddr rva) const;
    bool ProcessInlinedFunctionInfo(Dwarf_Die rootDie, Dwarf_Addr cuAddrLow) const;
    void UpdateInlinedFunctionName(FunctionSymbolInfo& funcInfo) const;
    bool AddInlinedFuncInfo(Dwarf_Die die, Dwarf_Addr addrLow, Dwarf_Addr addrHigh, gtVector<FuncAddressRange>* addrRanges) const;
    const FunctionSymbolInfo* FindAggrInlineInstance(const wchar_t* name) const;
    bool DieFindLowHighAddress(Dwarf_Die die, Dwarf_Half tag, Dwarf_Addr& cuAddrLow,
                               Dwarf_Addr& addrLow, Dwarf_Addr& addrHigh, gtVector<FuncAddressRange>* addrRanges) const;
    bool ProcessNestedInlinedFunctionInfo(gtRVAddr rva) const;

    const ExecutableFile* m_pExe;
    struct _Dwarf_Debug* m_dbg;
    gtByte** m_pBuffers;

    ModularSymbolEngine* m_pAuxSymEngine;

    // Cache the inlined functions info
    // In the <key, value> pair of map, key is FunctionSymbolInfo and
    // value is true if the instance is used for aggregation
    // else value is set to false
    typedef gtMap<FunctionSymbolInfo, bool> FunctionsMap;
    mutable FunctionsMap m_inlinedFuncsInfo;
    mutable FunctionsMap m_processedInlineInfo;

    osCriticalSection   m_dwarfLock;

    bool m_processInlineSamples;
    bool m_aggregateInlineSamples;

    typedef gtHashMap<gtRVAddr, gtVector<gtRVAddr>> NestedFuncMap;
    mutable NestedFuncMap m_nestedFuncMap;
};

#endif // _DWARFSYMBOLENGINE_H_