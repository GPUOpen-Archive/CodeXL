//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StabsSymbolEngine.h
/// \brief This file contains the class for querying STABS symbols.
///
//==================================================================================

#ifndef _STABSSYMBOLENGINE_H_
#define _STABSSYMBOLENGINE_H_

#include "../Generics/ModularSymbolEngine.h"

class ExecutableFile;

class StabsSymbolEngine : public ModularSymbolEngine
{
public:
    StabsSymbolEngine();
    virtual ~StabsSymbolEngine();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Initializes the functions information of the image file.
    ///
    /// \param[in] exe The executable to load the symbols for.
    /// \param[in] stabIndex The index of the ".stab" section.
    /// \param[in] stabstrIndex The index of the ".stabstr" section.
    /// \param[in] pAuxSymEngine An auxiliary symbol engine.
    ///
    /// \note If the function is successful then it takes ownership of \a pAuxSymEngine and is responsible for its destruction.
    ///
    /// \return A boolean value indicating success or failure of the initialization.
    /// -----------------------------------------------------------------------------------------------
    bool Initialize(const ExecutableFile& exe, unsigned stabIndex, unsigned stabstrIndex, ModularSymbolEngine* pAuxSymEngine = NULL);

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

    /// -----------------------------------------------------------------------------------------------
    /// \brief Check if the symbols are complete (or partial).
    ///
    /// \return A boolean value indicating whether the symbols are complete.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsComplete() const { return true; }

private:
    struct InternalNList
    {
        gtUInt32 n_strx;  // index into string table of name
        gtUByte  n_type;  // type of symbol
        gtUByte  n_other; // misc info (usually empty)
        gtUInt16 n_desc;  // description field
        gtUInt32 n_value; // value of symbol
    };

    const InternalNList* m_pEntries;
    unsigned m_numEntries;

    const char* m_pStrings;
    unsigned m_sizeStrings;
    gtUInt32 m_imageBase;

    const ExecutableFile* m_pExe;

    ModularSymbolEngine* m_pAuxSymEngine;
};

#endif // _STABSSYMBOLENGINE_H_
