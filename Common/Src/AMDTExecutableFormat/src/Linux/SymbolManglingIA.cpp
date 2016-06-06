//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <SymbolEngine.h>
#include <cxxabi.h>

static wchar_t* AnsiToWide(const char* pAnsi);


unsigned SymbolEngine::DemangleNameIA(const char* pMangledName, char* pDemangledName, unsigned maxDemangledNameLen)
{
    unsigned len;
    int status = 0;
    char* pDemangledNameCxa = abi::__cxa_demangle(pMangledName, NULL, NULL, &status);

    if (NULL != pDemangledNameCxa && 0 == status)
    {
        len = static_cast<unsigned>(strlen(pDemangledNameCxa));

        if ((len - 1U) <= ((maxDemangledNameLen - 1U) - 1U))
        {
            memcpy(pDemangledName, pDemangledNameCxa, len);
        }
        else
        {
            len = 0U;
        }
    }
    else
    {
        len = 0U;
    }

    if (NULL != pDemangledNameCxa)
    {
        free(pDemangledNameCxa);
    }

    return len;
}

wchar_t* SymbolEngine::DemangleNameIA(const char* pMangledName)
{
    int status = 0;
    char* pDemangledName = abi::__cxa_demangle(pMangledName, NULL, NULL, &status);

    if (NULL != pDemangledName && 0 == status && '\0' != pDemangledName[0])
    {
        pMangledName = pDemangledName;
    }

    wchar_t* pDemangledNameWc = AnsiToWide(pMangledName);

    if (NULL != pDemangledName)
    {
        free(pDemangledName);
    }

    return pDemangledNameWc;
}


static wchar_t* AnsiToWide(const char* pAnsi)
{
    wchar_t* pWide = NULL;

    // Calculate the size needed in wchar_t
    size_t len = mbstowcs(NULL, pAnsi, 0);

    if ((size_t)(-1) != len)
    {
        pWide = new wchar_t[len + 1];

        if (NULL != pWide)
        {
            if ((size_t)(-1) != mbstowcs(pWide, pAnsi, len))
            {
                pWide[len] = L'\0';
            }
            else
            {
                delete [] pWide;
                pWide = NULL;
            }
        }
    }

    return pWide;
}
