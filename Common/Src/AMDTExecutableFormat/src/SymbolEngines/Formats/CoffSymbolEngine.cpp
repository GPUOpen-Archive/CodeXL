//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CoffSymbolEngine.cpp
/// \brief This file contains the class for querying COFF symbols.
///
//==================================================================================

#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include "CoffSymbolEngine.h"
#include "PeFile.h"

static inline bool IsFunctionSymbol(const IMAGE_SYMBOL& symbol)
{
    return (IMAGE_SYM_CLASS_EXTERNAL == symbol.StorageClass || IMAGE_SYM_CLASS_STATIC == symbol.StorageClass) && ISFCN(symbol.Type);
}

static DWORD FindLinenumberBaseAddress(const IMAGE_LINENUMBER* pLinenumbers, unsigned numLines, gtVAddr imageBase);
static int FindFirstFileSymbol(const IMAGE_SYMBOL* pSymbolTable, unsigned numSymbols);
static const IMAGE_SYMBOL* FindBeginFunctionSymbol(const IMAGE_SYMBOL* pSymbolTable, unsigned numSymbols, unsigned funcIndex);
static wchar_t* ExtractDemangledName(const IMAGE_SYMBOL* pSymbol, const char* pStringTable, wchar_t* (*pfnDemangleName)(const char*));
static bool ExtractFileName(const char* pStringTable, const IMAGE_SYMBOL* pSymFile, wchar_t* pDst, unsigned maxLen);
static const char* ExtractFileName(const char* pStringTable, const IMAGE_SYMBOL* pSymFile);
static void SwapCellIndexAndValue(const WORD* pSrc, unsigned lenSrc, unsigned* pDst, unsigned lenDst);

CoffSymbolEngine::CoffSymbolEngine() : m_pPortableExe(NULL),
    m_pSymbolTable(NULL),
    m_pStringTable(NULL),
    m_firstFileIndex(-1),
    m_isComplete(false)
{
}

CoffSymbolEngine::~CoffSymbolEngine()
{
}

bool CoffSymbolEngine::Initialize(const PeFile& pe, wchar_t* (*pfnDemangleName)(const char*))
{
    bool ret = true;

    m_isComplete = (static_cast<wchar_t* (*)(const char*)>(SymbolEngine::DemangleNameVS) != pfnDemangleName);

    InitializeFunctionsInfo();

    m_pPortableExe = &pe;

    if (NULL != pe.m_pNtHeader && 0 != pe.m_pNtHeader->FileHeader.NumberOfSymbols)
    {
        m_pSymbolTable = reinterpret_cast<const IMAGE_SYMBOL*>(pe.m_pFileBase + pe.m_pNtHeader->FileHeader.PointerToSymbolTable);

        // StringTable starts right after the array of IMAGE_SYMBOL's
        m_pStringTable = reinterpret_cast<const char*>(m_pSymbolTable + pe.m_pNtHeader->FileHeader.NumberOfSymbols);
    }
    else
    {
        m_pSymbolTable = NULL;
        m_pStringTable = NULL;
    }

    EnumerateFunctionSymbols(pe, pfnDemangleName);

    const IMAGE_EXPORT_DIRECTORY* pExports =
        static_cast<const IMAGE_EXPORT_DIRECTORY*>(pe.DirectoryEntryToData(IMAGE_DIRECTORY_ENTRY_EXPORT));

    if (NULL != pExports)
    {
        EnumerateExports(pe, *pExports, pfnDemangleName);
    }

    return ret;
}

unsigned CoffSymbolEngine::GetSymbolsCount() const
{
    return static_cast<unsigned>(reinterpret_cast<const IMAGE_SYMBOL*>(m_pStringTable) - m_pSymbolTable);
}

