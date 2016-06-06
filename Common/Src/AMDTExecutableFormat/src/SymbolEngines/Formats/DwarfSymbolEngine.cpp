//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DwarfSymbolEngine.cpp
/// \brief This file contains the class for querying DWARF symbols.
///
//==================================================================================

#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtQueue.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <ExecutableFile.h>
#include "DwarfSymbolEngine.h"

extern "C"
{
#include <_libdwarf.h>

#ifndef DW_AT_MIPS_linkage_name
#   define DW_AT_MIPS_linkage_name  0x2007
#endif
}

#include <gelf.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define DWARF_API  __cdecl
#else
    #define DWARF_API
#endif

static int DWARF_API Access_get_section_info(void* pObj, Dwarf_Half index, Dwarf_Obj_Access_Section* pSection, int* pError);
static Dwarf_Endianness DWARF_API Access_get_byte_order(void* pObj);
static Dwarf_Small DWARF_API Access_get_length_size(void* pObj);
static Dwarf_Small DWARF_API Access_get_pointer_size(void* pObj);
static Dwarf_Unsigned DWARF_API Access_get_section_count(void* pObj);
static int DWARF_API Access_load_section(void* pObj, Dwarf_Half index, Dwarf_Small** ppData, int* pError);

static wchar_t* GetDemangledName(Dwarf_Debug dbg, Dwarf_Die die);
static wchar_t* ExtractSubprogramName(Dwarf_Debug dbg, Dwarf_Die die);
static Dwarf_Die DieFindAttribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half nattr);
static void InitCompilationUnit(Dwarf_Debug dbg);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define ACQUIRE_DWARF_LOCK(_lock)   osCriticalSectionLocker guard(const_cast<osCriticalSection&>(_lock))
#else
    #define ACQUIRE_DWARF_LOCK(_lock)
#endif

DwarfSymbolEngine::DwarfSymbolEngine() : m_pExe(NULL), m_dbg(NULL), m_pBuffers(NULL), m_pAuxSymEngine(NULL),
    m_processInlineSamples(false), m_aggregateInlineSamples(false)
{
}

DwarfSymbolEngine::~DwarfSymbolEngine()
{
    Clear();
}

bool DwarfSymbolEngine::Initialize(const ExecutableFile& exe, ModularSymbolEngine* pAuxSymEngine, Elf* pElf)
{
    bool ret = true;
    m_pExe = &exe;

    if (NULL == m_pExe)
    {
        ret = false;
    }

    ret = ret && (DW_DLE_NONE == _dwarf_alloc(&m_dbg, DW_DLC_READ, NULL));

    m_processInlineSamples = m_pExe->IsProcessInlineInfo();
    m_aggregateInlineSamples = m_pExe->IsAggregateInlinedInstances();

    if (ret)
    {
        if (NULL != pElf)
        {
            ret = (DW_DLE_NONE == _dwarf_elf_init(m_dbg, pElf, NULL));
        }
        else
        {
            static const Dwarf_Obj_Access_Methods PeAccessMethods =
            {
                Access_get_section_info,
                Access_get_byte_order,
                Access_get_length_size,
                Access_get_pointer_size,
                Access_get_section_count,
                Access_load_section
            };

            Dwarf_Obj_Access_Interface* iface = new Dwarf_Obj_Access_Interface;
            iface->object = this;
            iface->methods = &PeAccessMethods;

            m_dbg->dbg_iface = iface;
            m_dbg->dbg_machine = exe.Is64Bit() ? (Dwarf_Half)EM_X86_64 : (Dwarf_Half)EM_386;
        }

        ret = ret && (DW_DLE_NONE == _dwarf_init(m_dbg, 0, NULL, NULL, NULL));

        if (ret)
        {
            InitializeFunctionsInfo();
            ret = ForeachCompilationUnit(&DwarfSymbolEngine::TraverseSubprograms);

            if (ret)
            {
                gtSort(m_pFuncsInfoVec->begin(), m_pFuncsInfoVec->end());

                if (NULL != pAuxSymEngine)
                {
                    pAuxSymEngine->Splice(*this);

                    if (pAuxSymEngine->HasSourceLineInfo())
                    {
                        m_pAuxSymEngine = pAuxSymEngine;
                    }
                    else
                    {
                        // We have no further use of the auxiliary symbol engine.
                        delete pAuxSymEngine;
                    }
                }
            }
            else
            {
                Clear(true);
            }
        }
        else
        {
            Clear(false);
        }
    }

    return ret;
}

bool DwarfSymbolEngine::TraverseSubprograms(Dwarf_Die cuDie, void*)
{
    Dwarf_Die die;
    int r = dwarf_child(cuDie, &die, NULL);
    // dwarf_child can return DW_DLE_NO_ENTRY and in that case we need to stop traversing.
    bool ret = (DW_DLV_OK == r) ? true : false;

    if (DW_DLV_OK == r)
    {
        // We are looking for symbol information that belong to a function (subprogram) which stays at level 1.
        // Therefore, we only need to iterate over the level 1 siblings.
        do
        {
            Dwarf_Half tag;

            if (dwarf_tag(die, &tag, NULL) != DW_DLV_OK)
            {
                ret = false;
                break;
            }

            if (DW_TAG_subprogram == tag)
            {
                Dwarf_Addr addrLow, addrHigh;

                if (dwarf_lowpc(die, &addrLow , NULL) == DW_DLV_OK && dwarf_highpc(die, &addrHigh , NULL) == DW_DLV_OK)
                {
                    wchar_t* pName = ExtractSubprogramName(m_dbg, die);

                    if (NULL != pName)
                    {
                        FunctionSymbolInfo funcInfo;
                        FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U;)
                        funcInfo.m_addrRanges = NULL;
                        funcInfo.m_rva = static_cast<gtRVAddr>(addrLow - m_pExe->GetImageBase());
                        funcInfo.m_size = (addrLow <= addrHigh) ? static_cast<gtUInt32>(addrHigh - addrLow) :
                                          static_cast<gtUInt32>(addrHigh);
                        funcInfo.m_pName = pName;
                        funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);

                        m_pFuncsInfoVec->push_back(funcInfo);
                    }
                }
            }

            Dwarf_Die siblingDie = NULL;
            r = dwarf_siblingof(m_dbg, die, &siblingDie, NULL);
            dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
            die = siblingDie;

            ret = DW_DLV_ERROR != r;
        }
        while (DW_DLV_OK == r);

        if (NULL != die)
        {
            dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
        }
    }

    return ret;
}

