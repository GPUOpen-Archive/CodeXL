//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ElfSymbolEngine.cpp
/// \brief This file contains the class for querying ELF symbols.
///
//==================================================================================

#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <gelf.h>
#include "ElfSymbolEngine.h"
#include "ElfFile.h"

ElfSymbolEngine::ElfSymbolEngine() : m_pElf(NULL)
{
}

ElfSymbolEngine::~ElfSymbolEngine()
{
}

bool ElfSymbolEngine::Initialize(const ElfFile& elf)
{
    InitializeFunctionsInfo();

    m_pElf = &elf;

    size_t shstrndx;

    if (0 != elf_getshdrstrndx(elf.m_pElf, &shstrndx))
    {
        return false;
    }

    bool ret = false;
    bool first = true;
    Elf_Scn* pScn = NULL;
    // Start address and size of .plt section
    gtRVAddr pltStart = 0;
    gtUInt32 pltSize = 0;
    // one entry size in .plt section
    gtUInt32 pltEsize = 0;
    // section index for .rel.plt, .rela.plt, .dynsym
    size_t relpltScnIndex = 0;
    size_t relapltScnIndex = 0;
    size_t dynsymScnIndex = 0;

    while (NULL != (pScn = elf_nextscn(elf.m_pElf, pScn)))
    {
        GElf_Shdr shdr;

        if (NULL != gelf_getshdr(pScn, &shdr))
        {
            // look for .plt section
            if (SHT_PROGBITS == shdr.sh_type)
            {
                char* name = elf_strptr(elf.m_pElf, shstrndx, shdr.sh_name);

                if (NULL != name && 0 == strcmp(name, ".plt"))
                {
                    pltStart = static_cast<gtRVAddr>(shdr.sh_addr - elf.m_imageBase);
                    pltSize  = static_cast<gtUInt32>(shdr.sh_size);
                    // Sometimes pltEsize is wrong. The known size of
                    // one .plt entry is 16 bytes for both x86 and x64
                    // Ex: pltEsize = static_cast<gtUInt32>(shdr.sh_entsize);
                    pltEsize = 16;
                }

                continue;
            }

            // look for .rel.plt section
            if (SHT_REL == shdr.sh_type)
            {
                char* name = elf_strptr(elf.m_pElf, shstrndx, shdr.sh_name);

                if (NULL != name && 0 == strcmp(name, ".rel.plt"))
                {
                    relpltScnIndex = elf_ndxscn(pScn);
                }

                continue;
            }

            // look for .rela.plt section
            if (SHT_RELA == shdr.sh_type)
            {
                char* name = elf_strptr(elf.m_pElf, shstrndx, shdr.sh_name);

                if (NULL != name && 0 == strcmp(name, ".rela.plt"))
                {
                    relapltScnIndex = elf_ndxscn(pScn);
                }

                continue;
            }

            // look for .dynsym section
            if (SHT_DYNSYM == shdr.sh_type)
            {
                dynsymScnIndex = elf_ndxscn(pScn);
            }

            if (SHT_SYMTAB == shdr.sh_type || SHT_DYNSYM == shdr.sh_type)
            {
                Elf_Data* pData = elf_getdata(pScn, NULL);

                if (NULL == pData || 0 == pData->d_size)
                {
                    // Error - skip this section
                    continue;
                }

                gtSize_t symCount = static_cast<gtSize_t>(shdr.sh_size / shdr.sh_entsize);

                for (gtSize_t i = 0; i < symCount; ++i)
                {
                    GElf_Sym sym;

                    if (NULL != gelf_getsym(pData, static_cast<int>(i), &sym) && STT_FUNC == GELF_ST_TYPE(sym.st_info))
                    {
                        // At least this is not a stripped object
                        ret = true;

                        if (SHN_UNDEF == sym.st_shndx)
                        {
                            // Undefined symbol
                            continue;
                        }

                        char* pName = elf_strptr(elf.m_pElf, shdr.sh_link, sym.st_name);

                        if (NULL != pName && '\0' != pName[0])
                        {
                            FunctionSymbolInfo funcInfo;
                            FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U;)
                            funcInfo.m_rva = static_cast<gtRVAddr>(sym.st_value - elf.m_imageBase);
                            funcInfo.m_size = static_cast<gtUInt32>(sym.st_size);

                            // If we have already read the symbol table of this ELF file.
                            //
                            // Currently, this can only happen if the ELF file has both static and dynamic symbol tables.
                            if (!first)
                            {
                                bool duplicateSymbol = false;

                                for (gtVector<FunctionSymbolInfo>::iterator it = m_pFuncsInfoVec->begin(), itEnd = m_pFuncsInfoVec->end();
                                     it != itEnd; ++it)
                                {
                                    if (it->m_rva == funcInfo.m_rva)
                                    {
                                        if (it->m_size < funcInfo.m_size)
                                        {
                                            wchar_t* pNewName = DemangleNameIA(pName);

                                            if (NULL != pNewName)
                                            {
                                                delete [] it->m_pName;
                                                it->m_pName = pNewName;
                                                it->m_size = funcInfo.m_size;
                                            }
                                        }

                                        duplicateSymbol = true;
                                        break;
                                    }
                                }

                                if (duplicateSymbol)
                                {
                                    // We already have this symbol from the static symbol table
                                    continue;
                                }
                            }

                            funcInfo.m_pName = DemangleNameIA(pName);

                            if (NULL != funcInfo.m_pName)
                            {
                                funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);
                                m_pFuncsInfoVec->push_back(funcInfo);
                            }
                        }
                    }
                }

                first = false;
            }
        }
    }

    // insert each .plt entry as one dummy function into the functions list
    bool success = false;

    if ((SHN_UNDEF != relpltScnIndex || SHN_UNDEF != relapltScnIndex) && SHN_UNDEF != dynsymScnIndex)
    {
        // Theoretically both .rel.plt and .rela.plt can exist in a single elf file.
        // But usually, compilers generate .rel.plt for x86 systems and .rela.plt for x64 systems.
        // Here assumed that both don't exist in the same elf file to simplify the implementation.

        // process .rel.plt section, usually present in 32-bit ELF binary
        if (relpltScnIndex != SHN_UNDEF && relapltScnIndex == SHN_UNDEF)
        {
            success = ProcessRelPltSection(pltStart, pltSize, pltEsize, relpltScnIndex, dynsymScnIndex);
        }
        // process .rela.plt section, usually present in 64-bit ELF binary
        else if (relapltScnIndex != SHN_UNDEF && relpltScnIndex == SHN_UNDEF)
        {
            success = ProcessRelaPltSection(pltStart, pltSize, pltEsize, relapltScnIndex, dynsymScnIndex);
        }
    }

    // if failed to process .plt entries, use whole .plt section as one dummy function
    if (!success)
    {
        FunctionSymbolInfo funcInfo;
        FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U);
        funcInfo.m_addrRanges = NULL;
        funcInfo.m_rva = pltStart;
        funcInfo.m_size = pltSize;
        funcInfo.m_pName = new wchar_t[6] {'[', 'P', 'L', 'T', ']', '\0'};
        funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);

        m_pFuncsInfoVec->push_back(funcInfo);
    }

    gtSort(m_pFuncsInfoVec->begin(), m_pFuncsInfoVec->end());

    return ret;
}

