//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscVsUtils.h
///
//==================================================================================
#ifndef vscVsUtils_h__
#define vscVsUtils_h__
#include <string>
#include <vector>
#include <assert.h>

// Replaces all the occurrences of substrToReplace in target with replacementStr.
void vspReplaceAllOccurrences(std::string& target,
                              const std::string& substrToReplace, const std::string& replacementStr);

// Replaces all the occurrences of substrToReplace in target with replacementStr.
// This is the wide character version of the function.
void vspVsUtilsWReplaceAllOccurrences(std::wstring& target,
                                      const std::wstring& substrToReplace, const std::wstring& replacementStr);

// Returns true if the strings are equal (comparison is ignoring the case).
bool vspVsUtilsIsEqualNoCase(const std::wstring& strA, const std::wstring& strB);

// This function imitates gtString's removeTrailing function.
void vspVsUtilsRemoveTrailing(std::wstring& in, wchar_t c);

// This function imitates gtString's removeTrailing function.
void vspVsUtilsRemoveAllSpaces(std::wstring& str);

void vspVsUtilsWstrVectorToWstrArray(const std::vector<std::wstring>& in, wchar_t**& pOutBuffer, int& outArrSizeBuffer);

void vspVsUtilsDeleteWcharBuffersArray(wchar_t**& pBuffersArray, int buffersArraySize);

// TO_DO: Replace with a mechanism that calls GT_ASSERT:
#define VSP_ASSERT(cond) assert(cond)

#endif // vscVsUtils_h__
