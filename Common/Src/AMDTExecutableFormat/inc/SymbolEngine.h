//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SymbolEngine.h
/// \brief This file contains the base class for a symbol engine.
///
//==================================================================================

#ifndef _SYMBOLENGINE_H_
#define _SYMBOLENGINE_H_

#include "ExecutableFormatDLLBuild.h"
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>


#if defined(JIT_SUPPORT) || defined(TBI)
    #define FUNCSYM_OFFSET_SUPPORT(x) x
#else
    #define FUNCSYM_OFFSET_SUPPORT(x)
#endif


/// -----------------------------------------------------------------------------------------------
/// \struct AddressRange
/// \brief Address range information of a function
/// -----------------------------------------------------------------------------------------------
struct FuncAddressRange
{
    FuncAddressRange(gtRVAddr start, gtRVAddr end) : m_rvaStart(start), m_rvaEnd(end) {}

    bool operator<(const FuncAddressRange& rhs) const
    {
        return m_rvaStart < rhs.m_rvaStart;
    }

    gtRVAddr m_rvaStart;
    gtRVAddr m_rvaEnd;
};

/// -----------------------------------------------------------------------------------------------
/// \struct FunctionSymbolInfo
/// \brief An executable function information.
/// -----------------------------------------------------------------------------------------------
struct FunctionSymbolInfo
{
    bool operator==(const FunctionSymbolInfo& rhs) const
    {
        return m_rva  == rhs.m_rva  &&
               m_size == rhs.m_size &&
               FUNCSYM_OFFSET_SUPPORT(m_offset == rhs.m_offset&&)
               0 == wcscmp(((NULL !=     m_pName) ?     m_pName : L""),
                           ((NULL != rhs.m_pName) ? rhs.m_pName : L""));
    }

    bool operator<(const FunctionSymbolInfo& rhs) const
    {
        return m_rva < rhs.m_rva;
    }

    FUNCSYM_OFFSET_SUPPORT(gtUInt32 m_offset;) ///< The offset from the RVA
    gtRVAddr m_rva;   ///< The relative virtual address to the image base
    gtUInt32 m_size;  ///< The size of the function
    bool     m_hasInlines;
    gtVector<FuncAddressRange>* m_addrRanges;
    wchar_t* m_pName; ///< The function name
    gtUInt32  m_funcId = 0;
};

inline bool operator<(const FunctionSymbolInfo& funcInfo, gtRVAddr rva)
{
    return (funcInfo.m_rva + funcInfo.m_size + static_cast<gtVAddr>(0U == funcInfo.m_size)) <= rva;
}

inline bool operator<(gtRVAddr rva, const FunctionSymbolInfo& funcInfo)
{
    return rva < funcInfo.m_rva;
}


/// -----------------------------------------------------------------------------------------------
/// \struct SourceLineInfo
/// \brief A source file's line information.
/// -----------------------------------------------------------------------------------------------
struct SourceLineInfo
{
    gtRVAddr m_rva;                ///< The start relative virtual address to the image base
    gtRVAddr m_offset;             ///< The line offset from the RVA
    unsigned m_line;               ///< The line number
    wchar_t m_filePath[OS_MAX_PATH];  ///< The full file name
};

// This stores the line RVA to line number map for a given file.
typedef gtMap<gtRVAddr, unsigned> SrcLineInstanceMap;


