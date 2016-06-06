//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StabsSymbolEngine.cpp
/// \brief This file contains the class for querying STABS symbols.
///
//==================================================================================

#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <ExecutableFile.h>
#include "StabsSymbolEngine.h"

// Masks for n_type field
#ifndef N_STAB
    #define N_STAB      0xE0
#endif
#ifndef N_TYPE
    #define N_TYPE      0x1E
#endif
#ifndef N_EXT
    #define N_EXT       0x01
#endif

// Values for (n_type & N_TYPE)
#ifndef N_UNDF
    #define N_UNDF      0x00
#endif
#ifndef N_ABS
    #define N_ABS       0x02
#endif

#define N_GSYM      0x20
#define N_FUN       0x24
#define N_STSYM     0x26
#define N_LCSYM     0x28
#define N_MAIN      0x2A
#define N_ROSYM     0x2C
#define N_BNSYM     0x2E
#define N_OPT       0x3C
#define N_RSYM      0x40
#define N_SLINE     0x44
#define N_ENSYM     0X4E
#define N_SO        0x64
#define N_OSO       0x66
#define N_LSYM      0x80
#define N_BINCL     0x82
#define N_SOL       0x84
#define N_PSYM      0XA0
#define N_EINCL     0XA2
#define N_LBRAC     0XC0
#define N_EXCL      0XC2
#define N_RBRAC     0XE0

static wchar_t* ExtractDemangledName(const char* pSrc);

StabsSymbolEngine::StabsSymbolEngine() : m_pEntries(NULL),
    m_numEntries(0),
    m_pStrings(NULL),
    m_sizeStrings(0),
    m_pExe(NULL),
    m_pAuxSymEngine(NULL)
{
}

StabsSymbolEngine::~StabsSymbolEngine()
{
    if (NULL != m_pAuxSymEngine)
    {
        delete m_pAuxSymEngine;
    }
}

bool StabsSymbolEngine::Initialize(const ExecutableFile& exe, unsigned stabIndex, unsigned stabstrIndex,
                                   ModularSymbolEngine* pAuxSymEngine)
{
    bool ret = false;

    m_pExe = &exe;

    gtRVAddr startRva, endRva;
    const gtUByte* pBytes;

    if ((pBytes = exe.GetSectionBytes(stabIndex)) != NULL && exe.GetSectionRvaLimits(stabIndex, startRva, endRva))
    {
        m_pEntries = reinterpret_cast<const InternalNList*>(pBytes);
        m_numEntries = (endRva - startRva) / sizeof(InternalNList);

        if ((pBytes = exe.GetSectionBytes(stabstrIndex)) != NULL && exe.GetSectionRvaLimits(stabstrIndex, startRva, endRva))
        {
            m_pStrings = reinterpret_cast<const char*>(pBytes);
            m_sizeStrings = endRva - startRva;
            m_imageBase = static_cast<gtUInt32>(exe.GetImageBase());

            InitializeFunctionsInfo();

            gtASCIIString stringBuffer;
            FunctionSymbolInfo* pPrevFunc = NULL;

            char const* const pStringsEnd = m_pStrings + m_sizeStrings;

            for (unsigned i = 0; i < m_numEntries; ++i)
            {
                const InternalNList& entry = m_pEntries[i];
                const char* pString = m_pStrings + entry.n_strx;

                if (pStringsEnd < pString)
                {
                    continue;
                }

                int lenString = static_cast<int>(strlen(pString));

                if (pStringsEnd < (pString + lenString))
                {
                    // Invalid STABS string!
                    continue;
                }

                if (0 != lenString && '\\' == pString[lenString - 1])
                {
                    // Indicates continuation. Append this to the buffer, and go onto the next record.
                    // Repeat the process until we find a STAB without the '\' character, as this indicates we have the whole thing.
                    stringBuffer.append(pString, lenString - 1);
                    continue;
                }
                else if (!stringBuffer.isEmpty())
                {
                    stringBuffer.append(pString, lenString);
                    pString = stringBuffer.asCharArray();
                }

                gtUByte type = entry.n_type;

                if (0 == (type & N_STAB))
                {
                    type &= N_TYPE;
                }

                wchar_t* pSymbolName;

                switch (type)
                {
                    case N_FUN:
                        // Copy the string to a temp buffer so we can kill everything after the ':'.
                        // We do it this way because otherwise we end up dirtying all of the pages related to the stabs,
                        // and that sucks up swap space like crazy.
                        pSymbolName = ExtractDemangledName(pString);

                        if (NULL != pSymbolName && L'\0' != pSymbolName[0])
                        {
                            FunctionSymbolInfo funcInfo;
                            FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = 0U;)
                            funcInfo.m_rva = entry.n_value - m_imageBase;
                            funcInfo.m_size = 0U;
                            funcInfo.m_pName = pSymbolName;
                            funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);

                            if (NULL != pPrevFunc)
                            {
                                // First, clean up the previous function we were working on.
                                // Assume size of the func is the delta between current offset and offset of last function.
                                pPrevFunc->m_size = static_cast<gtUInt32>(funcInfo.m_rva - pPrevFunc->m_rva);
                            }

                            m_pFuncsInfoVec->push_back(funcInfo);
                            pPrevFunc = &m_pFuncsInfoVec->back();
                        }
                        else if (NULL != pPrevFunc)
                        {
                            // Some versions of GCC use a N_FUN "" to mark the end of a function
                            // and n_value contains the size of the func.
                            pPrevFunc->m_size = entry.n_value;
                            pPrevFunc = NULL;
                        }

                        break;

                    case N_SO:

                        // This indicates a new source file.
                        if ('\0' == pString[0]) // end of N_SO file
                        {
                            if (NULL != pPrevFunc)
                            {
                                pPrevFunc->m_size = 0;
                                pPrevFunc = NULL;
                            }
                        }

                        break;
                }

                stringBuffer.makeEmpty();
            }

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

            ret = true;
        }
        else
        {
            m_pEntries = NULL;
            m_numEntries = 0U;
        }
    }

    return ret;
}

