//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProxySymbolEngine.h
/// \brief This file contains the class for querying symbols from an external file.
///
//==================================================================================

#ifndef _PROXYSYMBOLENGINE_H_
#define _PROXYSYMBOLENGINE_H_

#include "ModularSymbolEngine.h"

template <class TExe>
class ProxySymbolEngine : public SymbolEngine
{
public:
    ProxySymbolEngine(const wchar_t* pImageName) : m_exe(pImageName)
    {
    }

    virtual ~ProxySymbolEngine()
    {
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Initializes the functions information of the image file.
    ///
    /// \return A boolean value indicating success or failure of the initialization.
    /// -----------------------------------------------------------------------------------------------
    bool Initialize(gtVAddr loadAddress = GT_INVALID_VADDR)
    {
        bool ret = false;

        if (m_exe.Open(loadAddress))
        {
            if (m_exe.InitializeSymbolEngine())
            {
                ret = true;
            }
            else
            {
                m_exe.Close();
            }
        }

        return ret;
    }

    ModularSymbolEngine* GetInternalEngine()
    {
        return static_cast<ModularSymbolEngine*>(m_exe.GetSymbolEngine());
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function containing the RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const
    {
        return m_exe.GetSymbolEngine()->LookupFunction(rva, pNextRva, handleInline);
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function bounding the RVA (the first function that the RVA comes after).
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function bounding the RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupBoundingFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const
    {
        return m_exe.GetSymbolEngine()->LookupBoundingFunction(rva, pNextRva, handleInline);
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the inlined function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    ///
    /// \return The FunctionSymbolInfo of the inlined function containing the RVA, if such exists.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupInlinedFunction(gtRVAddr rva) const
    {
        return m_exe.GetSymbolEngine()->LookupInlinedFunction(rva);
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Enumerates and associate a given source file line numbers with the corresponding image RVAs.
    ///
    /// \param[in] pSourceFilePath The full name of the source file's lines to enumerate.
    /// \param[out] srcLineInstanceMap The map of the image RVAs to line numbers.
    /// \param[in] handleInline Include inlined function (true) or caller function (false).
    ///
    /// \return A boolean value indicating success or failure of the enumeration.
    /// -----------------------------------------------------------------------------------------------
    virtual bool EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline = false)
    {
        return m_exe.GetSymbolEngine()->EnumerateSourceLineInstances(pSourceFilePath, srcLineInstanceMap, handleInline);
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the source line information of a RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] sourceLine The found source line information.
    /// \param[in] handleInline Include inlined function (true) or caller function (false).
    ///
    /// \return A boolean value indicating whether the a source line information was found.
    /// -----------------------------------------------------------------------------------------------
    virtual bool FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline = false)
    {
        return m_exe.GetSymbolEngine()->FindSourceLine(rva, sourceLine, handleInline);
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the RVA of the symbol named \a pName and has the size of \a size.
    ///
    /// \param[in] pName The case-sensitive name of the symbol.
    /// \param[in] size The size in bytes of the symbol.
    ///
    /// \return The image RVA of the symbol. GT_INVALID_RVADDR on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr LoadSymbol(const wchar_t* pName, gtUInt32 size)
    {
        return m_exe.GetSymbolEngine()->LoadSymbol(pName, size);
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Check if the symbols are complete (or partial).
    ///
    /// \return A boolean value indicating whether the symbols are complete.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsComplete() const
    {
        return m_exe.GetSymbolEngine()->IsComplete();
    }


    const TExe& GetExecutable() const { return m_exe; }

private:
    TExe m_exe;
};

#endif // _PROXYSYMBOLENGINE_H_