class EXE_API SymbolEngine
{
public:
    virtual ~SymbolEngine() {}

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function containing the RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the function bounding the RVA (the first function that the RVA comes after).
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] pNextRva The image relative virtual address of the following FunctionSymbolInfo.
    /// \param[in] handleInline Return inlined function (true) or caller function (false).
    ///
    /// \return The FunctionSymbolInfo of the function bounding the RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupBoundingFunction(gtRVAddr rva, gtRVAddr* pNextRva = NULL, bool handleInline = false) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the information of the inlined function containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    ///
    /// \return The FunctionSymbolInfo of the inlined function containing the RVA, if such exists.
    /// -----------------------------------------------------------------------------------------------
    virtual const FunctionSymbolInfo* LookupInlinedFunction(gtRVAddr rva) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Enumerates and associate a given source file line numbers with the corresponding image RVAs.
    ///
    /// \param[in] pSourceFilePath The full name of the source file's lines to enumerate.
    /// \param[out] srcLineInstanceMap The map of the image RVAs to line numbers.
    /// \param[in] handleInline Include inlined function (true) or caller function (false).
    ///
    /// \return A boolean value indicating success or failure of the enumeration.
    /// -----------------------------------------------------------------------------------------------
    virtual bool EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline = false) = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the source line information of a RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[out] sourceLine The found source line information.
    /// \param[in] handleInline Include inlined function (true) or caller function (false).
    ///
    /// \return A boolean value indicating whether the a source line information was found.
    /// -----------------------------------------------------------------------------------------------
    virtual bool FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline = false) = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the RVA of the symbol named \a pName and has the size of \a size.
    ///
    /// \param[in] pName The case-sensitive name of the symbol.
    /// \param[in] size The size in bytes of the symbol.
    ///
    /// \return The image RVA of the symbol. GT_INVALID_RVADDR on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr LoadSymbol(const wchar_t* pName, gtUInt32 size) = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Check if the symbols are complete (or partial).
    ///
    /// \return A boolean value indicating whether the symbols are complete.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsComplete() const = 0;

    virtual HRESULT FindFrameInterface(gtVAddr va, struct IDiaFrameData** ppFrame) const { (void)va; (void)ppFrame; return E_NOTIMPL; }
    virtual HRESULT FindSymbolInterface(gtVAddr va, struct IDiaSymbol** ppSymbol) const { (void)va; (void)ppSymbol; return E_NOTIMPL; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Convert the inlined RVA in the calling function to the corresponding RVA of inline function.
    ///
    /// \param[in] rva The RVA to be converted.
    ///
    /// \return The converted RVA. If RVA is not the inlined code then return same RVA.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr TranslateToInlineeRVA(gtRVAddr rva) const { return rva; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Find all the nested inlined caller function RVAs for given inlined callee function RVA.
    ///
    /// \param[in] rva The RVA to be searched.
    ///
    /// \return The list of RVAs. If input RVA is not inlined then return same RVA in the list.
    /// -----------------------------------------------------------------------------------------------
    virtual gtVector<gtRVAddr> FindNestedInlineFunctions(gtRVAddr rva) const { gtVector<gtRVAddr> list; list.push_back(rva); return list; }

    static unsigned FindFile(const wchar_t* pSearchPath, const wchar_t* pFileName, wchar_t* pBuffer);

    static unsigned DemangleNameIA(const char* pMangledName, char* pDemangledName, unsigned maxDemangledNameLen);
    static wchar_t* DemangleNameIA(const char* pMangledName);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    static unsigned DemangleExternCNameVS(const char* pMangledName, char* pDemangledName, unsigned maxDemangledNameLen);
    static unsigned DemangleExternCNameVS(const wchar_t* pMangledName, wchar_t* pDemangledName, unsigned maxDemangledNameLen);
    static unsigned DemangleExternCNameVS(const char* pMangledName, unsigned lenMangledName, char* pDemangledName, unsigned maxDemangledNameLen);
    static unsigned DemangleExternCNameVS(const wchar_t* pMangledName, unsigned lenMangledName, wchar_t* pDemangledName, unsigned maxDemangledNameLen);
    static unsigned DemangleNameVS(const char* pMangledName, char* pDemangledName, unsigned maxDemangledNameLen);
    static unsigned DemangleNameVS(const wchar_t* pMangledName, wchar_t* pDemangledName, unsigned maxDemangledNameLen);
    static wchar_t* DemangleNameVS(const char* pMangledName);

protected:
    static wchar_t* AnsiToUnicode(const char* pAnsi, int lenAnsi = -1);
#endif

protected:
    mutable gtInt32 m_nextFuncId = 1;
};

#endif // _SYMBOLENGINE_H_
