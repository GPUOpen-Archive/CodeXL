//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscVsUtils.cpp
///
//==================================================================================
#include "stdafx.h"
#include "vscVsUtils.h"
#include <vector>
#include <string>
#include <algorithm>

// Returns true iff a substrToReplace was found in str and was replaced with replacementStr.
template<typename TSTR>
static bool replaceSubstr(TSTR& str, const TSTR& substrToReplace, const TSTR& replacementStr)
{
    bool ret = false;
    size_t start_pos = str.find(substrToReplace);

    if (start_pos != TSTR::npos)
    {
        str.replace(start_pos, substrToReplace.length(), replacementStr);
        ret = true;
    }

    return ret;
}

void vspVsUtilsWReplaceAllOccurrences(std::wstring& target,
                                      const std::wstring& substrToReplace, const std::wstring& replacementStr)
{
    while (!target.empty() && replaceSubstr(target, substrToReplace, replacementStr));
}

void vspReplaceAllOccurrences(std::string& target, const std::string& substrToReplace, const std::string& replacementStr)
{
    while (!target.empty() && replaceSubstr(target, substrToReplace, replacementStr));
}

bool vspVsUtilsIsEqualNoCase(const std::wstring& strA, const std::wstring& strB)
{
    // First, create a copy of the two strings, and then work on the copies.
    std::wstring strAasLower;
    strAasLower.resize(strA.length());
    std::wstring strBasLower;
    strBasLower.resize(strB.length());

    // Ensure that the first string's copy is in lower case.
    std::transform(strA.cbegin(), strA.cend(), strAasLower.begin(), ::tolower);

    // Ensure that the second string's copy is in lower case.
    std::transform(strB.cbegin(), strB.cend(), strBasLower.begin(), ::tolower);

    // Check for equality.
    return (!strAasLower.compare(strBasLower));
}

void vspVsUtilsWstrVectorToWstrArray(const std::vector<std::wstring>& in, wchar_t**& pOutBuffer, int& outArrSizeBuffer)
{
    pOutBuffer = NULL;
    outArrSizeBuffer = 0;
    int sz = static_cast<int>(in.size());

    if (sz > 0)
    {
        // Allocate the output pointers array.
        pOutBuffer = new wchar_t* [sz];

        for (int i = 0; i < sz; i++)
        {
            size_t strSize = in[i].size() + 1;
            pOutBuffer[i] = new wchar_t[strSize];
            std::copy(in[i].begin(), in[i].end(), pOutBuffer[i]);
            pOutBuffer[i][strSize - 1] = L'\0';
        }

        outArrSizeBuffer = sz;
    }
}

void vspVsUtilsRemoveTrailing(std::wstring& in, wchar_t c)
{
    if (in.length() > 0)
    {
        std::wstring::iterator startIter = in.begin();
        std::wstring::iterator endIter = in.end();

        // Look for the position of the last wchar_t that is not the input char:
        std::wstring::iterator iter = endIter;

        while (iter != startIter)
        {
            iter--;

            if (*iter != c)
            {
                break;
            }
        }

        // If there are trailing chars to be removed:
        if ((iter + 1) != endIter)
        {
            // Remove them:
            in.erase(iter + 1, endIter);
        }
    }
}

void vspVsUtilsRemoveAllSpaces(std::wstring& str)
{
    std::wstring::iterator end_pos = std::remove(str.begin(), str.end(), L' ');
    str.erase(end_pos, str.end());
}

void vspVsUtilsDeleteWcharBuffersArray(wchar_t**& pBuffersArray, int buffersArraySize)
{
    if (buffersArraySize > 0)
    {
        // Delete all buffers.
        for (int i = 0; i < buffersArraySize; i++)
        {
            delete[] pBuffersArray[i];
            pBuffersArray[i] = NULL;
        }

        // Delete the buffers array.
        delete[] pBuffersArray;
        pBuffersArray = NULL;
    }
}