const FunctionSymbolInfo* ElfSymbolEngine::LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    const FunctionSymbolInfo* pFunc = ElfSymbolEngine::LookupBoundingFunction(rva, pNextRva, handleInline);

    if (NULL != pFunc)
    {
        if (0 == pFunc->m_size)
        {
            if (m_pElf->LookupSectionIndex(pFunc->m_rva) != m_pElf->LookupSectionIndex(rva))
            {
                pFunc = NULL;
            }
        }
        else if ((pFunc->m_rva + pFunc->m_size) <= rva)
        {
            pFunc = NULL;
        }
    }

    return pFunc;
}

bool ElfSymbolEngine::EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(pSourceFilePath);
    GT_UNREFERENCED_PARAMETER(srcLineInstanceMap);
    GT_UNREFERENCED_PARAMETER(handleInline);
    return false;
}

bool ElfSymbolEngine::FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(rva);
    GT_UNREFERENCED_PARAMETER(sourceLine);
    GT_UNREFERENCED_PARAMETER(handleInline);
    return false;
}

bool ElfSymbolEngine::HasSourceLineInfo() const
{
    return false;
}

bool ElfSymbolEngine::ProcessRelPltSection(
    gtRVAddr pltStart,
    gtUInt32 pltSize,
    gtUInt32 pltEsize,
    size_t relpltScnIndex,
    size_t dynsymScnIndex)
{
    bool result = false;
    int pltEcount = pltSize / pltEsize;

    // fetch pointer to .rel.plt data
    Elf_Scn* pScn = elf_getscn(m_pElf->m_pElf, relpltScnIndex);
    Elf_Data* pRelData = elf_getdata(pScn, NULL);

    // fetch pointer to .dymsym data
    pScn = elf_getscn(m_pElf->m_pElf, dynsymScnIndex);
    Elf_Data* pDynsymData = elf_getdata(pScn, NULL);

    GElf_Shdr shdr;

    if (NULL != pRelData && NULL != pDynsymData && NULL != gelf_getshdr(pScn, &shdr))
    {
        // skip first entry of .plt, it is for link resolver
        for (int i = 1; i < pltEcount; i++)
        {
            GElf_Rel rel;
            GElf_Sym sym;

            // fetch relocation data for corresponding .plt entry
            if (NULL == gelf_getrel(pRelData, (i - 1), &rel))
            {
                continue;
            }

            // fetch index for corresponding .dynsym entry
            gtUInt32 index = GELF_R_SYM(rel.r_info);

            // fetch .dynsym entry for the index
            if (NULL != gelf_getsym(pDynsymData, index, &sym))
            {
                // fetch name of the symbol of corresponding .plt entry
                char* pName = elf_strptr(m_pElf->m_pElf, shdr.sh_link, sym.st_name);

                if (NULL == pName || '\0' == pName[0])
                {
                    continue;
                }

                // demangle the symbol name and prepend [PLT]
                wchar_t* dmName = DemangleNameIA(pName);
                size_t len = 6 + wcslen(dmName) + 1;
                wchar_t* funcName = new wchar_t[len]();
                wcscpy(funcName, L"[PLT] ");
                wcscat(funcName, dmName);

                // insert the dummy plt entry to the functions vector
                FunctionSymbolInfo funcInfo;
                FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U);
                funcInfo.m_addrRanges = NULL;
                funcInfo.m_rva = pltStart + (pltEsize * i);
                funcInfo.m_size = pltEsize + 1;
                funcInfo.m_pName = funcName;
                funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);

                m_pFuncsInfoVec->push_back(funcInfo);
                result = true;
            }
        }
    }

    return result;
}

