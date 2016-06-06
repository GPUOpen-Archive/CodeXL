//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SymbolManglingVS.cpp
///
//==================================================================================

#include <SymbolEngine.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>

#ifdef DBGHELP_TRANSLATE_TCHAR
    #undef DBGHELP_TRANSLATE_TCHAR
#endif

#pragma warning( push )
#pragma warning( disable : 4091)
#include <dbghelp.h>
#pragma warning( pop )

#define UNDNAME_IA_STYLE   (UNDNAME_NO_LEADING_UNDERSCORES  |  \
                            UNDNAME_NO_MS_KEYWORDS          |  \
                            UNDNAME_NO_FUNCTION_RETURNS     |  \
                            UNDNAME_NO_ALLOCATION_MODEL     |  \
                            UNDNAME_NO_ALLOCATION_LANGUAGE  |  \
                            UNDNAME_NO_THISTYPE             |  \
                            UNDNAME_NO_ACCESS_SPECIFIERS    |  \
                            UNDNAME_NO_THROW_SIGNATURES     |  \
                            UNDNAME_NO_MEMBER_TYPE          |  \
                            UNDNAME_NO_RETURN_UDT_MODEL     |  \
                            UNDNAME_32_BIT_DECODE)


static osCriticalSection g_demangleVsLock;


static unsigned StrLenT(const char* pStr)
{
    return static_cast<unsigned>(strlen(pStr));
}

static unsigned StrLenT(const wchar_t* pStr)
{
    return static_cast<unsigned>(wcslen(pStr));
}

static unsigned UnDecorateSymbolNameT(const char* pName, char* pOutString, DWORD maxStringLength, DWORD flags)
{
    unsigned res;

    g_demangleVsLock.enter();
    res = ::UnDecorateSymbolName(pName, pOutString, maxStringLength, flags);
    g_demangleVsLock.leave();

    return res;
}

static unsigned UnDecorateSymbolNameT(const wchar_t* pName, wchar_t* pOutString, DWORD maxStringLength, DWORD flags)
{
    unsigned res;

    g_demangleVsLock.enter();
    res = ::UnDecorateSymbolNameW(pName, pOutString, maxStringLength, flags);
    g_demangleVsLock.leave();

    return res;
}

template <typename CharT>
static unsigned TruncateStdcallManglingInternalVS(const CharT* pMangledName, unsigned lenMangledName)
{
    unsigned lenDemangledName = lenMangledName;

    if (0U != lenMangledName)
    {
        // Check for the embedded NULL at the end of the string
        if ((CharT)('\0') == pMangledName[lenDemangledName - 1U])
        {
            --lenDemangledName;
        }

        // The minimum is 3 characters: "x@y"
        if (3U <= lenDemangledName && (CharT)('0') <= pMangledName[lenDemangledName - 1U] && pMangledName[lenDemangledName - 1U] <= (CharT)('9'))
        {
            // From now on we treat lenDemangledName as a Position rather than a Length
            --lenDemangledName;

            // The minimum position of the '@' sign is at position 1 of the string
            while (--lenDemangledName != 1U)
            {
                // We only truncate digits
                if (!((CharT)('0') <= pMangledName[lenDemangledName] && pMangledName[lenDemangledName] <= (CharT)('9')))
                {
                    break;
                }
            }

            // We remove the suffix only if the digits were preceded by a '@'
            if ((CharT)('@') != pMangledName[lenDemangledName])
            {
                // Check if we still have a prefix to truncate
                lenDemangledName = lenMangledName;
            }
        }
    }

    return lenDemangledName;
}

template <typename CharT>
static unsigned DemangleExternCNameInternalVS(const CharT* pMangledName, unsigned lenMangledName, CharT* pDemangledName, unsigned maxDemangledNameLen)
{
    pDemangledName[0] = (CharT)('\0');

    const bool trucPrefix = ((CharT)('_') == pMangledName[0] || (CharT)('@') == pMangledName[0]);
    pMangledName += static_cast<unsigned>(trucPrefix);
    lenMangledName -= static_cast<unsigned>(trucPrefix);

    if (lenMangledName > maxDemangledNameLen)
    {
        lenMangledName = maxDemangledNameLen;
    }

    unsigned lenDemangledName = TruncateStdcallManglingInternalVS(pMangledName, lenMangledName);

    if (lenDemangledName == lenMangledName && !trucPrefix)
    {
        lenDemangledName = 0U;
    }

    if (0U != lenDemangledName)
    {
        memcpy(pDemangledName, pMangledName, lenDemangledName * sizeof(CharT));
        pDemangledName[lenDemangledName] = (CharT)('\0');
    }

    return lenDemangledName;
}

