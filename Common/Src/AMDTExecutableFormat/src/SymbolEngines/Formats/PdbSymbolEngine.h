//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PdbSymbolEngine.h
/// \brief This file contains the class for querying PDB symbols.
///
//==================================================================================

#ifndef _PDBSYMBOLENGINE_H_
#define _PDBSYMBOLENGINE_H_

#include <AMDTOSWrappers/Include/osReadWriteLock.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <SymbolEngine.h>
#include <dia2.h>

class PeFile;

class PdbSymbolEngine : public SymbolEngine
{
public:
    PdbSymbolEngine();
    virtual ~PdbSymbolEngine();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Initializes the functions information of the image file.
    ///
    /// \param[in] pe The executable to load the symbols for.
    /// \param[in] pSearchPath A list of semicolon separated directories to search for the symbols file.
    /// \param[in] pServerList A list of semicolon separated servers for downloading the symbol file.
    /// \param[in] pCachePath The directory where to download the symbol file to.
    ///
    /// \return A boolean value indicating success or failure of the initialization.
    /// -----------------------------------------------------------------------------------------------
    bool Initialize(const PeFile& pe,
                    const IMAGE_DEBUG_DIRECTORY* pDebugDirs,
                    unsigned debugDirsCount,
                    const wchar_t* pSearchPath = NULL,
                    const wchar_t* pServerList = NULL,
                    const wchar_t* pCachePath = NULL);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function containing the RVA.
    /// -----------------------------------------------------------------------------------------------
    // Baskar: The 'const' function is just a fake here as the LookupFunction performs on demand
    // function discovery. Do we need this 'const' factor here and 'mutable' data fields?
    virtual const FunctionSymbolInfo* LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function bounding the RVA (the first function that the RVA comes after).
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function bounding the RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupBoundingFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const;

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
    /// \brief Searches for the RVA of the symbol named \a pName and has the size of \a size.
    ///
    /// \param[in] pName The case-sensitive name of the symbol.
    /// \param[in] size The size in bytes of the symbol.
    ///
    /// \return The image RVA of the symbol. GT_INVALID_RVADDR on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr LoadSymbol(const wchar_t* pName, gtUInt32 size);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Check if the symbols are complete (or partial).
    ///
    /// \return A boolean value indicating whether the symbols are complete.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsComplete() const { return true; }

    virtual HRESULT FindFrameInterface(gtVAddr va, struct IDiaFrameData** ppFrame) const;
    virtual HRESULT FindSymbolInterface(gtVAddr va, struct IDiaSymbol** ppSymbol) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Convert the inlined RVA in calling function to the corresponding RVA of inline function.
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

private:
    bool OpenPdbFile(const IMAGE_DEBUG_DIRECTORY* pDebugDirs,
                     unsigned debugDirsCount,
                     const wchar_t* pRemoteSearchPath,
                     const wchar_t* pLocalSearchPath);
    void Close();

    HRESULT LoadRenamedPdbFile(IDiaDataSource* pDataSource,
                               const IMAGE_DEBUG_DIRECTORY* pDebugDirs,
                               unsigned debugDirsCount,
                               const wchar_t* pExePath,
                               const wchar_t* pSearchPath);

    bool FindFunctionSymbols(IDiaSymbol* pGlobalScope);
    bool FindPublicCodeSymbols(IDiaSymbol* pGlobalScope);

    // Collect the inlined information from DIA and updated the maps.
    bool ProcessInlinedFunctionInfo(IDiaSymbol* pGlobalScope) const;

    // Collect the inlined information from DIA and the update the maps - for the given DIA symbol
    bool ProcessInlinedFunction(IDiaSymbol* pSymbol) const;

    // Collect the inlined information from DIA and the update the maps - for the given un-decorated (?)
    // symbol name
    bool ProcessInlinedFunctionByName(const wchar_t* pSymbolName, gtList<wchar_t*>& inlinedChildNameList) const;

    void ConstructFunctionSymbolInfo(FunctionSymbolInfo& funcInfo, IDiaSymbol* pSymbol, bool addInlinedChild = false) const;
    bool SymbolHasInlinedChild(IDiaSymbol* pSymbol, gtList<wchar_t*>& inlinedChildNameList) const;
    bool AddInlinedChild(IDiaSymbol* pSymbol) const;
    bool AddInlinedChildByName(wchar_t* pSymbolName) const;

    void ClearFunctionsInfo();
    // clears m_inlinedFuncsInfo and m_inlineeLinesCache
    void ClearInlinedFunctionInfo();

    IDiaSymbol* FindBoundingFunction(gtRVAddr rva) const;
    IDiaSymbol* FindNextBoundingFunction(IDiaSymbol* pPrevSymbol, gtRVAddr rva) const;

    IDiaSymbol* FindFunctionSymbol(gtRVAddr rva) const;
    IDiaSymbol* FindInlineSymbol(gtRVAddr rva) const;
    IDiaEnumFrameData* FindEnumFrameData() const;

    // To check if given is part inline function expansion
    bool IsFunctionInlined(gtRVAddr rva) const;

    // Update the inlined function name into funcInfo
    void UpdateInlinedFunctionName(FunctionSymbolInfo& funcInfo) const;

    // Search nested inlined caller functions for given RVA
    bool ProcessNestedInlinedFunctionInfo(gtRVAddr rva) const;

    IDiaSession* m_pSession;                    ///< The DIA symbols engine session
    IDiaEnumSymbolsByAddr* m_pEnumSymbols;      ///< The DIA symbols enumerator
    IDiaEnumFrameData* m_pEnumFrames;           ///< The DIA frame data enumerator
    osCriticalSection   m_diaLock;

    const PeFile* m_pPortableExe;

    // true : samples attributed to the corresponding inlined function
    // false: samples attributed to the caller of the inlined function
    bool m_processInlineSamples;

    // true : aggregate samples from all the inlined instances of a function
    // false: don't aggregate samples from all the inlined instances of a function
    bool m_aggregateInlineSamples;

    typedef gtMap<FunctionSymbolInfo, bool> FunctionsMap;
    mutable FunctionsMap m_funcsInfo;
    mutable osReadWriteLock m_funcsLock;

    // Cache the inlined functions info
    // In the <key, value> pair, key is FunctionSymbolInfo & value is dummy
    mutable FunctionsMap m_inlinedFuncsInfo;
    osCriticalSection    m_inlineFuncsLock;

    mutable gtMap<gtRVAddr, IDiaSymbol*> m_symbolsCache;
    mutable gtMap<gtRVAddr, IDiaFrameData*> m_framesCache;

    // Ex: foo() is being inlined within the calling funtion bar()
    // Map for: rva of expanded/inlined foo() within bar() -> source line num of foo()
    // Map RVA (of inline expansion) to the inlinee function source line num
    // Multiple RVAs would be mapped to One source line
    struct LineInfo
    {
        DWORD len;       // number of bytes of the inlined expansion statement
        IDiaLineNumber* pLine;
    };
    typedef gtMap<gtRVAddr, LineInfo> InlineeLinesMap;
    mutable InlineeLinesMap m_inlineeLinesCache;

    typedef gtHashMap<gtRVAddr, gtVector<gtRVAddr>> NestedFuncMap;
    mutable NestedFuncMap m_nestedFuncMap;

    mutable gtVector<IDiaLineNumber*> m_pendingLineObjectsToRelease;

    /// A flag to indicate if PdbSymbolEngine called CoInitialize
    bool m_needToCallCoUninitialize;
};

#endif // _PDBSYMBOLENGINE_H_