void CoffSymbolEngine::EnumerateFunctionSymbols(const PeFile& pe, wchar_t* (*pfnDemangleName)(const char*))
{
    for (const IMAGE_SYMBOL* pSymbol = m_pSymbolTable, *pSymbolEnd = reinterpret_cast<const IMAGE_SYMBOL*>(m_pStringTable);
         pSymbol < pSymbolEnd; pSymbol += pSymbol->NumberOfAuxSymbols + 1)
    {
        if (IsFunctionSymbol(*pSymbol))
        {
            gtRVAddr startRva, endRva;

            if (pe.GetSectionRvaLimits(static_cast<unsigned>(pSymbol->SectionNumber - 1), startRva, endRva))
            {
                FunctionSymbolInfo funcInfo;
                FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U;)
                funcInfo.m_rva = startRva + pSymbol->Value;
                funcInfo.m_pName = ExtractDemangledName(pSymbol, m_pStringTable, pfnDemangleName);
                funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);

                if (0 < pSymbol->NumberOfAuxSymbols)
                {
                    const IMAGE_AUX_SYMBOL* pAuxSymbol = reinterpret_cast<const IMAGE_AUX_SYMBOL*>(pSymbol + 1);
                    funcInfo.m_size = pAuxSymbol->Sym.Misc.TotalSize;
                }
                else
                {
                    funcInfo.m_size = 0U;
                }

                m_pFuncsInfoVec->push_back(funcInfo);
            }
        }
    }

    gtSort(m_pFuncsInfoVec->begin(), m_pFuncsInfoVec->end());
}