// Process the complete subprogram DIE tree for inline information
bool DwarfSymbolEngine::ProcessInlinedFunctionInfo(Dwarf_Die rootDie, Dwarf_Addr cuAddrLow) const
{
    bool ret = false;

    gtQueue<Dwarf_Die> dieQueue;
    dieQueue.push(rootDie);

    while (!dieQueue.empty())
    {
        Dwarf_Die die = dieQueue.front();
        Dwarf_Die childDie;

        if (dwarf_child(die, &childDie, NULL) == DW_DLV_OK)
        {
            int r;

            do
            {
                bool keep_childDie = false;  // to dealloc the childDie or not
                Dwarf_Half tag;

                if (dwarf_tag(childDie, &tag, NULL) != DW_DLV_OK)
                {
                    // can't process further
                    dwarf_dealloc(m_dbg, childDie, DW_DLA_DIE);
                    break;
                }

                if (DW_TAG_subprogram == tag || DW_TAG_lexical_block == tag)
                {
                    dieQueue.push(childDie);
                    keep_childDie = true;
                }
                else if (DW_TAG_inlined_subroutine == tag)
                {
                    Dwarf_Addr addrLow = 0, addrHigh = 0;
                    gtVector<FuncAddressRange>* addrRanges = new gtVector<FuncAddressRange>;

                    DieFindLowHighAddress(childDie, tag, cuAddrLow, addrLow, addrHigh, addrRanges);

                    // if address ranges list is empty, then delete it
                    if (addrRanges->empty())
                    {
                        delete addrRanges;
                        addrRanges = NULL;
                    }
                    else
                    {
                        gtSort(addrRanges->begin(), addrRanges->end());
                    }

                    ret = AddInlinedFuncInfo(childDie, addrLow, addrHigh, addrRanges);
                    addrRanges = NULL; // list is saved to funcinfo, no need to delete

                    // inlined_subroutine might contain another inlined_subroutine
                    // add it to queue
                    dieQueue.push(childDie);
                    keep_childDie = true;
                }

                Dwarf_Die siblingDie = NULL;
                r = dwarf_siblingof(m_dbg, childDie, &siblingDie, NULL);

                if (false == keep_childDie)
                {
                    dwarf_dealloc(m_dbg, childDie, DW_DLA_DIE);
                }

                childDie = siblingDie;
                ret = DW_DLV_ERROR != r;
            }
            while (DW_DLV_OK == r);
        }

        if (die != rootDie)
        {
            // don't free rootDie, as it will be deallocated by caller
            dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
        }

        dieQueue.pop();
    }

    return ret;
}

bool DwarfSymbolEngine::AddInlinedFuncInfo(Dwarf_Die die, Dwarf_Addr addrLow, Dwarf_Addr addrHigh, gtVector<FuncAddressRange>* addrRanges) const
{
    FunctionSymbolInfo funcInfo;
    funcInfo.m_rva = static_cast<gtRVAddr>(addrLow - m_pExe->GetImageBase());
    funcInfo.m_size = static_cast<gtRVAddr>(addrHigh - addrLow);
    funcInfo.m_pName = ExtractSubprogramName(m_dbg, die);
    funcInfo.m_addrRanges = addrRanges;
    UpdateInlinedFunctionName(funcInfo);

    auto res = m_inlinedFuncsInfo.insert(FunctionsMap::value_type(funcInfo, false));

    if (false == res.second)
    {
        funcInfo.m_rva += 1;
        m_inlinedFuncsInfo.insert(FunctionsMap::value_type(funcInfo, false));
    }

    return true;
}

bool DwarfSymbolEngine::ForeachCompilationUnit(bool (DwarfSymbolEngine::* pfnProcess)(Dwarf_Die, void*), void* pData)
{
    bool ret = true;
    Dwarf_Die die = NULL;

    ACQUIRE_DWARF_LOCK(m_dwarfLock);
    InitCompilationUnit(m_dbg);

    while (dwarf_next_cu_header(m_dbg, NULL, NULL, NULL, NULL, NULL, NULL) == DW_DLV_OK)
    {
        if (dwarf_siblingof(m_dbg, NULL, &die, NULL) != DW_DLV_OK)
        {
            continue;
        }

        Dwarf_Half tag;

        if (dwarf_tag(die, &tag, NULL) != DW_DLV_OK)
        {
            ret = false;
            break;
        }

        if (DW_TAG_compile_unit == tag)
        {
            if (!(this->*pfnProcess)(die, pData))
            {
                ret = false;
                break;
            }
        }

        dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
        die = NULL;
    }

    if (NULL != die)
    {
        dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
    }

    return ret;
}

