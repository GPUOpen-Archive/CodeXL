//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains utility functions to manipulate strings.
//==============================================================================

#ifdef _WIN32
    #include <windows.h>
#else
    // Include the UTF8 header from /common/Lib/Ext/utf8cpp/source/
    #include <utf8.h>
#endif //_WIN32
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <assert.h>
#include <ctime>
#include <stdarg.h>
#include <cstdio>
#include "Defs.h"
#include "StringUtils.h"
#include "Logger.h"

using namespace std;
#ifdef _WIN32
using namespace GPULogger;


wstring StringUtils::StringToWString(const string& str)
{
    int len;
    int slength = (int)str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
    wchar_t* pBuf = new(nothrow) wchar_t[len];
    SpAssertRet(pBuf != NULL) wstring();
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, pBuf, len);
    wstring r(pBuf);
    delete[] pBuf;
    return r;
}

#else

wstring StringUtils::StringToWString(const string& str)
{
    (void)(str); // Unused variable
    SP_TODO("Implement this function.")
    SpAssert(!"Not implemented");
    return wstring();
}

#endif //_WIN32

string StringUtils::GetTimeString()
{
    time_t rawtime;
    time(&rawtime);

#ifdef _WIN32
    char timebuf[26];
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    asctime_s(timebuf, 26, &timeinfo);
    return string(timebuf);
#else
    struct tm* pTimeinfo;
    pTimeinfo = localtime(&rawtime);
    return string(asctime(pTimeinfo));
#endif

}

wstring StringUtils::ToLowerW(const wstring& str)
{
    wstring res = str;

    for (size_t i = 0; i < str.length(); i++)
    {
        res[i] = towlower(str[i]);
    }

    return res;
}

string StringUtils::FormatString(const char* pFmt, ...)
{
    static const unsigned int MAX_FORMAT_STRING_LEN = 32768;
    char str[MAX_FORMAT_STRING_LEN] = "";
    va_list arg_ptr;

    va_start(arg_ptr, pFmt);
#ifdef _WIN32
    vsprintf_s(str, MAX_FORMAT_STRING_LEN, pFmt, arg_ptr);
#else
    vsprintf(str, pFmt, arg_ptr);
#endif
    va_end(arg_ptr);

    return string(str);
}

bool StringUtils::ParseMajorMinorVersion(const string& strVersion, unsigned int& majorVer, unsigned int& minorVer)
{
    bool retVal = false;

    size_t dotIdx = strVersion.find(".");

    if (dotIdx != string::npos)
    {
        string strMajor = strVersion.substr(0, dotIdx);
        string strMinor = strVersion.substr(dotIdx + 1);

        try
        {
            majorVer = stoi(strMajor);
            minorVer = stoi(strMinor);
            retVal = true;
        }
        catch (...)
        {
            // failed to parse version string
        }
    }

    return retVal;
}


string StringUtils::Trim(const string& str)
{
    string ret;
    ret.clear();
    size_t endpos = str.find_last_not_of(" \t");

    if (string::npos != endpos)
    {
        ret = str.substr(0, endpos + 1);
    }

    size_t startpos = ret.find_first_not_of(" \t");

    if (string::npos != startpos)
    {
        ret = ret.substr(startpos);
    }

    return ret;
}

string StringUtils::Replace(const string& input, const string& original, const string& replace)
{
    size_t lookHere = 0;
    size_t foundHere;
    string ret = input;

    while ((foundHere = ret.find(original, lookHere)) != string::npos)
    {
        ret.replace(foundHere, original.size(), replace);
        lookHere = foundHere + replace.size();
    }

    return ret;
}

int StringUtils::GetNumLines(const string& str)
{
    int ret = 0;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] == '\n')
        {
            ret++;
        }
    }

    return ret;
}

/// Convert nanosecond to millisec in string format
string StringUtils::NanosecToMillisec(ULONGLONG ullTime)
{
    if (ullTime == 0)
    {
        return "0";
    }
    else
    {
        return ToStringPrecision(ullTime * 1e-6, 5);
    }
}