template <typename CharT>
static unsigned DemangleNameInternalVS(const CharT* pMangledName, CharT* pDemangledName, unsigned maxDemangledNameLen)
{
    unsigned lenDemangledName = 0U;
    pDemangledName[0] = (CharT)('\0');

    const CharT* pBaseMangledName = pMangledName;

    while ((CharT)('.') == pMangledName[0])
    {
        ++pMangledName;
    }

    if ((CharT)('?') == pMangledName[0])
    {
        lenDemangledName = UnDecorateSymbolNameT(pMangledName, pDemangledName, maxDemangledNameLen, UNDNAME_IA_STYLE);

        if (0U == lenDemangledName && pBaseMangledName != pMangledName)
        {
            lenDemangledName = StrLenT(pMangledName);

            if (lenDemangledName >= maxDemangledNameLen)
            {
                lenDemangledName = maxDemangledNameLen - 1U;
            }

            memcpy(pDemangledName, pMangledName, lenDemangledName * sizeof(CharT));
            pDemangledName[lenDemangledName] = (CharT)('\0');
        }
    }
    else
    {
        lenDemangledName = DemangleExternCNameInternalVS(pMangledName, StrLenT(pMangledName), pDemangledName, maxDemangledNameLen);
    }

    return lenDemangledName;
}


unsigned SymbolEngine::DemangleExternCNameVS(const char* pMangledName, char* pDemangledName, unsigned maxDemangledNameLen)
{
    return DemangleExternCNameInternalVS(pMangledName, StrLenT(pMangledName), pDemangledName, maxDemangledNameLen);
}

unsigned SymbolEngine::DemangleExternCNameVS(const wchar_t* pMangledName, wchar_t* pDemangledName, unsigned maxDemangledNameLen)
{
    return DemangleExternCNameInternalVS(pMangledName, StrLenT(pMangledName), pDemangledName, maxDemangledNameLen);
}

unsigned SymbolEngine::DemangleExternCNameVS(const char* pMangledName, unsigned lenMangledName, char* pDemangledName, unsigned maxDemangledNameLen)
{
    return DemangleExternCNameInternalVS(pMangledName, lenMangledName, pDemangledName, maxDemangledNameLen);
}

unsigned SymbolEngine::DemangleExternCNameVS(const wchar_t* pMangledName, unsigned lenMangledName, wchar_t* pDemangledName, unsigned maxDemangledNameLen)
{
    return DemangleExternCNameInternalVS(pMangledName, lenMangledName, pDemangledName, maxDemangledNameLen);
}

unsigned SymbolEngine::DemangleNameVS(const char* pMangledName, char* pDemangledName, unsigned maxDemangledNameLen)
{
    return DemangleNameInternalVS(pMangledName, pDemangledName, maxDemangledNameLen);
}

unsigned SymbolEngine::DemangleNameVS(const wchar_t* pMangledName, wchar_t* pDemangledName, unsigned maxDemangledNameLen)
{
    return DemangleNameInternalVS(pMangledName, pDemangledName, maxDemangledNameLen);
}

wchar_t* SymbolEngine::DemangleNameVS(const char* pMangledName)
{
    const char* pDemangledNameAnsi;

    char demangledNameBuffer[2048];

    if (0U != DemangleNameVS(pMangledName, demangledNameBuffer, 2048))
    {
        pDemangledNameAnsi = demangledNameBuffer;
    }
    else
    {
        pDemangledNameAnsi = pMangledName;
    }

    return AnsiToUnicode(pDemangledNameAnsi);
}

wchar_t* SymbolEngine::AnsiToUnicode(const char* pAnsi, int lenAnsi)
{
    wchar_t* pUnicode = NULL;

    if (NULL != pAnsi)
    {
        if (0 > lenAnsi)
        {
            lenAnsi = static_cast<int>(strlen(pAnsi)) + 1;
        }

        int lenUnicode = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pAnsi, lenAnsi, NULL, 0);

        if (0 < lenUnicode)
        {
            pUnicode = new wchar_t[lenUnicode];

            if (NULL != pUnicode)
            {
                if (0 == MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pAnsi, lenAnsi, pUnicode, lenUnicode))
                {
                    delete [] pUnicode;
                    pUnicode = NULL;
                }
            }
        }
    }

    return pUnicode;
}