bool DwarfSymbolEngine::TraverseSourceLines(Dwarf_Die cuDie, void* pData)
{
    std::pair<const char*, SrcLineInstanceMap*>* pFullData = static_cast<std::pair<const char*, SrcLineInstanceMap*>*>(pData);
    const char* pSourceFilePath = pFullData->first;
    SrcLineInstanceMap& srcLineInstanceMap = *pFullData->second;

    // Do not call dwarf_dealloc() routines on this pointer. _dwarf_deinit() does the deallocation.
    char** srcFiles = NULL;
    Dwarf_Signed srcCount;

    if (DW_DLV_OK == dwarf_srcfiles(cuDie, &srcFiles, &srcCount, NULL))
    {
        for (int srcIndex = 0, sources = static_cast<int>(srcCount); srcIndex < sources; ++srcIndex)
        {
            if (0 == strcmp(pSourceFilePath, srcFiles[srcIndex]))
            {
                Dwarf_Line* pLineBuf;
                Dwarf_Signed lineCount = 0;

                if (DW_DLV_OK == dwarf_srclines(cuDie, &pLineBuf, &lineCount, NULL))
                {
                    for (int lineIndex = 0, lines = static_cast<int>(lineCount); lineIndex < lines; ++lineIndex)
                    {
                        // Check if this is end of text sequence
                        Dwarf_Bool lineEndSeq = 0;

                        if (DW_DLV_OK == dwarf_lineendsequence(pLineBuf[lineIndex], &lineEndSeq, NULL))
                        {
                            if (lineEndSeq)
                            {
                                continue;
                            }
                        }

                        // Skip lines NOT in pSourceFilePath
                        //
                        // Note that this can happen when we are looking at lines from other source files
                        // that are inlined into pSourceFilePath
                        Dwarf_Unsigned lineFileNo = 0;

                        if (DW_DLV_OK != dwarf_line_srcfileno(pLineBuf[lineIndex], &lineFileNo, NULL) ||
                            (lineFileNo - 1) != static_cast<Dwarf_Unsigned>(srcIndex))
                        {
                            continue;
                        }

                        Dwarf_Unsigned lineNo;
                        dwarf_lineno(pLineBuf[lineIndex], &lineNo, NULL);

                        Dwarf_Addr lineAddr;
                        dwarf_lineaddr(pLineBuf[lineIndex], &lineAddr, NULL);

                        gtRVAddr rva = static_cast<gtRVAddr>(lineAddr - m_pExe->GetImageBase());
                        srcLineInstanceMap[rva] = static_cast<unsigned>(lineNo);
                    }

                    dwarf_srclines_dealloc(m_dbg, pLineBuf, lineCount);
                }
            }
        }
    }

    return true;
}

bool DwarfSymbolEngine::EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    bool ret = true;

    if (NULL != m_pAuxSymEngine)
    {
        ret = m_pAuxSymEngine->EnumerateSourceLineInstances(pSourceFilePath, srcLineInstanceMap);
    }

    if (ret)
    {
        char sourceFilePathMb[OS_MAX_PATH];

        if ((size_t)(-1) != wcstombs(sourceFilePathMb, pSourceFilePath, OS_MAX_PATH))
        {
            std::pair<const char*, SrcLineInstanceMap*> data(sourceFilePathMb, &srcLineInstanceMap);
            ForeachCompilationUnit(&DwarfSymbolEngine::TraverseSourceLines, &data);
        }
    }

    return ret;
}

bool DwarfSymbolEngine::TraverseSourceLineInfo(Dwarf_Die cuDie, void* pData)
{
    SourceLineInfo& sourceLine = *static_cast<SourceLineInfo*>(pData);

    Dwarf_Die die;
    int r = dwarf_child(cuDie, &die, NULL);
    bool ret = (DW_DLV_OK == r) ? true : false;

    if (DW_DLV_OK == r)
    {
        Dwarf_Addr addr = static_cast<Dwarf_Addr>(sourceLine.m_rva) + m_pExe->GetImageBase();

        // We are looking for symbol information that belong to a function (subprogram) which stays at level 1.
        // Therefore, we only need to iterate over the level 1 siblings.
        do
        {
            Dwarf_Half tag;

            if (dwarf_tag(die, &tag, NULL) != DW_DLV_OK)
            {
                ret = false;
                break;
            }

            if (DW_TAG_subprogram == tag)
            {
                Dwarf_Addr addrLow, addrHigh;

                if (dwarf_lowpc(die, &addrLow , NULL) == DW_DLV_OK && dwarf_highpc(die, &addrHigh , NULL) == DW_DLV_OK)
                {
                    if (addrLow > addrHigh)
                    {
                        addrHigh += addrLow;
                    }

                    if (addrLow <= addr && addr <= addrHigh)
                    {
                        Dwarf_Line* pLineBuf;
                        Dwarf_Signed lineCount = 0;

                        if (DW_DLV_OK == dwarf_srclines(cuDie, &pLineBuf, &lineCount, NULL))
                        {
                            sourceLine.m_offset = sourceLine.m_rva + 1;
                            char* pSrcFilePath = NULL;

                            for (int lineIndex = 0, lines = static_cast<int>(lineCount); lineIndex < lines; ++lineIndex)
                            {
                                // Check if this is end of text sequence
                                Dwarf_Bool lineEndSeq = 0;

                                if (DW_DLV_OK == dwarf_lineendsequence(pLineBuf[lineIndex], &lineEndSeq, NULL))
                                {
                                    if (lineEndSeq)
                                    {
                                        continue;
                                    }
                                }

                                Dwarf_Addr lineAddr;

                                if (DW_DLV_OK != dwarf_lineaddr(pLineBuf[lineIndex], &lineAddr, NULL))
                                {
                                    continue;
                                }

                                gtRVAddr offset = static_cast<gtRVAddr>(addr - lineAddr);

                                if (offset >= sourceLine.m_offset)
                                {
                                    continue;
                                }

                                char* pLineSrc = NULL;
                                dwarf_linesrc(pLineBuf[lineIndex], &pLineSrc, NULL);

                                if (NULL == pLineSrc)
                                {
                                    continue;
                                }

                                Dwarf_Unsigned lineNo;

                                if (DW_DLV_OK != dwarf_lineno(pLineBuf[lineIndex], &lineNo, NULL))
                                {
                                    continue;
                                }

                                sourceLine.m_offset = offset;
                                sourceLine.m_line = static_cast<unsigned>(lineNo);
                                pSrcFilePath = pLineSrc;

                                if (0 == offset)
                                {
                                    break;
                                }
                            }

                            dwarf_srclines_dealloc(m_dbg, pLineBuf, lineCount);

                            if (NULL != pSrcFilePath && (size_t)(-1) != mbstowcs(sourceLine.m_filePath, pSrcFilePath, OS_MAX_PATH))
                            {
                                sourceLine.m_rva -= sourceLine.m_offset;
                            }
                        }

                        ret = false;
                        break;
                    }
                }
            }

            Dwarf_Die siblingDie = NULL;
            r = dwarf_siblingof(m_dbg, die, &siblingDie, NULL);
            dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
            die = siblingDie;

            ret = DW_DLV_ERROR != r;
        }
        while (DW_DLV_OK == r);

        if (NULL != die)
        {
            dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
        }
    }

    return ret;
}