void CoffSymbolEngine::EnumerateExports(const PeFile& pe, const IMAGE_EXPORT_DIRECTORY& exports,
                                        wchar_t* (*pfnDemangleName)(const char*))
{
    const DWORD* pFuncRvas = (0 != exports.NumberOfFunctions) ?
                             static_cast<const DWORD*>(pe.TranslateRvaToPtr(exports.AddressOfFunctions)) : NULL;

    if (NULL != pFuncRvas)
    {
        unsigned* pNameIndices = NULL;
        const DWORD* pNameRvas = NULL;

        if (0 != exports.NumberOfNames && exports.NumberOfNames <= exports.NumberOfFunctions)
        {
            pNameRvas = static_cast<const DWORD*>(pe.TranslateRvaToPtr(exports.AddressOfNames));
            const WORD* pOrdinalRvas = static_cast<const WORD*>(pe.TranslateRvaToPtr(exports.AddressOfNameOrdinals));

            if (NULL != pNameRvas && NULL != pOrdinalRvas)
            {
                pNameIndices = new unsigned[exports.NumberOfFunctions];

                if (NULL != pNameIndices)
                {
                    memset(pNameIndices, 0xFFFFFFFF, sizeof(unsigned) * exports.NumberOfFunctions);
                    SwapCellIndexAndValue(pOrdinalRvas, exports.NumberOfNames, pNameIndices, exports.NumberOfFunctions);
                }
            }
        }

        typedef gtMap<gtRVAddr, wchar_t*> ExportsInfoMap;
        ExportsInfoMap exportsInfo;

        gtRVAddr rva = GT_INVALID_RVADDR;

        for (gtVector<FunctionSymbolInfo>::iterator it = m_pFuncsInfoVec->begin(), itEnd = m_pFuncsInfoVec->end(); it != itEnd;)
        {
            wchar_t* pName = it->m_pName;

            if (rva != it->m_rva)
            {
                rva = it->m_rva;
                exportsInfo.insert(ExportsInfoMap::value_type(rva, pName));

                ++it;
            }
            else
            {
                if (NULL != pName)
                {
                    delete [] pName;
                }

                it = m_pFuncsInfoVec->erase(it);
                itEnd = m_pFuncsInfoVec->end();
            }
        }

        unsigned* const pNameIndicesBase = pNameIndices;
        const bool hasNames = (NULL != pNameIndices);

        for (DWORD i = exports.NumberOfFunctions; 0 != i; --i, ++pFuncRvas, pNameIndices += static_cast<unsigned>(hasNames))
        {
            rva = *pFuncRvas;

            // We are interested only in exported functions (and not other types, like exported global variables).
            // Functions must exist in the code section.
            if (pe.IsCodeSection(pe.LookupSectionIndex(rva)))
            {
                ExportsInfoMap::iterator itInfo = exportsInfo.find(rva);

                if (exportsInfo.end() == itInfo)
                {
                    itInfo = exportsInfo.insert(ExportsInfoMap::value_type(rva, static_cast<wchar_t*>(NULL))).first;
                }

                if (hasNames && NULL == itInfo->second && *pNameIndices < exports.NumberOfNames)
                {
                    // Get a pointer to the RVA name.
                    // We can perform this cast because we're using a RVA from the NameRVAs collection.
                    const char* pName = static_cast<const char*>(pe.TranslateRvaToPtr(pNameRvas[*pNameIndices]));

                    if (NULL != pName)
                    {
                        // Try to demangle AMD's OpenCL mangling scheme: for a kernel named foo -> _OpenCL_foo_stub and _OpenCL_foo_kernel
                        if ('_' == *pName)
                        {
                            // Look for "_OpenCL_" in the name.
                            // Use direct comparison instead of memcmp because this is done for each symbol and there are thousands of them!
                            const DWORD UNALIGNED* pDwName = reinterpret_cast<const DWORD UNALIGNED*>(pName + 1);

                            if (FCC('_Ope') == pDwName[0] && FCC('nCL_') == pDwName[1])
                            {
                                // Look for "_stub" or "_kernel" substrings in the name
                                const char* pSuffix = strrchr(pName + 9, '_');
                                pDwName = reinterpret_cast<const DWORD UNALIGNED*>(pSuffix + 1);

                                if ((FCC('stub') == pDwName[0] && '\0' == pSuffix[5]) ||
                                    (FCC('kern') == pDwName[0] && 'e' == pSuffix[5] && 'l' == pSuffix[6] && '\0' == pSuffix[7]))
                                {
                                    pName += 9;
                                    int lenName = static_cast<int>(pSuffix - pName);
                                    itInfo->second = AnsiToUnicode(pName, lenName);
                                }
                            }
                        }

                        if (NULL == itInfo->second)
                        {
                            // Select appropriate name demangler
                            // Visual Studio mangled names start with '?'
                            // GCC mangled names start with '_'
                            if ('?' == pName[0])
                            {
                                pfnDemangleName = SymbolEngine::DemangleNameVS;
                            }
                            else if ('_' == pName[0])
                            {
                                // DemangleNameIA can demangle names begin with "_Z" (1 underscore) or "___Z" (3 underscores)
                                // Cygwin PE32 executables, compiled using GCC, contain mangled names begin with "__Z" (2 underscores)
                                // To simplify the name demangling, skip first '_'
                                if ('_' == pName[1] && 'Z' == pName[2])
                                {
                                    ++pName;
                                }

                                pfnDemangleName = SymbolEngine::DemangleNameIA;
                            }

                            itInfo->second = pfnDemangleName(pName);
                        }
                    }
                }
            }
        }

        if (NULL != pNameIndicesBase)
        {
            delete [] pNameIndicesBase;
        }

        if (!exportsInfo.empty())
        {
            // Loop over the names.
            // For symbols that exist in both the symbols table and the Exports table we override the original names
            // (from the symbols table) with the names from the exports table.
            for (gtVector<FunctionSymbolInfo>::iterator it = m_pFuncsInfoVec->begin(), itEnd = m_pFuncsInfoVec->end(); it != itEnd; ++it)
            {
                ExportsInfoMap::iterator itExInfo = exportsInfo.find(it->m_rva);

                if (exportsInfo.end() != itExInfo)
                {
                    if (NULL == it->m_pName && NULL != itExInfo->second)
                    {
                        it->m_pName = itExInfo->second;
                    }

                    exportsInfo.erase(itExInfo);
                }
            }

            size_t numFuncs = m_pFuncsInfoVec->size();
            m_pFuncsInfoVec->resize(numFuncs + exportsInfo.size());

            gtVector<FunctionSymbolInfo>::iterator itFuncInfo = m_pFuncsInfoVec->begin() + numFuncs;

            // Push the symbols from the Exports table into the vector of functions info.
            for (ExportsInfoMap::iterator it = exportsInfo.begin(), itEnd = exportsInfo.end(); it != itEnd; ++it, ++itFuncInfo)
            {
                itFuncInfo->m_rva = it->first;
                itFuncInfo->m_pName = it->second;
                itFuncInfo->m_funcId = AtomicAdd(m_nextFuncId, 1);
            }

            // gtMap is already sorted. We only need to sort if we had previous values in the vector.
            if (0 != numFuncs)
            {
                gtSort(m_pFuncsInfoVec->begin(), m_pFuncsInfoVec->end());
            }
        }
    }
}