const FunctionSymbolInfo* StabsSymbolEngine::LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    const FunctionSymbolInfo* pFunc = StabsSymbolEngine::LookupBoundingFunction(rva, pNextRva);

    if (NULL != pFunc)
    {
        if (0 == pFunc->m_size)
        {
            if (m_pExe->LookupSectionIndex(pFunc->m_rva) != m_pExe->LookupSectionIndex(rva))
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

bool StabsSymbolEngine::EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    bool ret = true;

    if (NULL != m_pAuxSymEngine)
    {
        ret = m_pAuxSymEngine->EnumerateSourceLineInstances(pSourceFilePath, srcLineInstanceMap);
    }

    if (ret && NULL != m_pStrings)
    {
        gtASCIIString stringBuffer;
        gtRVAddr funcRva = GT_INVALID_RVADDR;

        char const* const pStringsEnd = m_pStrings + m_sizeStrings;

        for (unsigned i = 0; i < m_numEntries; ++i)
        {
            const InternalNList& entry = m_pEntries[i];
            const char* pString = m_pStrings + entry.n_strx;

            if (pStringsEnd < pString)
            {
                continue;
            }

            int lenString = static_cast<int>(strlen(pString));

            if (pStringsEnd < (pString + lenString))
            {
                // Invalid STABS string!
                continue;
            }

            if (0 != lenString && '\\' == pString[lenString - 1])
            {
                // Indicates continuation. Append this to the buffer, and go onto the next record.
                // Repeat the process until we find a STAB without the '\' character, as this indicates we have the whole thing.
                stringBuffer.append(pString, lenString - 1);
                continue;
            }
            else if (!stringBuffer.isEmpty())
            {
                stringBuffer.append(pString, lenString);
                pString = stringBuffer.asCharArray();
            }

            gtUByte type = entry.n_type;

            if (0 == (type & N_STAB))
            {
                type &= N_TYPE;
            }

            switch (type)
            {
                case N_FUN:
                    if (L'\0' != pString[0])
                    {
                        funcRva = entry.n_value - m_imageBase;
                    }
                    else
                    {
                        funcRva = GT_INVALID_RVADDR;
                    }

                    break;

                case N_SO:

                    // This indicates a new source file.
                    if ('\0' == pString[0]) // end of N_SO file
                    {
                        funcRva = GT_INVALID_RVADDR;
                    }

                    break;

                case N_SLINE:

                    // This is a line number.
                    // These are always relative to the start of the function (N_FUN).
                    if (GT_INVALID_RVADDR != funcRva)
                    {
                        srcLineInstanceMap[funcRva + entry.n_value] = entry.n_desc;
                    }

                    break;
            }

            stringBuffer.makeEmpty();
        }
    }

    return ret;
}

bool StabsSymbolEngine::FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool handleInline)
{
    GT_UNREFERENCED_PARAMETER(handleInline);
    bool ret = false;

    if (NULL != m_pAuxSymEngine)
    {
        ret = m_pAuxSymEngine->FindSourceLine(rva, sourceLine);
    }

    if (!ret && NULL != m_pStrings)
    {
        sourceLine.m_filePath[0] = L'\0';

        gtASCIIString stringBuffer;
        gtRVAddr funcRva = GT_INVALID_RVADDR;

        sourceLine.m_offset = GT_INT32_MAX;

        char* pCandidatePath = NULL;
        char linePath[2][OS_MAX_PATH];
        linePath[0][0] = '\0';
        linePath[1][0] = '\0';

        unsigned currPathIndex = 0U;

        char const* const pStringsEnd = m_pStrings + m_sizeStrings;

        for (unsigned i = 0; i < m_numEntries; ++i)
        {
            const InternalNList& entry = m_pEntries[i];
            const char* pString = m_pStrings + entry.n_strx;

            if (pStringsEnd < pString)
            {
                continue;
            }

            int lenString = static_cast<int>(strlen(pString));

            if (pStringsEnd < (pString + lenString))
            {
                // Invalid STABS string!
                continue;
            }

            if (0 != lenString && '\\' == pString[lenString - 1])
            {
                // Indicates continuation. Append this to the buffer, and go onto the next record.
                // Repeat the process until we find a STAB without the '\' character, as this indicates we have the whole thing.
                stringBuffer.append(pString, lenString - 1);
                continue;
            }
            else if (!stringBuffer.isEmpty())
            {
                stringBuffer.append(pString, lenString);
                pString = stringBuffer.asCharArray();
                lenString = stringBuffer.length();
            }

            gtUByte type = entry.n_type;

            if (0 == (type & N_STAB))
            {
                type &= N_TYPE;
            }

            switch (type)
            {
                case N_FUN:
                    if (L'\0' != pString[0])
                    {
                        funcRva = entry.n_value - m_imageBase;
                    }
                    else
                    {
                        funcRva = GT_INVALID_RVADDR;
                    }

                    break;

                case N_SO:

                    // This indicates a new source file.
                    if ('\0' == pString[0]) // end of N_SO file
                    {
                        funcRva = GT_INVALID_RVADDR;

                        if (pCandidatePath == linePath[currPathIndex])
                        {
                            currPathIndex = (currPathIndex + 1U) & 1U;
                        }

                        linePath[currPathIndex][0] = '\0';
                    }
                    else
                    {
                        if (pString[lenString - 1] != '/')
                        {
                            size_t pathLen = strlen(linePath[currPathIndex]);

                            if ((pathLen + lenString) < OS_MAX_PATH)
                            {
                                memcpy(&linePath[currPathIndex][pathLen], pString, lenString + 1);
                            }
                        }
                        else
                        {
                            if (lenString < OS_MAX_PATH)
                            {
                                memcpy(linePath[currPathIndex], pString, lenString + 1);
                            }
                        }
                    }

                    break;

                case N_SLINE:

                    // This is a line number.
                    // These are always relative to the start of the function (N_FUN).
                    if (GT_INVALID_RVADDR != funcRva)
                    {
                        gtRVAddr candidateOffset = rva - (funcRva + entry.n_value);

                        if (candidateOffset < sourceLine.m_offset)
                        {
                            sourceLine.m_offset = candidateOffset;
                            sourceLine.m_line = entry.n_desc;
                            pCandidatePath = linePath[currPathIndex];
                        }
                    }

                    break;
            }

            stringBuffer.makeEmpty();
        }

        if (NULL != pCandidatePath)
        {
            sourceLine.m_rva = rva - sourceLine.m_offset;
            sourceLine.m_filePath[0] = L'\0';
            mbstowcs(sourceLine.m_filePath, pCandidatePath, OS_MAX_PATH);
        }

        ret = (L'\0' != sourceLine.m_filePath[0]);
    }

    return ret;
}


static wchar_t* ExtractDemangledName(const char* pSrc)
{
    int sz = 4096;
    char dst[4096];
    char* ptr = dst;

    // A strcpy routine that stops when we hit the ':' character.
    // Faster than copying the whole thing, and then nuking the ':'.
    // Takes also care of (valid) a::b constructs.
    while (*pSrc != '\0')
    {
        if (pSrc[0] != ':' && sz-- > 0)
        {
            *ptr++ = *pSrc++;
        }
        else if (pSrc[1] == ':' && (sz -= 2) > 0)
        {
            *ptr++ = *pSrc++;
            *ptr++ = *pSrc++;
        }
        else
        {
            break;
        }
    }

    *ptr-- = '\0';

    // GCC emits, in some cases, a .<digit>+ suffix.
    // This is used for static variable inside functions,
    // so that we can have several such variables with same name in the same compilation unit.
    // We simply ignore that suffix when present.
    if (ptr >= dst && isdigit(*ptr))
    {
        while (ptr > dst && isdigit(*ptr))
        {
            ptr--;
        }

        if (*ptr == '.')
        {
            *ptr = '\0';
        }
    }

    GT_ASSERT(sz > 0);
    return SymbolEngine::DemangleNameIA(dst);
}