bool DwarfSymbolEngine::FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline)
{
    bool ret = false;

    if (NULL != m_pAuxSymEngine)
    {
        ret = m_pAuxSymEngine->FindSourceLine(rva, sourceLine, handleInline);
    }

    if (!ret)
    {
        const FunctionSymbolInfo* pFuncInfo = LookupFunction(rva, NULL, handleInline);

        if (NULL != pFuncInfo)
        {
            sourceLine.m_filePath[0] = L'\0';
            sourceLine.m_rva = rva;
            ForeachCompilationUnit(&DwarfSymbolEngine::TraverseSourceLineInfo, &sourceLine);
            sourceLine.m_rva = pFuncInfo->m_rva;

            if (L'\0' != sourceLine.m_filePath[0])
            {
                ret = true;
            }
        }
    }

    return ret;
}

gtByte* DwarfSymbolEngine::AllocateBuffer(unsigned size)
{
    gtByte** pNewBuffer = reinterpret_cast<gtByte**>(new gtByte[sizeof(gtByte**) + size]);
    *pNewBuffer = m_pBuffers ? *m_pBuffers : NULL;
    m_pBuffers = pNewBuffer;
    return reinterpret_cast<gtByte*>(pNewBuffer + 1);
}

void DwarfSymbolEngine::Clear(bool deinit)
{
    while (NULL != m_pBuffers)
    {
        gtByte* pAllocBuffer = reinterpret_cast<gtByte*>(m_pBuffers);
        m_pBuffers = reinterpret_cast<gtByte**>(*m_pBuffers);
        delete [] pAllocBuffer;
    }

    if (NULL != m_dbg)
    {
        if (deinit)
        {
            _dwarf_deinit(m_dbg);
        }

        if (IsElf())
        {
            _dwarf_elf_deinit(m_dbg);
        }
        else
        {
            delete m_dbg->dbg_iface;
        }

        free(m_dbg);
        m_dbg = NULL;
    }

    if (NULL != m_pFuncsInfoVec)
    {
        ClearFunctionsInfo();
    }

    if (NULL != m_pAuxSymEngine)
    {
        delete m_pAuxSymEngine;
    }

    if (true == m_processInlineSamples)
    {
        ClearInlinedFunctionInfo();
    }
}

void DwarfSymbolEngine::ClearInlinedFunctionInfo()
{
    if (!m_inlinedFuncsInfo.empty())
    {
        for (FunctionsMap::iterator it = m_inlinedFuncsInfo.begin();
             it != m_inlinedFuncsInfo.end(); ++it)
        {
            if (NULL != it->first.m_pName)
            {
                delete[] it->first.m_pName;
            }

            if (NULL != it->first.m_addrRanges)
            {
                delete it->first.m_addrRanges;
            }
        }

        m_inlinedFuncsInfo.clear();
    }

    if (m_processedInlineInfo.empty())
    {
        for (FunctionsMap::iterator it = m_processedInlineInfo.begin();
             it != m_processedInlineInfo.end(); ++it)
        {
            if (NULL != it->first.m_pName)
            {
                delete[] it->first.m_pName;
            }
        }

        m_processedInlineInfo.clear();
    }
}

bool DwarfSymbolEngine::IsElf() const
{
    return (this != m_dbg->dbg_iface->object);
}

const FunctionSymbolInfo* DwarfSymbolEngine::LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    const FunctionSymbolInfo* pFunc = DwarfSymbolEngine::LookupBoundingFunction(rva, pNextRva, handleInline);

    if (NULL != pFunc)
    {
        if (0 == pFunc->m_size)
        {
            if (m_pExe->LookupSectionIndex(pFunc->m_rva) != m_pExe->LookupSectionIndex(rva))
            {
                pFunc = NULL;
            }
        }
        else if (false == handleInline && (pFunc->m_rva + pFunc->m_size) <= rva)
        {
            pFunc = NULL;
        }
    }

    return pFunc;
}

const FunctionSymbolInfo* DwarfSymbolEngine::LookupInlinedFunction(gtRVAddr rva) const
{
    const FunctionSymbolInfo* pFunc = NULL;
    FunctionSymbolInfo funcInfo;
    funcInfo.m_pName = NULL;
    funcInfo.m_rva = rva;
    funcInfo.m_addrRanges = NULL;
    bool found = false;

    if (m_processInlineSamples)
    {
        // first, check if we have already processed this RVA for inline info
        FunctionsMap::iterator it = m_processedInlineInfo.upper_bound(funcInfo);

        if (m_processedInlineInfo.begin() != it)
        {
            --it;

            if (it->first.m_rva <= rva && rva < (it->first.m_rva + it->first.m_size))
            {
                // we have already processed this RVA
                found = true;
            }
        }

        // If we have not processed this RVA for inlined info, then process now
        if (!found)
        {
            // Search for inlined info for this RVA
            found = ProcessInlinedFunctionByRva(rva);
        }

        // if processed, then search RVA in the inlined functions map
        if (found)
        {
            FunctionsMap::iterator mit = m_inlinedFuncsInfo.upper_bound(funcInfo);

            while (m_inlinedFuncsInfo.begin() != mit)
            {
                --mit;

                if (mit->first.m_rva <= rva)
                {
                    bool matchFound = false;

                    if (NULL != mit->first.m_addrRanges)
                    {
                        // do a linear search in the addr range vector
                        for (auto& itr : * (mit->first.m_addrRanges))
                        {
                            if (itr.m_rvaStart <= rva && rva < itr.m_rvaEnd)
                            {
                                matchFound = true;
                                break;
                            }

                            // no further search needed
                            if (rva < itr.m_rvaStart)
                            {
                                break;
                            }
                        }
                    }

                    if (!matchFound)
                    {
                        if (rva < (mit->first.m_rva + mit->first.m_size))
                        {
                            matchFound = true;
                        }
                    }

                    if (matchFound)
                    {
                        if (mit->second)
                        {
                            pFunc = &mit->first;
                        }
                        else
                        {
                            pFunc = FindAggrInlineInstance(mit->first.m_pName);
                        }

                        break;
                    }
                }
                else
                {
                    // no more functions would match
                    break;
                }
            }
        }
    }

    return pFunc;
}

