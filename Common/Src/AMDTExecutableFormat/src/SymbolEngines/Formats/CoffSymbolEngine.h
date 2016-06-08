//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CoffSymbolEngine.h
/// \brief This file contains the class for querying COFF symbols.
///
//==================================================================================

#ifndef _COFFSYMBOLENGINE_H_
#define _COFFSYMBOLENGINE_H_

#include "../Generics/ModularSymbolEngine.h"

class PeFile;

class CoffSymbolEngine : public ModularSymbolEngine
{
public:
    CoffSymbolEngine();
    virtual ~CoffSymbolEngine();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Initializes the functions information of the image file.
    ///
    /// \param[in] pe The portable executable to load the symbols for.
    /// \param[in] pfnDemangleName The symbol name demangling function.
    ///
    /// \return A boolean value indicating success or failure of the initialization.
    /// -----------------------------------------------------------------------------------------------
    bool Initialize(const PeFile& pe, wchar_t* (*pfnDemangleName)(const char*));

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

    virtual bool HasSourceLineInfo() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Check if the symbols are complete (or partial).
    ///
    /// \return A boolean value indicating whether the symbols are complete.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsComplete() const;

private:
    void EnumerateFunctionSymbols(const PeFile& pe, wchar_t* (*pfnDemangleName)(const char*));
    void EnumerateExports(const PeFile& pe, const IMAGE_EXPORT_DIRECTORY& exports, wchar_t* (*pfnDemangleName)(const char*));
    unsigned GetSymbolsCount() const;
    const IMAGE_SYMBOL* FindFileSymbol(const char* pFileName) const;
    const IMAGE_SYMBOL* FindParentFileSymbol(unsigned index) const;

    const PeFile* m_pPortableExe;

    const IMAGE_SYMBOL* m_pSymbolTable;
    const char* m_pStringTable;

    int m_firstFileIndex;
    bool m_isComplete;
};

#endif // _COFFSYMBOLENGINE_H_