bool ElfSymbolEngine::ProcessRelaPltSection(
    gtRVAddr pltStart,
    gtUInt32 pltSize,
    gtUInt32 pltEsize,
    size_t relapltScnIndex,
    size_t dynsymScnIndex)
{
    bool result = false;
    int pltEcount = pltSize / pltEsize;

    // fetch pointer to .rela.plt data
    Elf_Scn* pScn = elf_getscn(m_pElf->m_pElf, relapltScnIndex);
    Elf_Data* pRelaData = elf_getdata(pScn, NULL);

    // fetch pointer to .dymsym data
    pScn = elf_getscn(m_pElf->m_pElf, dynsymScnIndex);
    Elf_Data* pDynsymData = elf_getdata(pScn, NULL);

    GElf_Shdr shdr;

    if (NULL != pRelaData && NULL != pDynsymData && NULL != gelf_getshdr(pScn, &shdr))
    {
        // skip first entry of .plt, it is for link resolver
        for (int i = 1; i < pltEcount; i++)
        {
            GElf_Rela rela;
            GElf_Sym sym;

            // fetch relocation data for corresponding .plt entry
            if (NULL == gelf_getrela(pRelaData, (i - 1), &rela))
            {
                continue;
            }

            // fetch index for corresponding .dynsym entry
            gtUInt32 index = GELF_R_SYM(rela.r_info);

            // fetch .dynsym entry for the index
            if (NULL != gelf_getsym(pDynsymData, index, &sym))
            {
                // fetch name of the symbol of corresponding .plt entry
                char* pName = elf_strptr(m_pElf->m_pElf, shdr.sh_link, sym.st_name);

                if (NULL == pName || '\0' == pName[0])
                {
                    continue;
                }

                // demangle the symbol name and prepend [PLT]
                wchar_t* dmName = DemangleNameIA(pName);
                size_t len = 6 + wcslen(dmName) + 1;
                wchar_t* funcName = new wchar_t[len];
                wcscpy(funcName, L"[PLT] ");
                wcscat(funcName, dmName);

                // insert the dummy plt entry to the functions vector
                FunctionSymbolInfo funcInfo;
                FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U);
                funcInfo.m_addrRanges = NULL;
                funcInfo.m_rva = pltStart + (pltEsize * i);
                funcInfo.m_size = pltEsize + 1;
                funcInfo.m_pName = funcName;
                funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);

                m_pFuncsInfoVec->push_back(funcInfo);
                result = true;
            }
        }
    }

    return result;
}

