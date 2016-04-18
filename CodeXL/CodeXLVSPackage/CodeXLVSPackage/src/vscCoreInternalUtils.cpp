//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCoreInternalUtils.cpp
///
//==================================================================================

#include <Include/vscCoreInternalUtils.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <algorithm>
#include <string>
#include <vector>
#include <locale>
#include <clocale>


wchar_t* vscAllocateAndCopy(const gtString& str)
{
    wchar_t* ret = NULL;
    size_t sz = (str.length() + 1);
    ret = new wchar_t[sz];
    const wchar_t* pUnderlyingBuffer = str.asCharArray();
    std::copy(pUnderlyingBuffer, pUnderlyingBuffer + sz, ret);
    ret[sz - 1] = L'\0';
    return ret;
}

char* vscAllocateAndCopy(const std::string& str)
{
    char* ret = NULL;
    size_t sz = (str.length() + 1);
    ret = new char[sz];
    const char* pUnderlyingBuffer = str.c_str();
    std::copy(pUnderlyingBuffer, pUnderlyingBuffer + sz, ret);
    ret[sz - 1] = '\0';
    return ret;
}

std::string vscWstrToStr(std::wstring const& text)
{
    std::locale const loc("");
    wchar_t const* from = text.c_str();
    std::size_t const len = text.size();
    std::vector<char> buffer(len + 1);
    std::use_facet<std::ctype<wchar_t>>(loc).narrow(from, from + len, '_', &buffer[0]);
    return std::string(&buffer[0], &buffer[len]);
}
