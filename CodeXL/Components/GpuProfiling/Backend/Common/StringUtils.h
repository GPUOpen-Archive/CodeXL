//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains utility functions to manipulate strings.
//==============================================================================

#ifndef _STRINGUTILS_H_
#define _STRINGUTILS_H_

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "OSUtils.h"

/// \addtogroup Common
// @{

/// Utility functions to manipulate strings
namespace StringUtils
{
/// Convert from char string to wchar string
/// \param str the input char string
/// \return the wchar string
std::wstring StringToWString(const std::string& str);

/// Convert from wchar string to UTF8 encoded char string
/// \param org the input wchar string
/// \param dst the output UTF8 encoded char string
/// \return 0 if successful, otherwise error code is returned.
int WideStringToUtf8String(const std::wstring& org, std::string& dst);

/// Convert from UTF8 encoded char string to wchar string
/// \param org the input UTF8 encoded char string
/// \param dst the output wchar string
/// \return 0 if successful, otherwise error code is returned.
int Utf8StringToWideString(const std::string& org, std::wstring& dst);

/// Convert input to string
/// \param val Input var
/// \return the string
template <class T>
std::string ToString(const T val)
{
    std::stringstream ss(std::stringstream::in | std::stringstream::out);
    ss << val;
    return ss.str();
}

/// Convert input to string
/// \param val Input var
/// \param precision specify the precision
/// \return the string
template <class T>
std::string ToString(const T val, int precision)
{
    std::stringstream ss(std::stringstream::in | std::stringstream::out);
    ss.precision(precision);
    ss << std::fixed << val;
    return ss.str();
}

/// Convert input to string
/// \param val Input var
/// \return the string
const char* ToString(bool val);

template <class T>
struct vector_traits {};

/// Convert vector type to string
/// \param vec Input vector
/// \return the string version of the vector
template <class T>
std::string VecToString(T vec)
{
    std::stringstream ss(std::stringstream::in | std::stringstream::out);
    typedef vector_traits<T> traits_type;
    typedef typename traits_type::vec_type vec_type;
    vec_type dim = vector_traits<T>::getDim(vec);

    if (dim == 0)
    {
        return "";
    }

    ss << "{ ";

    switch (dim)
    {
        case 1:
            ss << traits_type::getX(vec);
            break;

        case 2:
            ss << traits_type::getX(vec)
               << ", "
               << traits_type::getY(vec);
            break;

        case 3:
            ss << traits_type::getX(vec)
               << ", "
               << traits_type::getY(vec)
               << ", "
               << traits_type::getZ(vec);
            break;
    }

    ss << " }";

    return ss.str();
}

/// Convert input to hex string
/// \param value Input var
/// \return the hex string
template <typename T>
std::string ToHexString(T value)
{
    if (value == 0)
    {
        return "NULL";
    }

    std::ostringstream ss;
#ifdef _WIN32
    ss << "0x" << std::hex << std::uppercase << value;
#else // _LINUX || LINUX
    ss << std::hex << std::uppercase << value;
#endif
    return ss.str();
}

/// Parse string
/// \param[in] str input string
/// \param[out] output output data
/// \return true if succeed
template <typename T>
bool Parse(std::string str, T& output)
{
    std::stringstream ss(str);
    T ret;
    ss >> ret;

    if (ss.fail())
    {
        return false;
    }
    else
    {
        output = ret;
        return true;
    }
}

/// Get time string
/// \return time string
std::string GetTimeString();

/// Convert nanosecond to millisec in string format
/// \param ullTime Time in nanosecond
/// \return time in millisec in string
std::string NanosecToMillisec(ULONGLONG ullTime);

/// Convert size in byte to string format
/// \param sizeInByte size in byte
/// \param precision Precision
/// \return data size in string
std::string GetDataSizeStr(unsigned int sizeInByte, int precision);

/// To lower case
/// \param str input string
/// \return lower case of str
std::wstring ToLowerW(const std::wstring& str);

/// Trim tailing and leading space and tab
/// \param str input string
/// \return trimed string
std::string Trim(const std::string& str);

/// Helper function that returns format string
/// \param pFmt format char string
/// \param ... parameter list
/// \return format string
std::string FormatString(const char* pFmt, ...);

/// Replace all original string in input string with replace string
/// \param input input string
/// \param original string to be replaced with
/// \param replace string to replace
/// \return replaced string
std::string Replace(const std::string& input, const std::string& original, const std::string& replace);

/// Parse Major.Minor Version string
/// \param[in] strVersion input version string
/// \param[out] majorVer major version number
/// \param[out] minorVer minor version number
/// \return true if operation succeeded
bool ParseMajorMinorVersion(const std::string& strVersion, unsigned int& majorVer, unsigned int& minorVer);

/// Get number of lines in the string
/// \param str input string
/// \return number of lines
int GetNumLines(const std::string& str);

/// Insert leading space
/// \param input input string
/// \param strLen desired string length
/// \return new string
std::string InsertLeadingSpace(const std::string& input, size_t strLen);

/// Split input string into tokens
/// \param[out] output output vector
/// \param[in] input input string
/// \param[in] delimiters input delimiter list
/// \param[in] trim trim spaces for output tokens
/// \param[in] removeEmptyStr remove empty strings in output tokens
void Split(std::vector<std::string>& output, const std::string& input, const std::string& delimiters, bool trim = true, bool removeEmptyStr = false);

/// Strips leading/trailing brackets (i.e. "[" and "]" from the input string
/// \param input the input string
/// \return the input string with leading/trailing brackets stripped off
std::string StripBrackets(const std::string& input);

/// Trim  leading white spaces
/// \param[in,out] input string
/// \return reference to trimmed string
std::string& TrimLeft(std::string& s);

/// Trim  trailing  white spaces
/// \param[in,out] input string
/// \return reference to trimmed string
std::string& TrimRight(std::string& s);

/// Trim  leading  and trailing white spaces
/// \param[in,out] input string
/// \return reference to trimmed string
std::string& TrimInPlace(std::string& s);

} // StringUtils

// @}

#endif // _STRINGUTILS_H_