const FunctionSymbolInfo* CoffSymbolEngine::LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    const FunctionSymbolInfo* pFunc = CoffSymbolEngine::LookupBoundingFunction(rva, pNextRva);

    if (NULL != pFunc)
    {
        if (0 == pFunc->m_size)
        {
            if (m_pPortableExe->LookupSectionIndex(pFunc->m_rva) != m_pPortableExe->LookupSectionIndex(rva))
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

bool CoffSymbolEngine::EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    bool ret = NULL != pSourceFilePath;

    const unsigned numSymbols = GetSymbolsCount();

    if (ret &&
        (0 <= m_firstFileIndex ||
         static_cast<unsigned>(m_firstFileIndex = FindFirstFileSymbol(m_pSymbolTable, numSymbols)) < numSymbols))
    {
        const IMAGE_SYMBOL* pSymFile;
        char sourceFilePath[OS_MAX_PATH];

        if (0 != WideCharToMultiByte(CP_ACP, 0, pSourceFilePath, -1, sourceFilePath, OS_MAX_PATH, NULL, NULL) &&
            NULL != (pSymFile = FindFileSymbol(sourceFilePath)))
        {
            const unsigned compilandBegin = static_cast<unsigned>(pSymFile - m_pSymbolTable) + 2U;
            const unsigned compilandEnd = (numSymbols > pSymFile->Value && pSymFile->Value >= compilandBegin) ? pSymFile->Value :
                                          numSymbols;

            DWORD baseAddr = (DWORD)(-1);

            const IMAGE_SECTION_HEADER* pSection = m_pPortableExe->m_pSections;

            for (unsigned j = 0, numSections = m_pPortableExe->GetSectionsCount(); j < numSections; ++j, ++pSection)
            {
                if (0 < pSection->NumberOfLinenumbers && 0 != pSection->PointerToLinenumbers)
                {
                    IMAGE_LINENUMBER* pLinenum =
                        reinterpret_cast<IMAGE_LINENUMBER*>(m_pPortableExe->m_pFileBase + pSection->PointerToLinenumbers);

                    if ((DWORD)(-1) == baseAddr)
                    {
                        baseAddr = FindLinenumberBaseAddress(pLinenum, pSection->NumberOfLinenumbers, m_pPortableExe->GetImageBase());
                    }

                    unsigned baseLineNumber = 0U;
                    unsigned i = 0U, numLinenums = pSection->NumberOfLinenumbers;

                    while (i < numLinenums)
                    {
                        if (0 == pLinenum->Linenumber)
                        {
                            unsigned funcIndex = pLinenum->Type.SymbolTableIndex;
                            const IMAGE_SYMBOL& symFunc = m_pSymbolTable[funcIndex];

                            if (!IsFunctionSymbol(symFunc))
                            {
                                break;
                            }

                            if (!(compilandBegin <= funcIndex && funcIndex < compilandEnd))
                            {
                                // Skip this function
                                do
                                {
                                    ++i;
                                    ++pLinenum;
                                }
                                while (i < numLinenums && 0 != pLinenum->Linenumber);

                                continue;
                            }

                            const IMAGE_SYMBOL* pSymBeginFunc = FindBeginFunctionSymbol(m_pSymbolTable, numSymbols, funcIndex);

                            if (NULL == pSymBeginFunc || 0 == pSymBeginFunc->NumberOfAuxSymbols)
                            {
                                break;
                            }

                            gtRVAddr startRva, endRva;

                            if (!m_pPortableExe->GetSectionRvaLimits(symFunc.SectionNumber - 1, startRva, endRva))
                            {
                                break;
                            }

                            startRva += symFunc.Value;
                            baseLineNumber = reinterpret_cast<const IMAGE_AUX_SYMBOL*>(pSymBeginFunc + 1)->Sym.Misc.LnSz.Linenumber;

                            srcLineInstanceMap[startRva] = baseLineNumber;
                        }
                        else
                        {
                            gtRVAddr rva = pLinenum->Type.VirtualAddress - baseAddr;
                            unsigned lineNumber = baseLineNumber + pLinenum->Linenumber - 1;
                            srcLineInstanceMap[rva] = lineNumber;
                        }

                        ++i;
                        ++pLinenum;
                    }
                }
            }
        }
    }

    return ret;
}

bool CoffSymbolEngine::FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    bool ret = false;

    const unsigned numSymbols = GetSymbolsCount();

    if (0 <= m_firstFileIndex ||
        static_cast<unsigned>(m_firstFileIndex = FindFirstFileSymbol(m_pSymbolTable, numSymbols)) < numSymbols)
    {
        const IMAGE_SECTION_HEADER* pSection = m_pPortableExe->LookupSectionByRva(rva);

        if (NULL != pSection && 0 < pSection->NumberOfLinenumbers && 0 != pSection->PointerToLinenumbers)
        {
            IMAGE_LINENUMBER* pLinenum =
                reinterpret_cast<IMAGE_LINENUMBER*>(m_pPortableExe->m_pFileBase + pSection->PointerToLinenumbers);

            DWORD baseAddr = FindLinenumberBaseAddress(pLinenum, pSection->NumberOfLinenumbers, m_pPortableExe->GetImageBase());
            DWORD searchAddr = rva + baseAddr;

            DWORD funcIndex = (DWORD)(-1);

            for (unsigned i = 0U, numLinenums = pSection->NumberOfLinenumbers; i < numLinenums; ++i, ++pLinenum)
            {
                if (0 == pLinenum->Linenumber)
                {
                    funcIndex = pLinenum->Type.SymbolTableIndex;
                }
                else if (searchAddr <= pLinenum->Type.VirtualAddress)
                {
                    if (!(funcIndex < numSymbols && IsFunctionSymbol(m_pSymbolTable[funcIndex])))
                    {
                        break;
                    }

                    const IMAGE_SYMBOL* pSymBeginFunc = FindBeginFunctionSymbol(m_pSymbolTable, numSymbols, funcIndex);

                    if (NULL == pSymBeginFunc || 0 == pSymBeginFunc->NumberOfAuxSymbols)
                    {
                        break;
                    }

                    const IMAGE_SYMBOL* pSymFile = FindParentFileSymbol(funcIndex);

                    if (NULL == pSymFile || 0 == pSymFile->NumberOfAuxSymbols)
                    {
                        break;
                    }

                    sourceLine.m_line = reinterpret_cast<const IMAGE_AUX_SYMBOL*>(pSymBeginFunc + 1)->Sym.Misc.LnSz.Linenumber;

                    if (searchAddr < pLinenum->Type.VirtualAddress)
                    {
                        --pLinenum;

                        if (0 == pLinenum->Linenumber)
                        {
                            const IMAGE_SYMBOL& symFunc = m_pSymbolTable[pLinenum->Type.SymbolTableIndex];

                            if (!IsFunctionSymbol(symFunc))
                            {
                                break;
                            }

                            if (rva < symFunc.Value)
                            {
                                break;
                            }

                            gtRVAddr startRva, endRva;

                            if (!m_pPortableExe->GetSectionRvaLimits(symFunc.SectionNumber - 1, startRva, endRva))
                            {
                                break;
                            }

                            sourceLine.m_rva = startRva + symFunc.Value;
                        }
                    }

                    if (0 != pLinenum->Linenumber)
                    {
                        sourceLine.m_rva = pLinenum->Type.VirtualAddress - baseAddr;
                        sourceLine.m_line += pLinenum->Linenumber - 1;
                    }

                    sourceLine.m_offset = rva - sourceLine.m_rva;

                    ret = ExtractFileName(m_pStringTable, pSymFile, sourceLine.m_filePath, OS_MAX_PATH);
                    break;
                }
            }
        }
    }

    return ret;
}