bool DwarfSymbolEngine::ProcessInlinedFunctionByRva(gtRVAddr rva) const
{
    bool ret = false;
    Dwarf_Addr addr = static_cast<Dwarf_Addr>(rva) + m_pExe->GetImageBase();
    Dwarf_Die cuDie = NULL;
    Dwarf_Addr cuAddrLow = 0;

    ACQUIRE_DWARF_LOCK(m_dwarfLock);
    InitCompilationUnit(m_dbg);

    // iterate over CUs till we find matching CU
    while (dwarf_next_cu_header(m_dbg, NULL, NULL, NULL, NULL, NULL, NULL) == DW_DLV_OK)
    {
        // get the first DIE of current CU
        if (dwarf_siblingof(m_dbg, NULL, &cuDie, NULL) != DW_DLV_OK)
        {
            break;
        }

        Dwarf_Half tag;

        if (dwarf_tag(cuDie, &tag, NULL) != DW_DLV_OK)
        {
            break;
        }

        if (DW_TAG_compile_unit == tag)
        {
            // get highpc and lowpc
            Dwarf_Addr addrLow = 0, addrHigh = 0;

            if (DieFindLowHighAddress(cuDie, tag, cuAddrLow, addrLow, addrHigh, NULL))
            {
                if (addrLow < addrHigh && addrLow <= addr && addr < addrHigh)
                {
                    ret = true;
                    break;
                }
            }
        }
    }

    // iterate over subprograms till we find matching subprogram
    if (ret)
    {
        Dwarf_Die die = NULL;

        if (dwarf_child(cuDie, &die, NULL) == DW_DLV_OK)
        {
            int r;

            do
            {
                Dwarf_Half tag;

                if (dwarf_tag(die, &tag, NULL) != DW_DLV_OK)
                {
                    ret = false;
                    break;
                }

                if (DW_TAG_subprogram == tag)
                {
                    Dwarf_Addr addrLow = 0, addrHigh = 0;

                    if (DieFindLowHighAddress(die, tag, cuAddrLow, addrLow, addrHigh, NULL))
                    {
                        if (addrLow < addrHigh && addrLow <= addr && addr < addrHigh)
                        {
                            // Process inlined information present in this subprogram
                            ret = ProcessInlinedFunctionInfo(die, cuAddrLow);

                            if (ret)
                            {
                                // Update map that we have processed current function for inline information
                                FunctionSymbolInfo funcInfo;
                                FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U;)
                                funcInfo.m_addrRanges = NULL;
                                funcInfo.m_rva = static_cast<gtRVAddr>(addrLow - m_pExe->GetImageBase());
                                funcInfo.m_size = static_cast<gtUInt32>(addrHigh - addrLow);
                                funcInfo.m_pName = NULL;    // no need to store function name
                                m_processedInlineInfo.insert(FunctionsMap::value_type(funcInfo, true));
                                break;
                            }
                        }
                    }
                }

                Dwarf_Die siblingDie = NULL;
                r = dwarf_siblingof(m_dbg, die, &siblingDie, NULL);
                dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
                die = siblingDie;
                ret = DW_DLV_ERROR != r;

            }
            while (DW_DLV_OK == r);
        }
    }

    if (NULL != cuDie)
    {
        dwarf_dealloc(m_dbg, cuDie, DW_DLA_DIE);
    }

    return ret;
}

static wchar_t* GetDemangledName(Dwarf_Debug dbg, Dwarf_Die die)
{
    wchar_t* pName = NULL;

    // Getting DW_AT_linkage_name attribute; originally called DW_AT_MIPS_linkage_name in pre-DWARF4.
    Dwarf_Attribute attr;

    if (dwarf_attr(die, DW_AT_linkage_name , &attr, NULL) == DW_DLV_OK ||
        dwarf_attr(die, DW_AT_MIPS_linkage_name , &attr, NULL) == DW_DLV_OK)
    {
        char* pMangledName;

        if ((dwarf_formstring(attr, &pMangledName, NULL) == DW_DLV_OK) && (NULL != pMangledName))
        {
            pName = SymbolEngine::DemangleNameIA(pMangledName);
            dwarf_dealloc(dbg, pMangledName, DW_DLA_STRING);
        }

        dwarf_dealloc(dbg, attr, DW_DLA_ATTR);
    }

    char* pDieName;

    if (NULL == pName && dwarf_diename(die, &pDieName, NULL) == DW_DLV_OK)
    {
        pName = SymbolEngine::DemangleNameIA(pDieName);
        dwarf_dealloc(dbg, pDieName, DW_DLA_STRING);
    }

    return pName;
}

static wchar_t* ExtractSubprogramName(Dwarf_Debug dbg, Dwarf_Die die)
{
    wchar_t* pName = GetDemangledName(dbg, die);

    if (NULL == pName)
    {
        // In some case, the subprogram DIE does not have name,
        // but instead the DW_AT_abstract_origin, or DW_AT_specification attribute.

        Dwarf_Die offDie = DieFindAttribute(dbg, die, DW_AT_abstract_origin);

        if (NULL != offDie)
        {
            pName = GetDemangledName(dbg, offDie);

            if (NULL == pName)
            {
                Dwarf_Die spcDie = DieFindAttribute(dbg, offDie, DW_AT_specification);

                if (NULL != spcDie)
                {
                    pName = GetDemangledName(dbg, spcDie);
                    dwarf_dealloc(dbg, spcDie, DW_DLA_DIE);
                }
            }

            dwarf_dealloc(dbg, offDie, DW_DLA_DIE);
        }

        if (NULL == pName)
        {
            offDie = DieFindAttribute(dbg, die, DW_AT_specification);

            if (NULL != offDie)
            {
                pName = GetDemangledName(dbg, offDie);
                dwarf_dealloc(dbg, offDie, DW_DLA_DIE);
            }
        }
    }

    return pName;
}