string StringUtils::GetDataSizeStr(size_t sizeInByte, int precision)
{
    if (sizeInByte == 0)
    {
        return "0 Byte";
    }

    double newSize = 0;
    stringstream ss;
    size_t kb = 1 << 10;
    size_t mb = 1 << 20;
    size_t gb = 1 << 30;

    if (sizeInByte > gb)
    {
        newSize = static_cast<double>(sizeInByte) / static_cast<double>(gb);
        ss << ToStringPrecision(newSize, precision);
        ss << " GB";
    }
    else if (sizeInByte > mb)
    {
        newSize = static_cast<double>(sizeInByte) / static_cast<double>(mb);
        ss << ToStringPrecision(newSize, precision);
        ss << " MB";
    }
    else if (sizeInByte > kb)
    {
        newSize = static_cast<double>(sizeInByte) / static_cast<double>(kb);
        ss << ToStringPrecision(newSize, precision);
        ss << " KB";
    }
    else
    {
        newSize = static_cast<double>(sizeInByte);
        ss << ToStringPrecision(newSize, precision);
        ss << " Byte";
    }

    return ss.str();
}

string StringUtils::InsertLeadingSpace(const string& input, size_t strLen)
{
    if (input.length() >= strLen)
    {
        return input;
    }
    else
    {
        stringstream ss;
        size_t num = strLen - input.length();

        for (size_t i = 0; i < num; i++)
        {
            ss << ' ';
        }

        ss << input;
        return ss.str();
    }
}

void StringUtils::Split(vector<string>& output, const string& input, const string& delimiters, bool trim, bool removeEmptyStr)
{
    size_t strLen = input.length();
    size_t dNum = delimiters.length();
    stringstream ss;
    bool bMatch = false;

    for (size_t i = 0; i < strLen; i++)
    {
        char ch = input[i];

        for (size_t j = 0; j < dNum; j++)
        {
            char d = delimiters[j];

            if (d == ch)
            {
                // eof corrent token
                bMatch = true;
                break;
            }
        }

        if (bMatch)
        {
            string str = ss.str();

            if (trim)
            {
                str = Trim(str);
            }

            if (removeEmptyStr)
            {
                if (!str.empty())
                {
                    output.push_back(str);
                }
            }
            else
            {
                output.push_back(str);
            }

            bMatch = false;
            ss.str("");
        }
        else
        {
            ss << ch;
        }
    }

    // handle the remaining string after the last delimiter
    string str = ss.str();

    if (trim)
    {
        str = Trim(str);
    }

    if (removeEmptyStr)
    {
        if (!str.empty())
        {
            output.push_back(str);
        }
    }
    else
    {
        output.push_back(str);
    }
}

const char* StringUtils::ToString(bool val)
{
    return (val ? "true" : "false");
}

string StringUtils::StripBrackets(const string& input)
{
    string retVal = Trim(input);

    while (retVal[0] == '[' && retVal[retVal.length() - 1] == ']')
    {
        retVal = Trim(retVal.substr(1, retVal.length() - 2));
    }

    return retVal;
}

string& StringUtils::TrimLeft(string& s)
{
    //erase chars till  first not white spaces from string begin
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
    return s;

}
string& StringUtils::TrimRight(string& s)
{
    //erase chars till  first not white spaces from string end
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}
string& StringUtils::TrimInPlace(string& s)
{
    return TrimLeft(TrimRight(s));
}