const IMAGE_SYMBOL* CoffSymbolEngine::FindFileSymbol(const char* pFileName) const
{
    const IMAGE_SYMBOL* pSymFile = NULL;
    const unsigned numSymbols = GetSymbolsCount();
    unsigned prevFileIndex = numSymbols;
    unsigned fileIndex = static_cast<unsigned>(m_firstFileIndex);

    do
    {
        const IMAGE_SYMBOL& symbol = m_pSymbolTable[fileIndex];

        if (IMAGE_SYM_CLASS_FILE != symbol.StorageClass)
        {
            break;
        }

        if (0 == strcmp(ExtractFileName(m_pStringTable, &symbol), pFileName))
        {
            pSymFile = &symbol;
            break;
        }

        prevFileIndex = fileIndex;
        fileIndex = symbol.Value;
    }
    while (fileIndex < numSymbols && fileIndex > prevFileIndex);

    return pSymFile;
}

const IMAGE_SYMBOL* CoffSymbolEngine::FindParentFileSymbol(unsigned index) const
{
    const unsigned numSymbols = GetSymbolsCount();
    unsigned prevFileIndex = numSymbols;
    unsigned fileIndex = static_cast<unsigned>(m_firstFileIndex);

    do
    {
        if (index < fileIndex)
        {
            break;
        }

        const IMAGE_SYMBOL& symbol = m_pSymbolTable[fileIndex];

        if (IMAGE_SYM_CLASS_FILE != symbol.StorageClass)
        {
            break;
        }

        prevFileIndex = fileIndex;
        fileIndex = symbol.Value;
    }
    while (fileIndex < numSymbols && fileIndex > prevFileIndex);

    return (prevFileIndex < numSymbols) ? &m_pSymbolTable[prevFileIndex] : NULL;
}