static Dwarf_Die DieFindAttribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half nattr)
{
    Dwarf_Die offDie = NULL;

    Dwarf_Attribute attr;

    if (dwarf_attr(die, nattr , &attr, NULL) == DW_DLV_OK)
    {
        // Get the global offset
        Dwarf_Off globalOff;

        if (dwarf_global_formref(attr, &globalOff, NULL) == DW_DLV_OK)
        {
            dwarf_offdie(dbg, globalOff, &offDie, NULL);
        }

        dwarf_dealloc(dbg, attr, DW_DLA_ATTR);
    }

    return offDie;
}

static void InitCompilationUnit(Dwarf_Debug dbg)
{
    dbg->dbg_cu_current = NULL;
}


static int DWARF_API Access_get_section_info(void* pObj, Dwarf_Half index, Dwarf_Obj_Access_Section* pSection, int* pError)
{
    DwarfSymbolEngine* pEngine = static_cast<DwarfSymbolEngine*>(pObj);
    assert(NULL != pEngine);

    const ExecutableFile* pExe = pEngine->GetExecutable();
    assert(NULL != pExe);

    int ret;

    if (NULL != pSection)
    {
        unsigned sectionIndex = static_cast<unsigned>(index);

        unsigned nameLength;
        const char* pName = pExe->GetSectionName(sectionIndex, &nameLength);

        if (NULL != pName)
        {
            if ('\0' != pName[nameLength] || (8 == nameLength && 0 == memcmp(pName, ".eh_frame", 8)))
            {
                const unsigned isEhFrame = static_cast<unsigned>(0 == memcmp(pName, ".eh_frame", 8));

                char* pZtName = pEngine->AllocateBuffer(nameLength + isEhFrame + 1U);

                memcpy(pZtName, pName, nameLength);
                pZtName[nameLength] = 'e';
                pZtName[nameLength + isEhFrame] = '\0';
                pName = pZtName;
            }

            gtRVAddr startRva, endRva;
            pExe->GetSectionRvaLimits(sectionIndex, startRva, endRva);
            pSection->addr = static_cast<Dwarf_Addr>(pExe->RvaToVa(startRva));
            pSection->size = static_cast<Dwarf_Unsigned>(pExe->GetSectionSize(sectionIndex));
            pSection->name = pName;

            ret = DW_DLV_OK;
        }
        else
        {
            if (NULL != pError)
            {
                *pError = DW_DLE_NO_ENTRY;
            }

            ret = DW_DLV_NO_ENTRY;
        }
    }
    else
    {
        if (NULL != pError)
        {
            *pError = DW_DLE_ARGUMENT;
        }

        ret = DW_DLV_ERROR;
    }

    return ret;
}

static Dwarf_Endianness DWARF_API Access_get_byte_order(void* pObj)
{
    (void)pObj; // Unused
    return DW_OBJECT_LSB;
}

static Dwarf_Small DWARF_API Access_get_length_size(void* pObj)
{
    (void)pObj; // Unused
    return 4U;
}

static Dwarf_Small DWARF_API Access_get_pointer_size(void* pObj)
{
    DwarfSymbolEngine* pEngine = static_cast<DwarfSymbolEngine*>(pObj);
    assert(NULL != pEngine);

    const ExecutableFile* pExe = pEngine->GetExecutable();
    assert(NULL != pExe);

    return 4U << static_cast<unsigned>(pExe->Is64Bit());
}

static Dwarf_Unsigned DWARF_API Access_get_section_count(void* pObj)
{
    DwarfSymbolEngine* pEngine = static_cast<DwarfSymbolEngine*>(pObj);
    assert(NULL != pEngine);

    const ExecutableFile* pExe = pEngine->GetExecutable();
    assert(NULL != pExe);

    return static_cast<Dwarf_Unsigned>(pExe->GetSectionsCount());
}

static int DWARF_API Access_load_section(void* pObj, Dwarf_Half index, Dwarf_Small** ppData, int* pError)
{
    DwarfSymbolEngine* pEngine = static_cast<DwarfSymbolEngine*>(pObj);
    assert(NULL != pEngine);

    const ExecutableFile* pExe = pEngine->GetExecutable();
    assert(NULL != pExe);

    int ret;

    if (NULL != ppData)
    {
        const gtUByte* pBytes = pExe->GetSectionBytes(static_cast<unsigned>(index));

        if (NULL != pBytes)
        {
            *ppData = const_cast<gtUByte*>(pBytes);
            ret = DW_DLV_OK;
        }
        else
        {
            if (NULL != pError)
            {
                *pError = DW_DLE_NO_ENTRY;
            }

            ret = DW_DLV_NO_ENTRY;
        }
    }
    else
    {
        if (NULL != pError)
        {
            *pError = DW_DLE_ARGUMENT;
        }

        ret = DW_DLV_ERROR;
    }

    return ret;
}