#ifdef _WIN32
// ---------------------------------------------------------------------------
// Name:        WideStringToUtf8String
// Description: Convert a string expressed as an array of wide chars to an array of UTF8 encoded single chars
// Arguments:   const wstring& org - [INPUT] the original wide char string
//              string &dst - [OUTPUT] the UTF8 encoded multibyte string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      Doron Ofek
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int StringUtils::WideStringToUtf8String(const wstring& org, string& dst)
{
    unsigned int error = 0;
    dst.clear();

    if (org.size() == 0)
    {
        return 0;
    }

    // Step #1: get the number of characters length of the Get length (in chars) of resulting UTF8 string
    //
    const int dstLength = ::WideCharToMultiByte(CP_UTF8,            // Code Page = UTF8
                                                0,              // Use the default flags
                                                org.data(),     // Source wide string
                                                static_cast<int>(org.length()),   // Length of source string
                                                NULL,           // Not required as we're only calculating the output length
                                                0,              // Set to zero to force calculation without actual conversion
                                                NULL, NULL      // unused when only doing calculation
                                               );

    if (0 == dstLength)
    {
        // Error
        error = ::GetLastError();
        return error;
    }

    // Prepare the output string for the UTF8 string
    dst.resize(dstLength);

    // Do the actual conversion to UTF8
    int rc = ::WideCharToMultiByte(CP_UTF8,           // Code Page = UTF8
                                   0,                // Use the default flags
                                   org.data(),       // Source wide string
                                   static_cast<int>(org.length()),     // Length of source string
                                   &dst[0],          // Not required as we're only calculating the output length
                                   static_cast<int>(dst.length()),     // Address of the output buffer
                                   NULL, NULL        // Length of the output buffer
                                  );

    if (0 == rc)
    {
        // Error
        error = ::GetLastError();
        return error;
    }

    return error;
}

// ---------------------------------------------------------------------------
// Name:        Utf8StringToWideString
// Description: Convert a string expressed as an array of UTF8 encoded single chars to an array of wide chars
// Arguments:   const wstring& org - [INPUT] the original UTF8 encoded multibyte string
//              string &dst - [OUTPUT] the wide char string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      Doron Ofek
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int StringUtils::Utf8StringToWideString(const string& org, wstring& dst)
{
    unsigned int error = 0;
    dst.clear();

    if (org.size() == 0)
    {
        return 0;
    }

    // Step #1: get the number of characters length of the Get length (in chars) of resulting UTF8 string
    //
    const int dstLength = ::MultiByteToWideChar(CP_UTF8,            // Code Page = UTF8
                                                0,              // Use the default flags
                                                org.data(),     // Source string
                                                static_cast<int>(org.length()),   // Length of source string
                                                NULL,           // Not required as we're only calculating the output length
                                                0               // Set to zero to force calculation without actual conversion
                                               );

    if (0 == dstLength)
    {
        // Error
        error = ::GetLastError();
        return error;
    }

    // Prepare the output string for the wide string
    dst.resize(dstLength);

    // Do the actual conversion from UTF8
    int rc = ::MultiByteToWideChar(CP_UTF8,           // Code Page = UTF8
                                   0,                // Use the default flags
                                   org.data(),       // Source wide string
                                   static_cast<int>(org.length()),     // WideCharToMultiByte will deduce the string length based on null termination
                                   &dst[0],          // Address of the output buffer
                                   static_cast<int>(dst.length())      // Length of the output buffer
                                  );

    if (0 == rc)
    {
        // Error
        error = ::GetLastError();
        return error;
    }

    return error;
}

#else
// ---------------------------------------------------------------------------
// Name:        gtWideStringToUtf8String
// Description: Linux version: Convert a string expressed as an array of wide chars to an array of UTF8 encoded single chars
// Arguments:   const wstring& org - [INPUT] the original wide char string
//              string &dst - [OUTPUT] the UTF8 encoded multibyte string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      Doron Ofek
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int StringUtils::WideStringToUtf8String(const wstring& org, string& dst)
{
    int retVal = 0;
    dst.clear();

    try
    {
        utf8::utf32to8(org.begin(), org.end(), back_inserter(dst));
    }
    catch (...)
    {
        retVal = -1;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        Utf8StringToWideString
// Description: Linux version: Convert a string expressed as an array of UTF8 encoded single chars to an array of wide chars
// Arguments:   const wstring& org - [INPUT] the original UTF8 encoded multibyte string
//              string &dst - [OUTPUT] the wide char string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      Doron Ofek
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int StringUtils::Utf8StringToWideString(const string& org, wstring& dst)
{
    int retVal = 0;
    dst.clear();

    try
    {
        utf8::utf8to32(org.begin(), org.end(), back_inserter(dst));
    }
    catch (...)
    {
        retVal = -1;
    }

    return retVal;
}

#endif // _WIN32
