//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtString.cpp
///
//=====================================================================

//------------------------------ gtString.cpp ------------------------------

// C:
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <windows.h>
#else
    #include <Lib/Ext/utf8cpp/source/utf8.h>
#endif

// General Strings:
#define GT_STR_KilobytesShort L"KB"
#define GT_STR_MegabytesShort L"MB"
#define GT_STR_BytesShort L"bytes"

// ---------------------------------------------------------------------------
// Name:        gtString::gtString
// Description: Constructor - Initializes an empty string
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString::gtString()
    : _stringAsASCIICharArray(nullptr)
{
}


// ---------------------------------------------------------------------------
// Name:        gtString::gtString
// Description: Copy constructor - Create me from another string
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString::gtString(const gtString& otherString)
    : _impl(otherString._impl), _stringAsASCIICharArray(nullptr)
{
}


// ---------------------------------------------------------------------------
// Name:        gtString::gtString
// Description: Constructor - initialize self by copying a string from a wchar array.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString::gtString(const wchar_t* pOtherString)
    : _impl((pOtherString != nullptr) ? pOtherString : L""), _stringAsASCIICharArray(nullptr)
{
}

gtString::gtString(const wchar_t* pOtherString, int len)
    : _impl(pOtherString, len), _stringAsASCIICharArray(nullptr)
{
}


// ---------------------------------------------------------------------------
// Name:        gtString::gtString
// Description: Constructor - Initialize self from a single char.
// Author:      AMD Developer Tools Team
// Date:        6/12/2004
// ---------------------------------------------------------------------------
gtString::gtString(wchar_t character)
    : _stringAsASCIICharArray(nullptr)
{
    operator+=(character);
}


// ---------------------------------------------------------------------------
// Name:        gtString::~gtString
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        15/11/2003
// ---------------------------------------------------------------------------
gtString::~gtString()
{
    if (_stringAsASCIICharArray != nullptr)
    {
        delete[] _stringAsASCIICharArray;
        _stringAsASCIICharArray = nullptr;
    }
}


#if AMDT_HAS_CPP0X

gtString::gtString(gtString&& otherString) : _impl(std::move(otherString._impl)),
    _stringAsASCIICharArray(otherString._stringAsASCIICharArray)
{
    otherString._stringAsASCIICharArray = nullptr;
}


gtString& gtString::operator=(gtString&& otherString)
{
    if (this != &otherString)
    {
        _impl.operator = (std::move(otherString._impl));

        if (nullptr != _stringAsASCIICharArray)
        {
            delete[] _stringAsASCIICharArray;
        }

        _stringAsASCIICharArray = otherString._stringAsASCIICharArray;
        otherString._stringAsASCIICharArray = nullptr;
    }

    return *this;
}


gtString& gtString::assign(gtString&& otherString)
{
    _impl.assign(std::move(otherString._impl));
    return *this;
}

#endif // AMDT_HAS_CPP0X


// ---------------------------------------------------------------------------
// Name:        gtString::asCharArray
// Description: Return this string as a nullptr terminated wchar array.
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
const wchar_t* gtString::asCharArray() const
{
    return _impl.c_str();
};

// ---------------------------------------------------------------------------
// TO_DO: Unicode - check performance and memory for this function
// Name:        gtString::asASCIICharArray
// Description: Return the string a ASCII characters pointer. Use this function
//              only if you have to, because its allocating memory, and copy data.
// Return Val:  const char*
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
const char* gtString::asASCIICharArray() const
{
    // Get the length:
    gtSize_t amountOfChars = length() + 1;
    gtSize_t currentBufferLength = 0;

    if (nullptr != _stringAsASCIICharArray)
    {
        currentBufferLength = strlen(_stringAsASCIICharArray) + 1;
    }

    if (amountOfChars != currentBufferLength)
    {
        delete[] _stringAsASCIICharArray;
        _stringAsASCIICharArray = nullptr;
    }

    if (amountOfChars > 0)
    {
        // Convert me to a char*
        _stringAsASCIICharArray = new char[amountOfChars];

        gtSize_t rc = gtUnicodeStringToASCIIString(_impl.c_str(), _stringAsASCIICharArray, amountOfChars);
        GT_ASSERT(rc == 0);
    }

    return _stringAsASCIICharArray;
}

// ---------------------------------------------------------------------------
// TO_DO: Unicode - check performance and memory for this function
// Name:        gtString::asASCIICharArray
// Description: Return the string a ASCII characters pointer. Use this function
//              only if you have to, because its allocating memory, and copy data.
// Arguments:   int amountOfCharactersToCopy - the amount of characters to copy
// Return Val:  const char*
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
const char* gtString::asASCIICharArray(int amountOfCharactersToCopy) const
{
    // Get the length:
    int amountOfChars = (int)length();

    delete[] _stringAsASCIICharArray;
    _stringAsASCIICharArray = nullptr;

    GT_IF_WITH_ASSERT(amountOfCharactersToCopy <= amountOfChars)
    {
        _stringAsASCIICharArray = new char[amountOfCharactersToCopy + 1];


        // Convert the string:
        gtSize_t charsConverted = gtUnicodeStringToASCIIString(_impl.c_str(), _stringAsASCIICharArray, amountOfChars);
        GT_ASSERT(charsConverted == 0);
    }

    return _stringAsASCIICharArray;
}

// ---------------------------------------------------------------------------
// Name:        gtString::asUTF8CharArray
// Description: Return the string a UTF8 characters pointer. Use this function
//              only if you have to, because its allocating memory, and copy data.
// Return Val:  const char*
// Author:      AMD Developer Tools Team
// Date:        16/1/2014
// ---------------------------------------------------------------------------
const char* gtString::asUTF8CharArray() const
{
    std::string utf8String;

    delete[] _stringAsASCIICharArray;
    _stringAsASCIICharArray = nullptr;

    int errorCode = gtWideStringToUtf8String(_impl, utf8String);

    GT_ASSERT(0 == errorCode);
    size_t convertedSize = utf8String.size() ;

    if (0 == errorCode)
    {
        _stringAsASCIICharArray = new char[convertedSize + 1];

        strncpy(_stringAsASCIICharArray, utf8String.c_str(), convertedSize);
        _stringAsASCIICharArray[convertedSize] = '\0';
    }

    return _stringAsASCIICharArray;
}

// ---------------------------------------------------------------------------
// Name:        gtString::length
// Description: Returns this string length (the amount of chars it holds)
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
int gtString::length() const
{
    return (int)_impl.length();
};


// ---------------------------------------------------------------------------
// Name:        gtString::length
// Description: Returns this string length in bytes
// Author:      AMD Developer Tools Team
// Date:        07/09/2010
// ---------------------------------------------------------------------------
int gtString::lengthInBytes() const
{
    // TO_DO: Unicode - use preprocessor definitions (http://www.firstobject.com/wchar_t-string-on-linux-osx-windows.htm)
    // (Static sizeof variable)
    return (int)_impl.length() * sizeof(wchar_t);
};


// ---------------------------------------------------------------------------
// Name:        gtString::isEmpty
// Description: Return true iff this string is empty.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
bool gtString::isEmpty() const
{
    return _impl.empty();
}


// ---------------------------------------------------------------------------
// Name:        gtString::makeEmpty
// Description: Makes this string an empty string
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString& gtString::makeEmpty()
{
    _impl.clear();

    return *this;
}

gtString& gtString::assign(wchar_t character, int count)
{
    if (0 <= count)
    {
        _impl.assign(count, character);
    }

    return *this;
}

gtString& gtString::assign(const wchar_t* pOtherString)
{
    if (nullptr != pOtherString)
    {
        _impl.assign(pOtherString);
    }

    return *this;
}

gtString& gtString::assign(const wchar_t* pOtherString, int length)
{
    if (nullptr != pOtherString)
    {
        _impl.assign(pOtherString, length);
    }

    return *this;
}