bool DwarfSymbolEngine::DieFindLowHighAddress(Dwarf_Die die, Dwarf_Half tag, Dwarf_Addr& cuAddrLow, Dwarf_Addr& addrLow, Dwarf_Addr& addrHigh, gtVector<FuncAddressRange>* addrRanges) const
{
    bool ret = false;
    Dwarf_Attribute attrRange;

    // if lowpc, highpc attribute present
    if (dwarf_lowpc(die, &addrLow, NULL) == DW_DLV_OK &&
        dwarf_highpc(die, &addrHigh, NULL) == DW_DLV_OK)
    {
        if (addrLow > addrHigh)
        {
            addrHigh += addrLow;
        }

        if (DW_TAG_compile_unit == tag)
        {
            cuAddrLow = addrLow;
        }

        ret = true;
    }
    // else if ranges attribute present
    else if (dwarf_attr(die, DW_AT_ranges, &attrRange, NULL) == DW_DLV_OK)
    {
        Dwarf_Off offset = 0;
        Dwarf_Unsigned offsetData = 0;
        Dwarf_Ranges* pRanges = NULL;
        Dwarf_Signed count = 0;

        // Before DWARF4, DW_FORM_data{4,8} was used for rangelistptr,
        // which can be fetched by dwarf_formudata()
        // Since DWARF4, DW_FORM_sec_offset is used for rangelistptr,
        // which can be fetched by dwarf_global_formref()
        // Try first with global_formref(), if fails then try with formudata()
        if ((dwarf_global_formref(attrRange, &offset, NULL) == DW_DLV_OK &&
             dwarf_get_ranges_a(m_dbg, offset, die, &pRanges, &count, NULL, NULL) == DW_DLV_OK) ||
            (dwarf_formudata(attrRange, &offsetData, NULL) == DW_DLV_OK &&
             dwarf_get_ranges_a(m_dbg, static_cast<Dwarf_Off>(offsetData), die, &pRanges, &count, NULL, NULL) == DW_DLV_OK))
        {
            if (DW_TAG_compile_unit == tag)
            {
                // for compile_unit DIE, read lowpc to get the base address
                if (dwarf_lowpc(die, &cuAddrLow, NULL) != DW_DLV_OK)
                {
                    // if failed, set image base as base address
                    cuAddrLow = m_pExe->GetImageBase();
                }
            }

            Dwarf_Addr offsetLow  = 0xFFFFFFFFFFFFFFFFULL;
            Dwarf_Addr offsetHigh = 0ULL;

            // compare the saved entry with the rest of the entris to find low/high offsets
            int i = 0;

            while (i < count)
            {
                if (DW_RANGES_ENTRY == pRanges[i].dwr_type)
                {
                    Dwarf_Addr newLow  = pRanges[i].dwr_addr1;
                    Dwarf_Addr newHigh = pRanges[i].dwr_addr2;
                    offsetLow  = (offsetLow > newLow) ? newLow : offsetLow;
                    offsetHigh = (offsetHigh < newHigh) ? newHigh : offsetHigh;

                    if (NULL != addrRanges)
                    {
                        gtRVAddr rva1 = static_cast<gtRVAddr>(newLow + cuAddrLow - m_pExe->GetImageBase());
                        gtRVAddr rva2 = static_cast<gtRVAddr>(newHigh + cuAddrLow - m_pExe->GetImageBase());
                        FuncAddressRange ar(rva1, rva2);
                        addrRanges->push_back(ar);
                    }
                }

                i++;
            }

            addrLow = offsetLow + cuAddrLow;
            addrHigh = offsetHigh + cuAddrLow;

            dwarf_ranges_dealloc(m_dbg, pRanges, count);
            ret = true;
        }

        dwarf_dealloc(m_dbg, attrRange, DW_DLA_ATTR);
    }

    return ret;
}

void DwarfSymbolEngine::UpdateInlinedFunctionName(FunctionSymbolInfo& funcInfo) const
{
    if (true == m_processInlineSamples)
    {
        gtString inlinedName = L"[inlined] ";
        inlinedName += funcInfo.m_pName;
        delete[] funcInfo.m_pName;
        size_t len = wcslen(inlinedName.asCharArray());
        funcInfo.m_pName = new wchar_t[len + 1];
        wcscpy(funcInfo.m_pName, inlinedName.asCharArray());
        funcInfo.m_pName[len] = L'\0';
    }
}

gtRVAddr DwarfSymbolEngine::TranslateToInlineeRVA(gtRVAddr rva) const
{
    FunctionSymbolInfo funcInfo;
    funcInfo.m_pName = NULL;
    funcInfo.m_rva = rva;

    const FunctionSymbolInfo* pFunc = NULL;
    const FunctionSymbolInfo* pMatchFunc = NULL;

    FunctionsMap::iterator mit = m_inlinedFuncsInfo.upper_bound(funcInfo);

    while (m_inlinedFuncsInfo.begin() != mit)
    {
        --mit;

        if (mit->first.m_rva <= rva)
        {
            bool matchFound = false;

            if (NULL != mit->first.m_addrRanges)
            {
                // do a linear search in the addr range vector
                for (auto& it : * (mit->first.m_addrRanges))
                {
                    // matching range found
                    if (it.m_rvaStart <= rva && rva < it.m_rvaEnd)
                    {
                        matchFound = true;
                        break; // out of for loop
                    }

                    // no further search needed
                    if (rva < it.m_rvaStart)
                    {
                        break; // out of for loop
                    }
                }
            }

            if (!matchFound)
            {
                if (rva < (mit->first.m_rva + mit->first.m_size))
                {
                    matchFound = true;
                }
            }

            if (matchFound)
            {
                pMatchFunc = &mit->first;

                if (mit->second)
                {
                    pFunc = pMatchFunc;
                }
                else
                {
                    pFunc = FindAggrInlineInstance(mit->first.m_pName);
                }

                break; // out of while loop
            }
        }
        else
        {
            // no more functions would match
            break;
        }
    }

    gtRVAddr funcRva = rva;

    if (NULL != pFunc && pFunc != pMatchFunc)
    {
        funcRva = pFunc->m_rva;
        gtRVAddr offset = 0;

        if (NULL != pMatchFunc->m_addrRanges)
        {
            // calculate offset from the ranges
            for (auto& it : * (pMatchFunc->m_addrRanges))
            {
                if (rva < it.m_rvaStart)
                {
                    break;
                }

                if (rva < it.m_rvaEnd)
                {
                    offset += rva - it.m_rvaStart;
                }
                else
                {
                    offset += it.m_rvaEnd - it.m_rvaStart;
                }
            }
        }
        else
        {
            offset = rva - pMatchFunc->m_rva;
        }

        if (NULL != pFunc->m_addrRanges)
        {
            for (auto& it : * (pFunc->m_addrRanges))
            {
                if (offset >= (it.m_rvaEnd - it.m_rvaStart))
                {
                    offset -= (it.m_rvaEnd - it.m_rvaStart);
                }
                else
                {
                    funcRva = it.m_rvaStart + offset;
                    break;
                }
            }
        }
        else
        {
            funcRva += offset;
        }
    }

    return funcRva;
}