bool CoffSymbolEngine::HasSourceLineInfo() const
{
    bool ret = false;
    const IMAGE_SECTION_HEADER* pSection = m_pPortableExe->m_pSections;

    for (unsigned i = 0, numSections = m_pPortableExe->GetSectionsCount(); i < numSections; ++i, ++pSection)
    {
        if (0 < pSection->NumberOfLinenumbers)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

bool CoffSymbolEngine::IsComplete() const
{
    return m_isComplete;
}


static DWORD FindLinenumberBaseAddress(const IMAGE_LINENUMBER* pLinenumbers, unsigned numLines, gtVAddr imageBase)
{
    DWORD baseAddr = 0;

    for (unsigned i = 0U, numLinenums = numLines; i < numLinenums; ++i, ++pLinenumbers)
    {
        if (0 != pLinenumbers->Linenumber)
        {
            if (imageBase <= static_cast<gtVAddr>(pLinenumbers->Type.VirtualAddress))
            {
                baseAddr = static_cast<DWORD>(imageBase);
            }

            break;
        }
    }

    return baseAddr;
}

static int FindFirstFileSymbol(const IMAGE_SYMBOL* pSymbolTable, unsigned numSymbols)
{
    int index = 0U;

    while (index < static_cast<int>(numSymbols))
    {
        if (IMAGE_SYM_CLASS_FILE == pSymbolTable[index].StorageClass)
        {
            break;
        }

        index += 1 + pSymbolTable[index].NumberOfAuxSymbols;
    }

    return index;
}

static const IMAGE_SYMBOL* FindBeginFunctionSymbol(const IMAGE_SYMBOL* pSymbolTable, unsigned numSymbols, unsigned funcIndex)
{
    const IMAGE_SYMBOL* pSymBeginFunc = NULL;

    unsigned index = funcIndex + 1 + pSymbolTable[funcIndex].NumberOfAuxSymbols;

    if (index < numSymbols)
    {
        if (0 < pSymbolTable[funcIndex].NumberOfAuxSymbols)
        {
            const IMAGE_AUX_SYMBOL* pAuxSym = reinterpret_cast<const IMAGE_AUX_SYMBOL*>(&pSymbolTable[funcIndex + 1U]);
            unsigned nextFuncIndex = pAuxSym->Sym.FcnAry.Function.PointerToNextFunction;

            if (funcIndex < nextFuncIndex && nextFuncIndex < numSymbols)
            {
                numSymbols = nextFuncIndex;
            }
        }

        while (index < numSymbols)
        {
            const IMAGE_SYMBOL* pSym = &pSymbolTable[index];

            // Don't go beyond the compiland and the function's scope.
            if (IMAGE_SYM_CLASS_FILE == pSym->StorageClass || IMAGE_SYM_CLASS_EXTERNAL == pSym->StorageClass)
            {
                break;
            }

            if (IMAGE_SYM_CLASS_FUNCTION == pSym->StorageClass)
            {
                if (0 != pSym->N.Name.Short && 0 == memcmp(pSym->N.ShortName, ".bf", sizeof(".bf")))
                {
                    pSymBeginFunc = pSym;
                    break;
                }
            }

            index += 1 + pSym->NumberOfAuxSymbols;
        }
    }

    return pSymBeginFunc;
}

static wchar_t* ExtractDemangledName(const IMAGE_SYMBOL* pSymbol, const char* pStringTable, wchar_t* (*pfnDemangleName)(const char*))
{
    char shortNameBuffer[9];

    const char* pName;

    if (0 == pSymbol->N.Name.Short)
    {
        pName = pStringTable + pSymbol->N.Name.Long;
    }
    else
    {
        memcpy(shortNameBuffer, pSymbol->N.ShortName, 8);
        shortNameBuffer[8] = '\0';
        pName = shortNameBuffer;
    }

    // Select appropriate name demangler
    // Visual Studio mangled names start with '?'
    // GCC mangled names start with '_'
    if ('?' == pName[0])
    {
        pfnDemangleName = SymbolEngine::DemangleNameVS;
    }
    else if ('_' == pName[0])
    {
        // DemangleNameIA can demangle names begin with "_Z" (1 underscore) or "___Z" (3 underscores)
        // Cygwin PE32 executables, compiled using GCC, contain mangled names begin with "__Z" (2 underscores)
        // To simplify the name demangling, skip first '_'
        if ('_' == pName[1] && 'Z' == pName[2])
        {
            ++pName;
        }

        pfnDemangleName = SymbolEngine::DemangleNameIA;
    }

    return pfnDemangleName(pName);
}

static const char* ExtractFileName(const char* pStringTable, const IMAGE_SYMBOL* pSymFile)
{
    const char* pName;

    if (0 == pSymFile[1].N.Name.Short)
    {
        pName = pStringTable + pSymFile[1].N.Name.Long;
    }
    else
    {
        pName = reinterpret_cast<const char*>(reinterpret_cast<const IMAGE_AUX_SYMBOL*>(pSymFile + 1)->File.Name);
    }

    return pName;
}

static bool ExtractFileName(const char* pStringTable, const IMAGE_SYMBOL* pSymFile, wchar_t* pDst, unsigned maxLen)
{
    const char* pName = ExtractFileName(pStringTable, pSymFile);
    return 0 != MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pName, -1, pDst, maxLen);
}

// This function populates pDst array so that for each pSrc[x] with value y, pDst[y] will contain the value x.
static void SwapCellIndexAndValue(const WORD* pSrc, unsigned lenSrc, unsigned* pDst, unsigned lenDst)
{
    (void)lenDst; // Unused

    while (0U != lenSrc)
    {
        unsigned val = static_cast<unsigned>(pSrc[--lenSrc]);
        pDst[val] = lenSrc;
    }
}