gtString& gtString::assign(const gtString& otherString)
{
    _impl.assign(otherString._impl);
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::append
// Description: Appends a single character to this string.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
gtString& gtString::append(wchar_t character)
{
    _impl += character;
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::append
// Description: Append another string into me.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString& gtString::append(const wchar_t* pOtherString)
{
    if (pOtherString != nullptr)
    {
        _impl.append(pOtherString);
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::append
// Description: Appends the first N charachters of another string to me.
// Arguments:   pOtherString - The other string.
//              length - The appended string length (N).
// Author:      AMD Developer Tools Team
// Date:        2/2/2005
// ---------------------------------------------------------------------------
gtString& gtString::append(const wchar_t* pOtherString, int length)
{
    if (pOtherString != nullptr)
    {
        _impl.append(pOtherString, length);
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::append
// Description: Append another string into me.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString& gtString::append(const gtString& otherString)
{
    _impl.append(otherString._impl);
    return *this;
}



// ---------------------------------------------------------------------------
// Name:        gtString::appendFormattedString
// Description: Appends a formatted string to this string.
//              The formatted string is given in sprintf notation
//              (See sprintf documentation for more details):
//              a. A format string.
//              b. A veriable list of arguments.
//
// Arguments:   pFormatString - the Format string
//              ...     - A variable list of arguments.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// Usage Sample:
//   gtString str;
//   str.appendFormattedString(L"Decimal: %d , Float:  %f", 7, 6.34);
//
// ---------------------------------------------------------------------------
gtString& gtString::appendFormattedString(const wchar_t* pFormatString, ...)
{
    // Get a pointer to the variable list of arguments:
    va_list argptr;
    va_start(argptr, pFormatString);

    // Yaki 10/6/2009:
    // On Windows, if the number of characters to write is greater than count, vsnprintf return -1 indicating that output has been truncated.
    // This means that Windows does not conforms to the C99 standard (as Linux and Mac OS X does).
    // Therefore, we have a different implementation for Windows and Linux :-)
    // (For more details see the documentation of the return value of vsnprintf in MSDN and Linux and Mac OS X man pages)
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        int buffSize = 1024;

        bool goOn = true;

        while (goOn)
        {
            goOn = false;

            // Allocate a buffer that will contain the formatted string:
            wchar_t* pBuff = new wchar_t[buffSize];


            // Write the formatted string into the buffer:
            int size = vswprintf(pBuff, buffSize - 1, pFormatString, argptr);

            // The buffer was big enough to contains the formatted string:
            if (0 <= size)
            {
                // Terminate the string manually:
                // (Some implementations of vsnprintf() don't nullptr terminate the string if
                //  there is not enough space for it)
                pBuff[size] = '\0';

                // Append the formatted string onto me:
                append(pBuff);
            }
            else
            {
                // The buffer wasn't big enough to contains the formatted string -
                // make it larger and try again:
                buffSize *= 2;
                goOn = true;
            }

            // Clean up:
            delete[] pBuff;
        }
    }
#else
    {
        // We are on Linux or Mac OS X:

        int buffSize = 1024;

        bool goOn = true;

        while (goOn)
        {
            goOn = false;

            wchar_t* pBuff = new wchar_t[buffSize];

            int size = vswprintf(pBuff, buffSize - 1, pFormatString, argptr);

            // The buffer was big enough to contains the formatted string:
            if (0 <= size)
            {
                // Terminate the string manually:
                // (Some implementations of vsnprintf() don't nullptr terminate the string if
                //  there is not enough space for it)
                pBuff[size] = '\0';

                // Append the formatted string onto me:
                append(pBuff);
            }
            else
            {
                // The buffer wasn't big enough to contains the formatted string -
                // make it larger and try again:
                va_end(argptr);
                buffSize *= 2;
                goOn = true;
                va_start(argptr, pFormatString);
            }

            delete[] pBuff;
        }

        /*
        // Get another pointer to the variable list of arguments:
        va_list argptr2;
        va_start(argptr2, pFormatString);

        // Calculate the formatted string size:
        int formattedStringSize = vswprintf(nullptr, 0, pFormatString, argptr2);
        if (0 < formattedStringSize)
        {
            // Allocate buffer that will contain the formatted string:
            int buffSize = formattedStringSize + 1;
            wchar_t* pBuff = new wchar_t[buffSize];


            // Restart the arguments pointer:
            va_start(argptr, pFormatString);

            // Write the formatted string into the buffer:
            int charsWritten = vswprintf(pBuff, buffSize, pFormatString, argptr);
            GT_IF_WITH_ASSERT(0 < charsWritten)
            {
            // nullptr terminate the formatted string:
                pBuff[charsWritten] = 0;

                // Append the formatted string onto me:
                append(pBuff);
            }

            // Release allocated memory:
            delete[] pBuff;

        }
        */
        // Terminate the second copy of the argptr pointer:
        //va_end(argptr2);

    }
#endif

    // Terminate the argptr pointer:
    va_end(argptr);

    return *this;
}

gtString& gtString::appendUnsignedIntNumber(unsigned int uintNumber)
{
    wchar_t buf[16];

    wchar_t* pWc = buf + (16 - 1);
    *pWc = L'\0';

    do
    {
        *(--pWc) = L'0' + uintNumber % 10;
        uintNumber /= 10;
    }
    while (uintNumber != 0U);

    return append(pWc, (int)((16 - 1) - (pWc - buf)));
}

// ---------------------------------------------------------------------------
// Name:        gtString::prepend
// Description: Prepends a single character to this string.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtString& gtString::prepend(wchar_t character)
{
    std::wstring temp = L"";
    temp += character;
    temp.append(_impl);
    _impl = temp;
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::prepend
// Description: Prepend another string into me.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtString& gtString::prepend(const wchar_t* pOtherString)
{
    if (pOtherString != nullptr)
    {
        std::wstring temp;
        temp.append(pOtherString);
        temp.append(_impl);
        _impl = temp;
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::prepend
// Description: Prepends the first N charachters of another string to me.
// Arguments:   pOtherString - The other string.
//              length - The appended string length (N).
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtString& gtString::prepend(const wchar_t* pOtherString, int length)
{
    if (pOtherString != nullptr)
    {
        std::wstring temp;
        temp.append(pOtherString, length);
        temp.append(_impl);
        _impl = temp;
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::prepend
// Description: Prepend another string into me.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtString& gtString::prepend(const gtString& otherString)
{
    std::wstring temp;
    temp.append(otherString._impl);
    temp.append(_impl);
    _impl = temp;
    return *this;
}



// ---------------------------------------------------------------------------
// Name:        gtString::prependFormattedString
// Description: Prepends a formatted string to this string.
//              The formatted string is given in sprintf notation
//              (See sprintf documentation for more details):
//              a. A format string.
//              b. A veriable list of arguments.
//
// Arguments:   pFormatString - the Format string
//              ...     - A variable list of arguments.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// Usage Sample:
//   gtString str = ", Info:";
//   str.prependFormattedString(L"Decimal: %d , Float:  %f", 7, 6.34);
//      result: str == "Decimal: 7 , Float:  6.34, Info:"
// ---------------------------------------------------------------------------
gtString& gtString::prependFormattedString(const wchar_t* pFormatString, ...)
{
    // Get a pointer to the variable list of arguments:
    va_list argptr;
    va_start(argptr, pFormatString);

    int buffSize = 1024;
    bool goOn = true;

    std::wstring temp;

    while (goOn)
    {
        goOn = false;

        // Allocate a buffer that will contain the formatted string:
        wchar_t* pBuff = new wchar_t[buffSize];



        // Write the formatted string into the buffer:
        int size = vswprintf(pBuff, buffSize, pFormatString, argptr);

        // The buffer was big enough to contains the formatted string:
        if (size > 0)
        {
            // Terminate the string manually:
            // (Some implementations of vsnprintf() don't nullptr terminate the string if
            //  there is not enough space for it)
            pBuff[size] = '\0';

            // Append the formatted string onto me:
            temp.append(pBuff);
        }
        else
        {
            // The buffer wasn't big enough to contains the formatted string -
            // make it larger and try again:
            buffSize *= 2;
            goOn = true;
        }

        // Clean up:
        delete[] pBuff;
    }

    // Terminate the argptr pointer:
    va_end(argptr);

    temp.append(_impl);

    _impl = temp;

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::find
// Description: Finds the first occurrence of a substring in this string.
//
// Arguments:   subString - The sub string to be searched.
//              searchStartPosition - The search start position.
//                                    (By default - we will start searching from
//                                     this string beginning).
// Return Val:  int - The index of the first occurrence of the subString in this string.
//                    Or -1 if this string does not contain an instance of the sub string.
//
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
int gtString::find(const gtString& subString, int searchStartPosition) const
{
    int retVal = (int)_impl.find(subString._impl, searchStartPosition);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::find
// Description: Finds the first occurrence of a character in this string.
//
// Arguments:   character - The character to be searched.
//              searchStartPosition - The search start position.
//                                    (By default - we will start searching from
//                                     this string beginning).
// Return Val:  int - The index of the first occurrence of the input character in this string.
//                    Or -1 if this string does not contain the input character.
//
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
int gtString::find(wchar_t character, int searchStartPosition) const
{
    int retVal = (int)_impl.find(character, searchStartPosition);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::findNextLine
// Description: finds the first new line (newline or carriage return character)
//              after searchStartPosition
// Arguments: searchStartPosition
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
int gtString::findNextLine(int searchStartPosition) const
{
    int nlPosition = find(L"\n", searchStartPosition);
    int crPosition = find(L"\r", searchStartPosition);
    int nextLineStart = -1;

    if ((crPosition != -1) && (nlPosition != -1))
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        nextLineStart = min(crPosition, nlPosition);
#else
        nextLineStart = std::min(crPosition, nlPosition);
#endif
    }
    else
    {
        // if we didn't find one of the line enders, choose the other or leave the value as -1
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        nextLineStart = max(nextLineStart, max(crPosition, nlPosition));
#else
        nextLineStart = std::max(nextLineStart, std::max(crPosition, nlPosition));
#endif
    }

    return nextLineStart;
}

// ---------------------------------------------------------------------------
// Name:        gtString::lineNumberFromCharacterIndex
// Description: returns the 0/1-based line number in which the character with the
//              given index resides, or -1 if the index is out of range.
// Author:      AMD Developer Tools Team
// Date:        19/10/2010
// ---------------------------------------------------------------------------
int gtString::lineNumberFromCharacterIndex(int characterIndex, bool oneBased) const
{
    int retVal = -1;

    if ((characterIndex > -1) && (characterIndex < length()))
    {
        int currentLineStart = -1;
        int currentLineNumber = oneBased ? 1 : 0;
        bool goOn = true;

        while (goOn)
        {
            currentLineStart = findNextLine(currentLineStart + 1);

            if (currentLineStart > characterIndex)
            {
                retVal = currentLineNumber;
                break;
            }
            else if (currentLineStart < 0)
            {
                // This should not happen, as any in-range character should be in SOME line:
                GT_ASSERT(false);
                goOn = false;
            }

            // Increment the line number:
            currentLineNumber++;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::reverseFind
// Description:
//   Search the string backwards, and finds the last occurrence of a substring
//   in this string.
//
// Arguments:   subString - The sub string to be searched.
//              searchStartPosition - The search start position.
//                                    (By default - we will start searching from
//                                     this string end (position = -1)).
// Return Val:  int - The index of the last occurrence of the subString in this string.
//                    Or -1 if this string does not contain an instance of the sub string.
//
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
int gtString::reverseFind(const gtString& subString, int searchStartPosition) const
{
    // If we were asked to start the search from the string end:
    if (searchStartPosition == -1)
    {
        searchStartPosition = (int)std::string::npos;
    }

    int retVal = (int)_impl.rfind(subString._impl, searchStartPosition);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::startsWith
// Description: Returns true iff this string starts with the input prefix string.
// Author:      AMD Developer Tools Team
// Date:        6/6/2005
// ---------------------------------------------------------------------------
bool gtString::startsWith(const gtString& prefixString) const
{
    bool retVal = false;

    // This string need to be at least as long as the prefix string:
    unsigned int prefixStringLength = prefixString.length();

    if (prefixStringLength <= _impl.length())
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        retVal = (_wcsnicmp(_impl.c_str(), prefixString.asCharArray(), prefixStringLength) == 0);
#else
        retVal = (wcsncasecmp(_impl.c_str(), prefixString.asCharArray(), prefixStringLength) == 0);
#endif
    }

    return retVal;
}

bool gtString::endsWith(const gtString& suffixString) const
{
    int suffixStringLength = suffixString.length();

    if (0 != suffixStringLength && suffixStringLength <= length())
    {
        const wchar_t* pStr = asCharArray() + length() - 1;
        const wchar_t* pSuffix = suffixString.asCharArray() + suffixStringLength - 1;

        do
        {
            if (tolower(*pStr) != tolower(*pSuffix))
            {
                break;
            }

            --pStr;
            --pSuffix;
        }
        while (0 != --suffixStringLength);
    }

    return (0 == suffixStringLength);
}

// ---------------------------------------------------------------------------
// Name:        gtString::onlyContainsCharacters
// Description: Returns true iff the string constitutes only characters in validCharacterList.
// Author:      AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool gtString::onlyContainsCharacters(const gtString& validCharacterList) const
{
    bool retVal = true;

    // Iterate this string's characters:
    int strLen = length();

    for (int i = 0; i < strLen; i++)
    {
        // Get the current character:
        wchar_t currentChar = (*this)[i];

        // If the i-th character is not in validCharacterList:
        if (validCharacterList.find(currentChar) < 0)
        {
            // Fail the validation:
            retVal = false;
            break;
        }
    }

    return retVal;
}


int gtString::findFirstOf(const gtString& characters, int searchStartPosition) const
{
    int retVal = (int)_impl.find_first_of(characters._impl, searchStartPosition);
    return retVal;
}



int gtString::findLastOf(const gtString& characters) const
{
    int retVal = (int)_impl.find_last_of(characters._impl);
    return retVal;
}


int gtString::findFirstNotOf(const gtString& characters, int searchStartPosition) const
{
    int retVal = (int)_impl.find_first_not_of(characters._impl, searchStartPosition);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::reverseFind
// Description:
//   Search the string backwards, and finds the last occurrence of a character
//   in this string.
//
// Arguments:   character - The character to be searched.
//              searchStartPosition - The search start position.
//                                    (By default - we will start searching from
//                                     this string end (position = -1))).
// Return Val:  int - The index of the last occurrence of the input character in this string.
//                    Or -1 if this string does not contain the input character.
//
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
int gtString::reverseFind(wchar_t character, int searchStartPosition) const
{
    // If we were asked to start the search from the string end:
    if (searchStartPosition == -1)
    {
        searchStartPosition = (int)std::string::npos;
    }

    int retVal = (int)_impl.rfind(character, searchStartPosition);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::count
// Description: Counts how many times does subString appear in the string.
//              in case of intersecting repeat, function DOES count repeats i.e.
//              "aaabbaaa".count("aa") = 4 because of
//               AAabbaaa, aAAbbaaa, aaabbAAa, aaabbaAA
// Arguments: subString - the string to be searched
//            countStartPosition - where to start counting
// Return Val: int
// Author:      AMD Developer Tools Team
// Date:        30/3/2008
// ---------------------------------------------------------------------------
int gtString::count(const gtString& subString, int countStartPosition) const
{
    int currentpos = countStartPosition;
    int counter = 0;

    while (currentpos != -1)
    {
        currentpos = find(subString, currentpos + 1);

        if (currentpos != -1)
        {
            counter++;
        }
    }

    return counter;
}

// ---------------------------------------------------------------------------
// Name:        gtString::count
// Description:  Counts how many times does character appear in the string.
// Arguments: character - the character to be searched
//            countStartPosition - where to start counting
// Return Val: int
// Author:      AMD Developer Tools Team
// Date:        30/3/2008
// ---------------------------------------------------------------------------
int gtString::count(wchar_t character, int countStartPosition) const
{
    int currentpos = countStartPosition;
    int counter = 0;

    while (currentpos != -1)
    {
        currentpos = find(character, currentpos + 1);

        if (currentpos != -1)
        {
            counter++;
        }
    }

    return counter;
}


// ---------------------------------------------------------------------------
// Name:        gtString::FromASCIIString
// Description: Create a string from ASCII string
// Arguments:   const char* pOtherString
// Return Val:  gtString&
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
gtString& gtString::fromASCIIString(const char* pOtherString)
{
    // Make me empty:
    makeEmpty();

    if (pOtherString != nullptr)
    {
        // Get the string length:
        gtSize_t strLength = strlen(pOtherString) + 1;

        if (strLength > 0)
        {
            // Allocate a unicode string:
            wchar_t* pUnicodeString = new wchar_t[strLength];


            // Convert the characters:
            gtSize_t rc = gtASCIIStringToUnicodeString(pOtherString, pUnicodeString, strLength);
            GT_IF_WITH_ASSERT(rc == 0)
            {
                // Add the unicode string to me:
                append(pUnicodeString);
            }

            // Release the allocated memory:
            delete[] pUnicodeString;
        }
    }

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::FromASCIIString
// Description: Create a string from ASCII string
// Arguments:   const char* pOtherString
// Return Val:  gtString&
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
gtString& gtString::fromASCIIString(const char* pOtherString, int stringLength)
{
    // Make me empty:
    makeEmpty();

    if (pOtherString != nullptr)
    {
        if (stringLength > 0)
        {
            // Allocate a unicode string:
            wchar_t* pUnicodeString = new wchar_t[stringLength + 1]();

            // Convert the characters:
            gtSize_t rc = gtASCIIStringToUnicodeString(pOtherString, pUnicodeString, stringLength + 1);
            GT_IF_WITH_ASSERT(rc == 0)
            {
                // Add the unicode string to me:
                append(pUnicodeString, stringLength);
            }

            // Release the allocated memory:
            delete[] pUnicodeString;
        }
    }

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::fromUtf8String
// Description: Create a string from UTF8 string
// Arguments:   const char* pOtherString
// Return Val:  gtString&
// Author:      AMD Developer Tools Team
// Date:        Oct-22, 2012
// ---------------------------------------------------------------------------
gtString& gtString::fromUtf8String(const char* pOtherString)
{
    // Make me empty:
    makeEmpty();

    if (pOtherString != nullptr)
    {
        std::string utf8String(pOtherString);
        int rc = gtUtf8StringToWideString(utf8String, _impl);
        GT_ASSERT(rc == 0);
    }

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::fromUtf8String
// Description: Create a string from UTF8 string
// Arguments:   const char* pOtherString
// Return Val:  gtString&
// Author:      AMD Developer Tools Team
// Date:        Oct-22, 2012
// ---------------------------------------------------------------------------
gtString& gtString::fromUtf8String(const std::string& utf8String)
{
    // Make me empty:
    makeEmpty();

    if (!utf8String.empty())
    {
        int rc = gtUtf8StringToWideString(utf8String, _impl);
        GT_ASSERT(rc == 0);
    }

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::isEqual
// Description: Comapre the input char* to me
// Arguments:   const char* pOtherString
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/9/2010
// ---------------------------------------------------------------------------
bool gtString::isEqual(const char* pOtherString)
{
    bool retVal = false;

    if (pOtherString != nullptr)
    {
        // Get the ASCII string length:
        gtSize_t length = strlen(pOtherString) + 1;

        // Allocate a unicode string:
        wchar_t* pUnicodeString = new wchar_t[length];


        gtSize_t rc = gtASCIIStringToUnicodeString(pOtherString, pUnicodeString, length);
        GT_IF_WITH_ASSERT(rc == 0)
        {
            // Build the string to compare to:
            gtString compareToStr(pUnicodeString);
            retVal = (compareToStr == (*this));
        }

        // Delete the allocated string:
        delete[] pUnicodeString;
    }
    else // pOtherString == nullptr
    {
        // The nullptr string is equal only to the empty string:
        retVal = isEmpty();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::operator=
// Description: Initialize this string from a single char.
// Author:      AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
gtString& gtString::operator=(wchar_t c)
{
    _impl.operator = (c);

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::operator=
// Description: Copy another string content into me.
// Return Val:  gtString - Return myself (This enables a user to write a = b = c;).
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString& gtString::operator=(const wchar_t* pOtherString)
{
    if (pOtherString != nullptr)
    {
        _impl.operator = (pOtherString);
    }
    else
    {
        makeEmpty();
    }

    // Return a reference to myself:
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::operator=
// Description: Copy another string content into me.
// Return Val:  gtString - Return myself (This enables a user to write a = b = c;).
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtString& gtString::operator=(const gtString& otherString)
{
    _impl.operator = (otherString._impl);

    // Retruns a reference to myself:
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::operator[]
// Description: Returns the i'th wchar in this string.
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
const wchar_t& gtString::operator[](int i) const
{
    return _impl.operator[](i);
}


// ---------------------------------------------------------------------------
// Name:        gtString::operator[]
// Description: Returns the i'th wchar in this string.
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
wchar_t& gtString::operator[](int i)
{
    return _impl.operator[](i);
}


// ---------------------------------------------------------------------------
// Name:        gtString::operator<
// Description: Returns true iff this string is smaller (lexicographic order)
//              than the other string.
// Return Val:  bool - true - Iff I am smaller than other.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
bool gtString::operator<(const gtString& otherString) const
{
    return _impl < otherString._impl;
}

// ---------------------------------------------------------------------------
// Name:        gtString::operator>
// Description: Returns true iff this string is bigger (lexicographic order)
//              than the other string.
// Return Val:  bool - true - Iff I am smaller than other.
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
bool gtString::operator>(const gtString& otherString) const
{
    return _impl > otherString._impl;
}

bool gtString::isEqualNoCase(const gtString& otherString) const
{
    bool retVal = false;

    int l = length();

    if (l == otherString.length())
    {
        retVal = true;

        for (int i = 0; i < l; ++i)
        {
            if (tolower((*this)[i]) != tolower(otherString[i]))
            {
                retVal = false;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::compareNoCase
// Description: Compares this string to another string in lexicographic order.
//              The string character cases (upper or lower) are ignored.
// Return Val:  int - 0 - the strings are identical (ignoring case).
//                   <0 - This string is less than the other string (ignoring case).
//                   >0 - This string is greater than the other string (ignoring case).
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
int gtString::compareNoCase(const gtString& otherString) const
{
    int retVal = 0;

    // Get this string and the other string in upper case format:
    gtString thisUpper = *this;
    thisUpper.toUpperCase();

    gtString otherUpper = otherString;
    otherUpper.toUpperCase();

    // Compare the upper case format:
    if (thisUpper < otherUpper)
    {
        retVal = -1;
    }
    else if (thisUpper > otherUpper)
    {
        retVal = 1;
    }

    return retVal;
}


int gtString::compare(const wchar_t* pOtherString) const
{
    return _impl.compare(pOtherString);
}

int gtString::compare(const gtString& otherString) const
{
    return _impl.compare(otherString._impl);
}

int gtString::compare(int pos, int len, const wchar_t* pOtherString) const
{
    return _impl.compare(pos, len, pOtherString);
}

int gtString::compare(int pos, int len, const gtString& otherString) const
{
    return _impl.compare(pos, len, otherString._impl);
}


// ---------------------------------------------------------------------------
// Name:        gtString::removeTrailing
// Description:
//   Removes trailing char's of a given input char.
//   Example:
//   gtString foo = "abcdddd";
//   foo.removeTrailing('d');
//   Will yield "abc" string
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
gtString& gtString::removeTrailing(wchar_t c)
{
    if (_impl.length() > 0)
    {
        std::wstring::iterator startIter = _impl.begin();
        std::wstring::iterator endIter = _impl.end();

        // Look for the position of the last wchar that is not the input char:
        std::wstring::iterator iter = endIter;

        while (iter != startIter)
        {
            --iter;

            if (*iter != c)
            {
                break;
            }
        }

        // If there are trailing chars to be removed:
        if ((iter + 1) != endIter)
        {
            // Remove them:
            _impl.erase(iter + 1, endIter);
        }
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::trim
// Description:
//   Removes leading and trailing whitespace characters from the string.
//   Example:
//   gtString foo = "  abc  ";
//   foo.trim();
//   Will yield "abc" string
// Author:      AMD Developer Tools Team
// Date:        Dec-25,2013
// ---------------------------------------------------------------------------
gtString& gtString::trim()
{
    if (_impl.length() > 0)
    {
        std::wstring::iterator startIter = _impl.begin();
        std::wstring::iterator endIter = _impl.end();

        // Look for the first non-whitespace character
        std::wstring::iterator iterFirst = startIter;

        while (iterFirst != endIter && iswspace(*iterFirst))
        {
            ++iterFirst;
        }

        if (iterFirst != endIter)
        {
            // Look for the position of the last non-whitespace
            std::wstring::iterator iterLast = endIter;
            --iterLast;

            while (iterLast != iterFirst && iswspace(*iterLast))
            {
                --iterLast;
            }

            // Note: It is critical to remove the trailing chars before removing
            // the leading chars. Removing the leading chars first would invalidate
            // the iterators iterLast and endIter and cause an exception when
            // removing the trailing chars.

            // If there are trailing whitespace chars to be removed
            if ((iterLast + 1) != endIter)
            {
                // Remove them:
                _impl.erase(iterLast + 1, endIter);
            }

            // If there are leading whitespace chars to be removed
            if (iterFirst != startIter)
            {
                // Remove them:
                _impl.erase(startIter, iterFirst);
            }
        }
        else
        {
            // The whole string is whitespace chars
            _impl.clear();
        }
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::getSubString
// Description: Returns a sub string, beginning at startPosition and ending at
//              endPosition.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
void gtString::getSubString(int startPosition, int endPosition, gtString& subString) const
{
    std::wstring subStr = _impl.substr(startPosition, (endPosition - startPosition + 1));
    subString = subStr.c_str();
}

// ---------------------------------------------------------------------------
// Name:        gtString::truncate
// Description: Sets this string to be a substring of itself starting at
//              startPosition and ending at endPosition (inclusive)
// Author:      AMD Developer Tools Team
// Date:        27/8/2008
// ---------------------------------------------------------------------------
gtString& gtString::truncate(int startPosition, int endPosition)
{
    if (startPosition < length())
    {
        _impl = _impl.substr(startPosition, (endPosition - startPosition + 1));
    }
    else
    {
        makeEmpty();
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::extruct
// Description: Remove a middle of the string and make the string by joining the left
//              part (before the middle) and the right part (after the middle)
// Author:      AMD Developer Tools Team
// Date:        20/6/2013
// ---------------------------------------------------------------------------
gtString& gtString::extruct(int startPosition, int endPosition)
{
    _impl.erase(startPosition, endPosition - startPosition);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::replace
// Description: Replaces a occurrences of a substring with another one.
// Arguments: oldSubString - The sub string to be replaced.
//            replacementString - The sub string that will replace oldSubString.
//            replaceAllOccurrences - true will replace all occurrences of oldSubString.
//                                    false will just replace the first occurrence.
// Return Val: int - The amount of occurrences replaced.
// Author:      AMD Developer Tools Team
// Date:        28/11/2005
// ---------------------------------------------------------------------------
int gtString::replace(const gtString& oldSubString, const gtString& newSubString,
                      bool replaceAllOccurrences)
{
    int retVal = 0;

    // Verify that this string is not empty:
    if (!isEmpty())
    {
        // Get the input sub strings lengths:
        int oldSubStrLen = oldSubString.length();
        int newSubStrLen = newSubString.length();

        // Will hold our current position in this string:
        int currentPos = 0;

        // While we didn't reach the end of the string:
        while (currentPos < length())
        {
            if (_impl[currentPos] != '\0')
            {
                // Look for the old sub string from our current position:
                currentPos = (int)_impl.find(oldSubString.asCharArray(), currentPos);

                if ((currentPos == -1) || (currentPos > length()))
                {
                    // The old sub string was not found - exit the loop:
                    break;
                }
                else
                {
                    // Replace this occurrence of the old string with the new one
                    _impl.replace(currentPos, oldSubStrLen, newSubString.asCharArray(), newSubStrLen);

                    // Update the current position:
                    currentPos += newSubStrLen;

                    // Increment the replace count
                    retVal++;

                    // If we were asked to replace only the first occurrence:
                    if (!replaceAllOccurrences)
                    {
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::replace
// Description: Replaces a occurrences of a substring with another one.
// Arguments:   startPos - The start position for the replace search.
//              endPos - The end position for the replace search.
//              oldSubString - The sub string to be replaced.
//              replacementString - The sub string that will replace oldSubString.
//              replaceAllOccurrences - true will replace all occurrences of oldSubString.
//                                    false will just replace the first occurrence.
// Return Val: int - The amount of occurrences replaced.
// Author:      AMD Developer Tools Team
// Date:        28/11/2005
// ---------------------------------------------------------------------------
int gtString::replace(int startPos, int endPos, const gtString& oldSubString, const gtString& newSubString,
                      bool replaceAllOccurrences)
{
    int retVal = 0;

    // Verify that this string is not empty:
    if (!isEmpty())
    {
        // Get the input sub strings lengths:
        int oldSubStrLen = oldSubString.length();
        int newSubStrLen = newSubString.length();

        if (startPos <= length())
        {
            // Will hold our current position in this string:
            int currentPos = startPos;

            // While we didn't reach the end of the string:
            while (_impl[currentPos] != '\0')
            {
                // Look for the old sub string from our current position:
                currentPos = (int)_impl.find(oldSubString.asCharArray(), currentPos);

                if (currentPos == -1)
                {
                    // The old sub string was not found - exit the loop:
                    break;
                }
                else if (currentPos > endPos)
                {
                    // The old sub string is out of range - exit the loop:
                    break;
                }
                else
                {
                    // Replace this occurrence of the old string with the new one
                    _impl.replace(currentPos, oldSubStrLen, newSubString.asCharArray(), newSubStrLen);

                    // Update the current position:
                    currentPos += newSubStrLen;

                    // Increment the replace count
                    retVal++;

                    // If we were asked to replace only the first occurrence:
                    if (!replaceAllOccurrences)
                    {
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gtString::toUpperCase
// Description: Converts a range of string characters to upper case.
// Arguments:   startPosition - The range start position.
//              endPosition - The range end position.
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
gtString& gtString::toUpperCase(int startPosition, int endPosition)
{
    if (endPosition == -1)
    {
        endPosition = this->length() - 1;
    }

    for (int i = startPosition; i <= endPosition; i++)
    {
        // Sanity check (wchar is signed):
        if (0 <= _impl[i])
        {
            // Make sure this is English:
            if (isascii(_impl[i]))
            {
                if (islower(_impl[i]))
                {
                    _impl[i] = (wchar_t)toupper(_impl[i]);
                }
            }
        }
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toLowerCase
// Description: Converts a range of string characters to lower case.
// Arguments:   startPosition - The range start position.
//              endPosition - The range end position.
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
gtString& gtString::toLowerCase(int startPosition, int endPosition)
{
    if (endPosition == -1)
    {
        endPosition = this->length() - 1;
    }

    for (int i = startPosition; i <= endPosition; i++)
    {
        // Sanity check (wchar is signed):
        if (0 <= _impl[i])
        {
            // Make sure this is English:
            if (isascii(_impl[i]))
            {
                if (isupper(_impl[i]))
                {
                    _impl[i] = (wchar_t)tolower(_impl[i]);
                }
            }
        }
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::isIntegerNumber
// Description: Returns true iff this string contains an integer number.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtString::isIntegerNumber() const
{
    bool retVal = false;

    // Verify that this is not an empty string:
    int strLength = (int)_impl.length();

    if (strLength > 0)
    {
        // If the prefix is a + or -:
        int i = 0;

        if ((_impl[i] == '-') || (_impl[i] == '+'))
        {
            i++;
        }

        // If the first wchar is a digit:
        if (gtIsDigit(_impl[i]))
        {
            // Assume that this string is an integer number until proven otherwise:
            retVal = true;

            int j = -1;

            // Verify that the other string chars are also integers or thousands separators:
            while (++i < strLength)
            {
                if (_impl[i] == GT_THOUSANDS_SEPARATOR)
                {
                    // If we have a thousands separator, make sure it is in a logical position:
                    if (j == -1)
                    {
                        j = i;
                    }
                    else
                    {
                        if (i == j + GT_THOUSANDS_SEPARATOR_DISTANCE + 1)
                        {
                            j = i;
                        }
                        else
                        {
                            retVal = false;
                            break;
                        }
                    }
                }
                else
                {
                    if (!gtIsDigit(_impl[i]))
                    {
                        retVal = false;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toIntNumber
// Description: Converts a string that contains an integer number into an int.
// Arguments:   integerNumber - Will get the output int.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtString::toIntNumber(int& intNumber) const
{
    bool retVal = false;

    // Convert the string into a long:
    long stringAsLong = 0;

    if (toLongNumber(stringAsLong))
    {
        // Verify that we are in the int range:
        if ((INT_MIN <= stringAsLong) && (stringAsLong <= INT_MAX))
        {
            intNumber = int(stringAsLong);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toUnsignedIntNumber
// Description: Converts a string that contains an unsigned integer number into
//              an unsigned int.
// Arguments:   uintNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtString::toUnsignedIntNumber(unsigned int& uintNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Will get the read number:
    unsigned int readNumber = 0;
    int rc1 = 0;

    // If the string is given in Hex (0x) format:
    int stringLength = this->length();

    if ((3 <= stringLength) && (_impl[0] == '0') && ((_impl[1] == 'x') || (_impl[1] == 'X')))
    {
        rc1 = swscanf(clone.asCharArray(), L"%x", &readNumber);
    }
    else
    {
        rc1 = swscanf(clone.asCharArray(), L"%u", &readNumber);
    }

    if (rc1 == 1)
    {
        uintNumber = readNumber;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toLongNumber
// Description: Converts a string that contains an integer number into a long.
// Arguments:   longNumber - Will get the output long.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtString::toLongNumber(long& longNumber) const
{
    bool retVal = false;

    // Verify that this string represents an integer number
    if (isIntegerNumber())
    {
        // Remove the thousands separators from the string:
        gtString clone = *this;
        clone.removeChar(GT_THOUSANDS_SEPARATOR);

        // Convert the string to an int:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        longNumber = _wtol(clone.asCharArray());
#else
        wchar_t* endPosition = nullptr;
        longNumber = wcstol(clone.asCharArray(), &endPosition, 10);
#endif
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toUnsignedLongNumber
// Description: Converts a string that contains an unsigned long number into
//              an unsigned long.
// Arguments:   ulongNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtString::toUnsignedLongNumber(unsigned long& ulongNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Will get the read number:
    unsigned long readNumber = 0;
    int rc1 = 0;

    // If the string is given in Hex (0x) format:
    int stringLength = this->length();

    if ((3 <= stringLength) && (_impl[0] == '0') && ((_impl[1] == 'x') || (_impl[1] == 'X')))
    {
        rc1 = swscanf(clone.asCharArray(), L"%lx", &readNumber);
    }
    else
    {
        // Convert the string into an unsigned int:
        rc1 = swscanf(clone.asCharArray(), L"%lu", &readNumber);
    }

    if (rc1 == 1)
    {
        ulongNumber = readNumber;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toLongLongNumber
// Description: Converts a string that contains a long long integer (64 bit integer)
//              number into a long long.
// Arguments:   longLongNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/2/2008
// ---------------------------------------------------------------------------
bool gtString::toLongLongNumber(long long& longLongNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Convert the string into an unsigned int:
    long long readNumber = 0;
    int rc = swscanf(clone.asCharArray(), L"%lld", &readNumber);

    if (rc == 1)
    {
        longLongNumber = readNumber;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtString::toUnsignedLongLongNumber
// Description: Converts a string that contains an unsigned long long number into
//              an unsigned long long.
// Arguments:   unsignedLongLongNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/2/2008
// ---------------------------------------------------------------------------
bool gtString::toUnsignedLongLongNumber(unsigned long long& unsignedLongLongNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Will get the read number:
    unsigned long long readNumber = 0;
    int rc1 = 0;

    // If the string is given in Hex (0x) format:
    int stringLength = this->length();

    if ((3 <= stringLength) && (_impl[0] == '0') && ((_impl[1] == 'x') || (_impl[1] == 'X')))
    {
        rc1 = swscanf(clone.asCharArray(), L"%llx", &readNumber);
    }
    else
    {
        // Convert the string into an unsigned int:
        rc1 = swscanf(clone.asCharArray(), L"%llu", &readNumber);
    }

    if (rc1 == 1)
    {
        unsignedLongLongNumber = readNumber;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::toUnsignedInt64Number
// Description: Converts a string that contains an unsigned long long number into
//              an unsigned Int64 number.
//              This function is similar to gtString::toUnsignedLongLongNumber but
//              is required because gcc does not have an implicit conversion of
//              gtUInt64 to unsigned long long :-(
// Arguments:   unsignedInt64Number - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        July-1, 2014
// ---------------------------------------------------------------------------
bool gtString::toUnsignedInt64Number(gtUInt64& unsignedInt64Number) const
{
    unsigned long long unsignedLongLongNumber = 0;
    bool retVal = gtString::toUnsignedLongLongNumber(unsignedLongLongNumber);

    if (retVal)
    {
        unsignedInt64Number = unsignedLongLongNumber;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::addThousandSeperators
// Description: Adds thousands separators to the string. Note this assumes a
//              string is a legal number eg 12345, 123456.78 or 1.234+e01.
// Author:      AMD Developer Tools Team
// Date:        14/9/2008
// ---------------------------------------------------------------------------
gtString& gtString::addThousandSeperators()
{
    gtString oldString = *this;
    gtString subString;

    makeEmpty();

    if (oldString[0] == '-')
    {
        append('-');
        oldString.truncate(1, -1);
    }

    int l = oldString.find('.');
    int ll = oldString.length();

    if (l == -1)
    {
        l = ll;
    }

    int k = l % GT_THOUSANDS_SEPARATOR_DISTANCE;

    if (k == 0)
    {
        k = GT_THOUSANDS_SEPARATOR_DISTANCE;
    }

    int j = 0;

    int i = k - 1;

    for (; i < l - GT_THOUSANDS_SEPARATOR_DISTANCE; i += GT_THOUSANDS_SEPARATOR_DISTANCE)
    {
        oldString.getSubString(j, i, subString);
        append(subString);
        append(GT_THOUSANDS_SEPARATOR);
        j = i + 1;
    }

    oldString.getSubString(j, i, subString);
    append(subString);
    j = i + 1;

    if (l < ll - 1)
    {
        oldString.getSubString(j, ll - 1, subString);
        append(subString);
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::fromMemorySize
// Description: Build a memory string
// Author:      AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
gtString& gtString::fromMemorySize(gtUInt64 memoryInSize)
{
    // Empty the string:
    makeEmpty();

    // Initialize memory size suffix:
    gtString suffix = GT_STR_BytesShort;
    gtUInt64 valueAsUInt64 = (gtUInt64)memoryInSize;

    if (memoryInSize > 1024)
    {
        float val = (float)memoryInSize / (float)1024;
        valueAsUInt64 = (size_t)ceil(val);
        suffix = GT_STR_KilobytesShort;

        if (valueAsUInt64 > 1024)
        {
            val = (float)valueAsUInt64 / (float)1024;
            valueAsUInt64 = (gtUInt64)ceil(val);
            suffix = GT_STR_MegabytesShort;
        }
    }

    // Build the string:
    appendFormattedString(L"%llu", valueAsUInt64);

    // Add ',''s to format the memory as number:
    addThousandSeperators();

    // Add space before the suffix:
    append(L" ");

    // Add the suffix to the memory string:
    append(suffix);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtString::removeChar
// Description: Removes all instances of the wchar c from the string
// Author:      AMD Developer Tools Team
// Date:        14/9/2008
// ---------------------------------------------------------------------------
gtString& gtString::removeChar(wchar_t c)
{
    gtString newString;
    gtString subString;
    int j = -1;
    int jj = -1;
    bool goOn = true;

    while (goOn)
    {
        jj = find(c, j + 1);

        if (jj == -1)
        {
            getSubString(j + 1, length() - 1, subString);
            newString.append(subString);
            goOn = false;
        }
        else if (jj == j + 1)
        {
            // do nothing this iteration
        }
        else
        {
            getSubString(j + 1, jj - 1, subString);
            newString.append(subString);
        }

        j = jj;
    }

    *this = newString;

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtString::isAlnum
// Description: Return true iff the string contain only alpha numeric characters
// Return Val:  bool - Success / failure.
// Arguments:   const gtString& validCharacterList - additional valid characters
// Author:      AMD Developer Tools Team
// Date:        4/5/2011
// ---------------------------------------------------------------------------
bool gtString::isAlnum(const gtString& validCharacterList)
{
    bool retVal = true;

    for (int i = 0; i < length(); i++)
    {
        // Get the current character:
        wchar_t ch = (*this)[i];

        // Check if this character is alpha numeric:
        bool isValidCharacter = isascii(ch);

        if (isValidCharacter)
        {
            isValidCharacter = (isalnum(ch) != 0);
        }

        if (!isValidCharacter)
        {
            // Check if the valid characters list contain this character:
            if (!validCharacterList.isEmpty())
            {
                isValidCharacter = (validCharacterList.find(ch) != -1);
            }

            if (!isValidCharacter)
            {
                retVal = false;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::isAlnum
// Description: Return true iff the string contain only alpha characters
// Return Val:  bool - Success / failure.
// Arguments:   const gtString& validCharacterList - additional valid characters
// Author:      AMD Developer Tools Team
// Date:        4/5/2011
// ---------------------------------------------------------------------------
bool gtString::isAlpha(const gtString& validCharacterList)
{
    bool retVal = true;

    for (int i = 0; i < length(); i++)
    {
        // Get the current character:
        wchar_t ch = (*this)[i];

        // Check if this character is alpha numeric:
        bool isValidCharacter = isascii(ch);

        if (isValidCharacter)
        {
            isValidCharacter = (isalpha(ch) != 0);
        }

        if (!isValidCharacter)
        {
            // Check if the valid characters list contain this character:
            if (!validCharacterList.isEmpty())
            {
                isValidCharacter = (validCharacterList.find(ch) != -1);
            }

            if (!isValidCharacter)
            {
                retVal = false;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtString::asUtf8
// Description: fill the argument string with a UTF8 encoded string representing the contents of this string
// Return Val:  0 if successful, error code otherwise
// Arguments:   utf8String - the string that will contain the UTF8 encoded representation of this string
// Author:      AMD Developer Tools Team
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int gtString::asUtf8(std::string& utf8String) const
{
    return gtWideStringToUtf8String(_impl, utf8String);
}

// ---------------------------------------------------------------------------
// Name:        gtString::reserve
// Description: Sets the capacity of the string to a number at least as great as a specified number.
// Return Val:  NA
// Arguments:   requestedLength - The number of characters for which memory is being reserved.
// Author:      AMD Developer Tools Team
// Date:        May-23, 2013
// ---------------------------------------------------------------------------
void gtString::reserve(size_t requestedLength)
{
    _impl.reserve(requestedLength);
}

// ---------------------------------------------------------------------------
// Name:        gtString::resize
// Description: Specifies a new size for a string, appending or erasing elements as required..
// Return Val:  NA
// Arguments:   newSize - The new size of the string.
// Author:      AMD Developer Tools Team
// Date:        May-23, 2013
// ---------------------------------------------------------------------------
void gtString::resize(size_t newSize)
{
    _impl.resize(newSize);
}


// ---------------------------------------------------------------------------
// Name:        operator==
// Description: Comparison operator.
// Return Val:  bool - true - iff the strings are equal.
// Author:      AMD Developer Tools Team
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool operator==(const gtString& str1, const gtString& str2)
{
    return operator==(str1._impl, str2._impl);
}


// ---------------------------------------------------------------------------
// Name:        operator==
// Description: Comparison operator.
// Return Val:  bool - true - iff the strings are equal.
// Author:      AMD Developer Tools Team
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool operator==(const gtString& str1, const wchar_t* pString)
{
    return operator==(str1._impl, pString);
}


// ---------------------------------------------------------------------------
// Name:        operator==
// Description: Comparison operator.
// Return Val:  bool - true - iff the strings are equal.
// Author:      AMD Developer Tools Team
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool operator==(const wchar_t* pString, const gtString& str2)
{
    return operator==(pString, str2._impl);
}


// ---------------------------------------------------------------------------
// Name:        operator==
// Description: "Not" Comparison operator.
// Return Val:  bool - true - iff the strings are NOT equal.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool operator!=(const gtString& str1, const gtString& str2)
{
    return !(operator==(str1, str2));
}


// ---------------------------------------------------------------------------
// Name:        operator==
// Description: "Not" Comparison operator.
// Return Val:  bool - true - iff the strings are NOT equal.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool operator!=(const gtString& str1, const wchar_t* pString)
{
    return !(operator==(str1, pString));
}


// ---------------------------------------------------------------------------
// Name:        operator==
// Description: "Not" Comparison operator.
// Return Val:  bool - true - iff the strings are NOT equal.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool operator!=(const wchar_t* pString, const gtString& str2)
{
    return !(operator==(pString, str2));
}


// ---------------------------------------------------------------------------
// Name:        gtIsDigit
// Description: Inputs a wchar and returns true iff it represents a digit.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtIsDigit(wchar_t c)
{
    bool retVal = false;

    // If the wchar is a digit:
    if (('0' <= c) && (c <= '9'))
    {
        retVal = true;
    }

    return retVal;
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// ---------------------------------------------------------------------------
// Name:        gtANSIStringToUnicodeString
// Description: Converts a given ANSI string to a Unicode string.
// Arguments: pANSIString - The input ANSI string.
//            pUnicodeStringBuff - A buffer that will receive the output Unicode string.
//            UnicodeStringBuffSize - The unicode string buffer size (measured in unicode
//                                    characters).
// TO_DO: Unicode: change to boolean return value
// Return Val: 0 for success, otherwise the code error
// Author:      AMD Developer Tools Team
// Date:        26/11/2005
// ---------------------------------------------------------------------------
size_t gtASCIIStringToUnicodeString(const char* pANSIString, wchar_t* pUnicodeStringBuff, size_t UnicodeStringBuffSize)
{
    // Perform the Unicode -> ANSI string conversion:
    size_t amountOfCharsConverted = 0;
    size_t retVal = mbstowcs_s(&amountOfCharsConverted, pUnicodeStringBuff, UnicodeStringBuffSize, pANSIString, _TRUNCATE);

    // Replace the error code that the Windows implementation returns when truncation occurs with a zero.
    // Truncation is a normal product of this function and does not require an error code to be returned.
    if (STRUNCATE == retVal)
    {
        retVal = 0;
    }

    GT_ASSERT(retVal == 0);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtUnicodeStringToANSIString
// Description:
//   Converts a given Unicode string to an ANSI string.
//
// Arguments: pUnicodeString - The input Unicode string.
//            pANSIStringBuff - A buffer that will receive the ANSI string.
//            ANSIStringBuffSize - The ANSI string buffer size.
//
// TO_DO: Unicode: change to boolean return value
// Return Val: 0 for success, otherwise the code error
// Author:      AMD Developer Tools Team
// Date:        26/11/2005
// ---------------------------------------------------------------------------
size_t gtUnicodeStringToASCIIString(const wchar_t* pUnicodeString, char* pANSIStringBuff, size_t ANSIStringBuffSize)
{
    // Perform the Unicode -> ANSI string conversion:
    size_t amountOfCharsConverted = 0;
    size_t retVal = wcstombs_s(&amountOfCharsConverted, pANSIStringBuff, ANSIStringBuffSize, pUnicodeString, _TRUNCATE);

    GT_ASSERT(retVal == 0);
    return retVal;
}

#else
// ---------------------------------------------------------------------------
// Name:        gtANSIStringToUnicodeString
// Description: Linux version Converts a given ANSI string to a Unicode string.
// Arguments: pANSIString - The input ANSI string.
//            pUnicodeStringBuff - A buffer that will receive the output Unicode string.
//            UnicodeStringBuffSize - The unicode string buffer size (measured in unicode
//                                    characters).
// TO_DO: Unicode: change to boolean return value
// Return Val: 0 for success, otherwise the code error
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------
size_t gtASCIIStringToUnicodeString(const char* pANSIString, wchar_t* pUnicodeStringBuff, size_t UnicodeStringBuffSize)
{
    int amountOfCharsConverted = mbstowcs(pUnicodeStringBuff, pANSIString, UnicodeStringBuffSize);
    GT_ASSERT(amountOfCharsConverted != -1);

    int retVal = (amountOfCharsConverted == -1);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtUnicodeStringToANSIString
// Description: Linux version
//   Converts a given Unicode string to an ANSI string.
//
// Arguments: pUnicodeString - The input Unicode string.
//            pANSIStringBuff - A buffer that will receive the ANSI string.
//            ANSIStringBuffSize - The ANSI string buffer size.
//
// TO_DO: Unicode: change to boolean return value
// Return Val: 0 for success, otherwise the code error
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------
size_t gtUnicodeStringToASCIIString(const wchar_t* pUnicodeString, char* pANSIStringBuff, size_t ANSIStringBuffSize)
{
    int amountOfCharsConverted = wcstombs(pANSIStringBuff, pUnicodeString, ANSIStringBuffSize);
    GT_ASSERT(amountOfCharsConverted != -1);

    int retVal = (amountOfCharsConverted == -1);

    return retVal;
}

#endif


// TO_DO: Unicode implement me
// ---------------------------------------------------------------------------
// Name:        gtStrTok
// Description: Implement a unicode version for strtok
// Arguments:   wchar_t* str
//              const wchar_t* delim
// Return Val:  wchar_t*
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
wchar_t* gtStrTok(wchar_t* str, const wchar_t* delim)
{
    (void)(str); // unused
    (void)(delim); // unused
    GT_ASSERT(false);
    return nullptr;
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// ---------------------------------------------------------------------------
// Name:        gtWideCharToUtf8
// Description: Convert a string expressed as an array of wide chars to an array of UTF8 encoded single chars
// Arguments:   const wstring& org - [INPUT] the original wide char string
//              std::string &dst - [OUTPUT] the UTF8 encoded multibyte string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      AMD Developer Tools Team
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int gtWideStringToUtf8String(const std::wstring& org, std::string& dst)
{
    unsigned int error = 0;
    dst.clear();

    if (org.size() == 0)
    {
        return 0;
    }

    // Step #1: get the number of characters length of the Get length (in chars) of resulting UTF8 string
    //
    const int orgLength = (int)org.length();
    const int dstLength = ::WideCharToMultiByte(CP_UTF8,        // Code Page = UTF8
                                                0,              // Use the default flags
                                                org.data(),     // Source wide string
                                                orgLength,      // Length of source string
                                                nullptr,           // Not required as we're only calculating the output length
                                                0,              // Set to zero to force calculation without actual conversion
                                                nullptr, nullptr      // unused when only doing calculation
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
    int rc = ::WideCharToMultiByte(CP_UTF8,          // Code Page = UTF8
                                   0,                // Use the default flags
                                   org.data(),       // Source wide string
                                   orgLength,        // Length of source string
                                   &dst[0],          // Not required as we're only calculating the output length
                                   dstLength,        // Address of the output buffer
                                   nullptr, nullptr        // Length of the output buffer
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
// Name:        gtUtf8StringToWideString
// Description: Convert a string expressed as an array of UTF8 encoded single chars to an array of wide chars
// Arguments:   const wstring& org - [INPUT] the original UTF8 encoded multibyte string
//              std::string &dst - [OUTPUT] the wide char string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      AMD Developer Tools Team
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int gtUtf8StringToWideString(const std::string& org, std::wstring& dst)
{
    unsigned int error = 0;
    dst.clear();

    if (org.size() == 0)
    {
        return 0;
    }

    // Step #1: get the number of characters length of the Get length (in chars) of resulting UTF8 string
    //
    const int orgLength = (int)org.length();
    const int dstLength = ::MultiByteToWideChar(CP_UTF8,        // Code Page = UTF8
                                                0,              // Use the default flags
                                                org.data(),     // Source string
                                                orgLength,      // Length of source string
                                                nullptr,           // Not required as we're only calculating the output length
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
    int rc = ::MultiByteToWideChar(CP_UTF8,          // Code Page = UTF8
                                   0,                // Use the default flags
                                   org.data(),       // Source wide string
                                   orgLength,        // WideCharToMultiByte will deduce the string length based on nullptr termination
                                   &dst[0],          // Address of the output buffer
                                   dstLength         // Length of the output buffer
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
//              std::string &dst - [OUTPUT] the UTF8 encoded multibyte string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      AMD Developer Tools Team
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int gtWideStringToUtf8String(const std::wstring& org, std::string& dst)
{
    int retVal = 0;
    dst.clear();

    try
    {
        utf8::utf32to8(org.begin(), org.end(), std::back_inserter(dst));
    }
    catch (...)
    {
        retVal = -1;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtUtf8StringToWideString
// Description: Linux version: Convert a string expressed as an array of UTF8 encoded single chars to an array of wide chars
// Arguments:   const wstring& org - [INPUT] the original UTF8 encoded multibyte string
//              std::string &dst - [OUTPUT] the wide char string
// Return Val:  0 if successful, otherwise an error code is returned
// Author:      AMD Developer Tools Team
// Date:        Oct-21, 2012
// ---------------------------------------------------------------------------
int gtUtf8StringToWideString(const std::string& org, std::wstring& dst)
{
    int retVal = 0;
    dst.clear();

    try
    {
        utf8::utf8to32(org.begin(), org.end(), std::back_inserter(dst));
    }
    catch (...)
    {
        retVal = -1;
    }

    return retVal;
}

#endif // #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


// -------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept nullptr-terminated
//              character strings as an input.
// Arguments:   gtString& target      - the target stream
//              const gtString& input - the gtString that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// -------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, const char* input)
{
    gtString tmp;
    tmp.fromASCIIString(input);
    target.append(tmp);
    return target;
}

// -------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept nullptr-terminated wide
//              character strings as an input.
// Arguments:   gtString& target      - the target stream
//              const wchar_t* input - the wchar_t* that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// -------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, const wchar_t* input)
{
    target.append(input);
    return target;
}

// -------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept another gtString as
//              an input.
// Arguments:   gtString& target      - the target stream
//              const gtString& input - the gtString that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// -------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, const gtString& other)
{
    target.append(other);
    return target;
}

// ----------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept integer values as an input
// Arguments:   gtString& target - the target stream
//              int input        - the gtString that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// ----------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, int input)
{
    target.appendFormattedString(L"%d", input);
    return target;
}

// -------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept unsigned integer values
//              as an input
// Arguments:   gtString& target   - the target stream
//              unsigned int input - the unsigned int that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// -------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, unsigned int input)
{
    target.appendFormattedString(L"%u", input);
    return target;
}

// -------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept long values as an input
// Arguments:   gtString& target - the target stream
//              long input       - the long value that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// -------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, long input)
{
    target.appendFormattedString(L"%ld", input);
    return target;
}

// ---------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept unsigned long values
//              as an input
// Arguments:   gtString& target    - the target stream
//              unsigned long input - the unsigned long that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// ---------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, unsigned long input)
{
    target.appendFormattedString(L"%lu", input);
    return target;
}

// --------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept float values as as input
// Arguments:   gtString& target - the target stream
//              float input      - the float value that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// --------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, float input)
{
    target.appendFormattedString(L"%f", input);
    return target;
}

// ----------------------------------------------------------------------------------------------
// Name:        operator<<
// Description: Friend function of gtString, allows gtString to accept double values as an input
// Arguments:   gtString& target - the target stream
//              double input     - the double value that should get into the target gtString
// Return Val:  the gtString (by reference)
// Author:      AMD Developer Tools Team
// Date:        Feb-10, 2014
// ----------------------------------------------------------------------------------------------
gtString& operator<<(gtString& target, double input)
{
    target.appendFormattedString(L"%f", input);
    return target;
}