//TODO: Optimize the linear search.
const FunctionSymbolInfo* DwarfSymbolEngine::FindAggrInlineInstance(const wchar_t* name) const
{
    const FunctionSymbolInfo* pFunc = NULL;
    FunctionsMap::iterator firstIt = m_inlinedFuncsInfo.end();

    for (FunctionsMap::iterator it = m_inlinedFuncsInfo.begin();
         it != m_inlinedFuncsInfo.end(); ++it)
    {
        if (0 == wcscmp(name, it->first.m_pName))
        {
            if (firstIt == m_inlinedFuncsInfo.end())
            {
                firstIt = it;
            }

            if (true == it->second)
            {
                pFunc = &it->first;
                break;
            }
        }
    }

    // if the instance is choosen first time, then mark the instance
    if (NULL == pFunc && m_inlinedFuncsInfo.end() != firstIt)
    {
        firstIt->second = true;
        pFunc = &firstIt->first;
    }

    return pFunc;
}

// This function is called only when rva is an inlined rva.
gtVector<gtRVAddr> DwarfSymbolEngine::FindNestedInlineFunctions(gtRVAddr rva) const
{
    gtVector<gtRVAddr> list;
    auto it = m_nestedFuncMap.find(rva);

    // LookupInlinedFunction() can be replaced with a smipler new function IsRvaInlined()
    if (m_nestedFuncMap.end() == it && LookupInlinedFunction(rva) != NULL)
    {
        // rva not addded to map. Add it now
        ProcessNestedInlinedFunctionInfo(rva);

        // search the map again after processing
        it = m_nestedFuncMap.find(rva);
    }

    if (m_nestedFuncMap.end() != it)
    {
        // copy the pre-processed list
        list = it->second;
    }
    else
    {
        // failed to find any list. just return the same rva back.
        list.push_back(rva);
    }

    return list;
}

// This function is called only when rva is not added to map.
bool DwarfSymbolEngine::ProcessNestedInlinedFunctionInfo(gtRVAddr rva) const
{
    bool ret = false;
    gtVector<gtRVAddr> list;
    Dwarf_Addr addr = static_cast<Dwarf_Addr>(rva) + m_pExe->GetImageBase();
    Dwarf_Die cuDie = NULL;
    Dwarf_Addr cuAddrLow = 0;

    ACQUIRE_DWARF_LOCK(m_dwarfLock);
    InitCompilationUnit(m_dbg);

    // iterate over CUs till we find matching CU
    while (dwarf_next_cu_header(m_dbg, NULL, NULL, NULL, NULL, NULL, NULL) == DW_DLV_OK)
    {
        // get the first DIE of current CU
        if (dwarf_siblingof(m_dbg, NULL, &cuDie, NULL) != DW_DLV_OK)
        {
            continue;
        }

        Dwarf_Half tag;

        if (dwarf_tag(cuDie, &tag, NULL) != DW_DLV_OK)
        {
            break;
        }

        if (DW_TAG_compile_unit == tag)
        {
            // get highpc and lowpc
            Dwarf_Addr addrLow = 0, addrHigh = 0;

            if (DieFindLowHighAddress(cuDie, tag, cuAddrLow, addrLow, addrHigh, NULL))
            {
                if (addrLow <= addr && addr < addrHigh)
                {
                    ret = true;
                    break;
                }
            }
        }
    }

    bool possibleLeafFuncFound = false;

    // Iterate over subprograms, inlined_subroutines till we find matching inlined_subroutine
    // Traverse the DIE tree from top subprogram till leaf/last inlined function
    // Add the last non-inlined caller function and rest inlined callee functions into the vector
    if (ret)
    {
        Dwarf_Die die = NULL;

        if (dwarf_child(cuDie, &die, NULL) == DW_DLV_OK)
        {
            int r;

            do
            {
                bool visitChild = false;
                Dwarf_Half tag;

                if (dwarf_tag(die, &tag, NULL) != DW_DLV_OK)
                {
                    break;
                }

                if (DW_TAG_subprogram == tag || DW_TAG_lexical_block == tag || DW_TAG_inlined_subroutine == tag)
                {
                    Dwarf_Addr addrLow = 0, addrHigh = 0;

                    if (DieFindLowHighAddress(die, tag, cuAddrLow, addrLow, addrHigh, NULL))
                    {
                        // Need to add check in addr-ranges here for RVA matching later
                        if (addrLow <= addr && addr < addrHigh)
                        {
                            if (DW_TAG_subprogram == tag)
                            {
                                // reached a non-inlined caller function
                                // clear if anything already added to list
                                list.clear();
                                gtRVAddr rva1 = static_cast<gtRVAddr>(addrLow - m_pExe->GetImageBase());
                                list.push_back(rva1);
                            }
                            else if (DW_TAG_inlined_subroutine == tag)
                            {
                                // reached a inlined callee function instance
                                gtRVAddr rva1 = static_cast<gtRVAddr>(addrLow - m_pExe->GetImageBase());
                                rva1 = TranslateToInlineeRVA(rva1);
                                list.push_back(rva1);
                                possibleLeafFuncFound = true;
                            }

                            visitChild = true;
                        }
                    }
                }

                Dwarf_Die nextDie = NULL;

                if (visitChild)
                {
                    r = dwarf_child(die, &nextDie, NULL);
                }
                else
                {
                    r = dwarf_siblingof(m_dbg, die, &nextDie, NULL);
                }

                dwarf_dealloc(m_dbg, die, DW_DLA_DIE);
                die = nextDie;
            }
            while (DW_DLV_OK == r);
        }
    }

    if (NULL != cuDie)
    {
        dwarf_dealloc(m_dbg, cuDie, DW_DLA_DIE);
    }

    if (possibleLeafFuncFound)
    {
        m_nestedFuncMap[rva] = list;
    }

    return true;
}
