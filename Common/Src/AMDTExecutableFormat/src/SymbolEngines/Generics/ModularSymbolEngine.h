//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModularSymbolEngine.h
/// \brief This file contains the base class for a modular symbol engine.
///
//==================================================================================

#ifndef _MODULARSYMBOLENGINE_H_
#define _MODULARSYMBOLENGINE_H_

#include <SymbolEngine.h>
#include <AMDTBaseTools/Include/gtVector.h>

class ModularSymbolEngine : public SymbolEngine
{
public:
    ModularSymbolEngine();
    virtual ~ModularSymbolEngine();

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
    /// \brief Searches for the RVA of the symbol named \a pName and has the size of \a size.
    ///
    /// \param[in] pName The case-sensitive name of the symbol.
    /// \param[in] size The size in bytes of the symbol.
    ///
    /// \return The image RVA of the symbol. GT_INVALID_RVADDR on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr LoadSymbol(const wchar_t* pName, gtUInt32 size);

    virtual bool HasSourceLineInfo() const;

    void Splice(ModularSymbolEngine& targetSymEngine);

protected:
    void InitializeFunctionsInfo();
    void ClearFunctionsInfo();

    gtVector<FunctionSymbolInfo>* m_pFuncsInfoVec; ///< The vector of known functions information
};

#endif // _MODULARSYMBOLENGINE_H_
