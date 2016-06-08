//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtASCIIString.cpp
///
//=====================================================================

//------------------------------ gtASCIIString.cpp ------------------------------

// C:
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <algorithm>

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// General Strings:
#define GT_STR_KilobytesShort "KB"
#define GT_STR_MegabytesShort "MB"
#define GT_STR_BytesShort "bytes"

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::gtASCIIString
// Description: Constructor - Initializes an empty string
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString::gtASCIIString()
{
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::gtASCIIString
// Description: Copy constructor - Create me from another string
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString::gtASCIIString(const gtASCIIString& otherString)
    : _impl(otherString._impl)
{
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::gtASCIIString
// Description: Constructor - initialize self by copying a string from a char array.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString::gtASCIIString(const char* pOtherString)
    : _impl((pOtherString != NULL) ? pOtherString : "")
{
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::gtASCIIString
// Description: Constructor - Initialize self from a single char.
// Author:      AMD Developer Tools Team
// Date:        6/12/2004
// ---------------------------------------------------------------------------
gtASCIIString::gtASCIIString(char character)
{
    operator+=(character);
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::~gtASCIIString
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        15/11/2003
// ---------------------------------------------------------------------------
gtASCIIString::~gtASCIIString()
{
}


#if AMDT_HAS_CPP0X

gtASCIIString::gtASCIIString(gtASCIIString&& otherString) : _impl(std::move(otherString._impl))
{
}


gtASCIIString& gtASCIIString::operator=(gtASCIIString&& otherString)
{
    _impl.operator = (std::move(otherString._impl));
    return *this;
}

#endif // AMDT_HAS_CPP0X


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::asCharArray
// Description: Return this string as a null terminated char array.
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
const char* gtASCIIString::asCharArray() const
{
    return _impl.c_str();
};


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::length
// Description: Returns this string length (the amount of chars it holds)
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
int gtASCIIString::length() const
{
    return (int)_impl.length();
};

void gtASCIIString::resize(size_t newSize)
{
    _impl.resize(newSize);
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::isEmpty
// Description: Return true iff this string is empty.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
bool gtASCIIString::isEmpty() const
{
    return _impl.empty();
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::makeEmpty
// Description: Makes this string an empty string
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::makeEmpty()
{
    _impl.operator = ("");

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::append
// Description: Appends a single character to this string.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::append(char character)
{
    _impl += character;
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::append
// Description: Append another string into me.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::append(const char* pOtherString)
{
    if (pOtherString != NULL)
    {
        _impl.append(pOtherString);
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::append
// Description: Appends the first N characters of another string to me.
// Arguments:   pOtherString - The other string.
//              length - The appended string length (N).
// Author:      AMD Developer Tools Team
// Date:        2/2/2005
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::append(const char* pOtherString, int length)
{
    if (pOtherString != NULL)
    {
        _impl.append(pOtherString, length);
    }

    return *this;
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// ---------------------------------------------------------------------------
// Name:        gtASCIIString::append
// Description: Appends the first N characters of another string to me.
// Arguments:   pOtherString - The other string.
// Author:      AMD Developer Tools Team
// Date:        12/7/2009
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::append(const wchar_t* pOtherString)
{
    if (pOtherString != NULL)
    {
        // Get the wide char length:
        size_t wStrLength = wcslen(pOtherString);
        size_t buffSize = wStrLength + 1;

        // Allocate a string to copy to:
        char* pANSIString = new char[buffSize];


        // Copy the wide char string to me:
        gtUnicodeStringToASCIIString(pOtherString, pANSIString, buffSize);

        // Append the string to me:
        _impl.append(pANSIString, wStrLength);

        // Release the memory:
        delete[] pANSIString;
    }

    return (*this);
}

#endif


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::append
// Description: Append another string into me.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::append(const gtASCIIString& otherString)
{
    _impl.append(otherString._impl);
    return *this;
}



// ---------------------------------------------------------------------------
// Name:        gtASCIIString::appendFormattedString
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
//   gtASCIIString str;
//   str.appendFormattedString("Decimal: %d , Float:  %f", 7, 6.34);
//
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::appendFormattedString(const char* pFormatString, ...)
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
            char* pBuff = new char[buffSize]();


            // Write the formatted string into the buffer:
            int size = vsnprintf_s(pBuff, buffSize - 1, _TRUNCATE, pFormatString, argptr);

            // The buffer was big enough to contains the formatted string:
            if (0 <= size)
            {
                // Terminate the string manually:
                // (Some implementations of vsnprintf() don't NULL terminate the string if
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

        // Get another pointer to the variable list of arguments:
        va_list argptr2;
        va_start(argptr2, pFormatString);

        // Calculate the formatted string size:
        int formattedStringSize = vsnprintf(NULL, 0, pFormatString, argptr2);
        GT_IF_WITH_ASSERT(0 < formattedStringSize)
        {
            // Allocate buffer that will contain the formatted string:
            int buffSize = formattedStringSize + 1;
            char* pBuff = new char[buffSize]();


            // Restart the arguments pointer:
            va_end(argptr);
            va_start(argptr, pFormatString);

            // Write the formatted string into the buffer:
            int charsWritten = vsnprintf(pBuff, buffSize, pFormatString, argptr);
            GT_IF_WITH_ASSERT(0 <= charsWritten)
            {
                // NULL terminate the formatted string:
                pBuff[charsWritten] = 0;

                // Append the formatted string onto me:
                append(pBuff);
            }

            // Release allocated memory:
            delete[] pBuff;
        }

        // Terminate the second copy of the argptr pointer:
        va_end(argptr2);

    }
#endif


    // Terminate the argptr pointer:
    va_end(argptr);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::prepend
// Description: Prepends a single character to this string.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::prepend(char character)
{
    string temp = "";
    temp += character;
    temp.append(_impl);
    _impl = temp;
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::prepend
// Description: Prepend another string into me.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::prepend(const char* pOtherString)
{
    if (pOtherString != NULL)
    {
        string temp;
        temp.append(pOtherString);
        temp.append(_impl);
        _impl = temp;
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::prepend
// Description: Prepends the first N charachters of another string to me.
// Arguments:   pOtherString - The other string.
//              length - The appended string length (N).
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::prepend(const char* pOtherString, int length)
{
    if (pOtherString != NULL)
    {
        string temp;
        temp.append(pOtherString, length);
        temp.append(_impl);
        _impl = temp;
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::prepend
// Description: Prepend another string into me.
// Author:      AMD Developer Tools Team
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::prepend(const gtASCIIString& otherString)
{
    string temp;
    temp.append(otherString._impl);
    temp.append(_impl);
    _impl = temp;
    return *this;
}



// ---------------------------------------------------------------------------
// Name:        gtASCIIString::prependFormattedString
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
//   gtASCIIString str = ", Info:";
//   str.prependFormattedString("Decimal: %d , Float:  %f", 7, 6.34);
//      result: str == "Decimal: 7 , Float:  6.34, Info:"
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::prependFormattedString(const char* pFormatString, ...)
{
    // Get a pointer to the variable list of arguments:
    va_list argptr;
    va_start(argptr, pFormatString);

    int buffSize = 1024;
    bool goOn = true;

    string temp;

    while (goOn)
    {
        goOn = false;

        // Allocate a buffer that will contain the formatted string:
        char* pBuff = new char[buffSize];



        // Write the formatted string into the buffer:
        int size = vsnprintf(pBuff, buffSize, pFormatString, argptr);

        // The buffer was big enough to contains the formatted string:
        if (size > 0)
        {
            // Terminate the string manually:
            // (Some implementations of vsnprintf() don't NULL terminate the string if
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
// Name:        gtASCIIString::find
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
int gtASCIIString::find(const gtASCIIString& subString, int searchStartPosition) const
{
    int retVal = (int)_impl.find(subString._impl, searchStartPosition);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::find
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
int gtASCIIString::find(char character, int searchStartPosition) const
{
    int retVal = (int)_impl.find(character, searchStartPosition);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::findNextLine
// Description: finds the first new line (newline or carriage return character)
//              after searchStartPosition
// Arguments: searchStartPosition
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
int gtASCIIString::findNextLine(int searchStartPosition) const
{
    int nlPosition = find("\n", searchStartPosition);
    int crPosition = find("\r", searchStartPosition);
    int nextLineStart = -1;

    if ((crPosition != -1) && (nlPosition != -1))
    {
        nextLineStart = min(crPosition, nlPosition);
    }
    else
    {
        // if we didn't find one of the line enders, choose the other or leave the value as -1
        nextLineStart = max(nextLineStart, max(crPosition, nlPosition));
    }

    return nextLineStart;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::lineNumberFromCharacterIndex
// Description: returns the 0/1-based line number in which the character with the
//              given index resides, or -1 if the index is out of range.
// Author:      AMD Developer Tools Team
// Date:        19/10/2010
// ---------------------------------------------------------------------------
int gtASCIIString::lineNumberFromCharacterIndex(int characterIndex, bool oneBased) const
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
// Name:        gtASCIIString::reverseFind
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
int gtASCIIString::reverseFind(const gtASCIIString& subString, int searchStartPosition) const
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
// Name:        gtASCIIString::startsWith
// Description: Returns true iff this string starts with the input prefix string.
// Author:      AMD Developer Tools Team
// Date:        6/6/2005
// ---------------------------------------------------------------------------
bool gtASCIIString::startsWith(const gtASCIIString& prefixString) const
{
    bool retVal = false;

    // This string need to be at least as long as the prefix string:
    unsigned int prefixStringLength = prefixString.length();

    if (prefixStringLength <= _impl.length())
    {
        retVal = (strncmp(_impl.c_str(), prefixString.asCharArray(), prefixStringLength) == 0);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::onlyContainsCharacters
// Description: Returns true iff the string constitues only characters in validCharacterList.
// Author:      AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool gtASCIIString::onlyContainsCharacters(const gtASCIIString& validCharacterList) const
{
    bool retVal = true;

    // Iterate this string's characters:
    int strLen = length();

    for (int i = 0; i < strLen; i++)
    {
        // If the i-th character is not in validCharacterList:
        if (validCharacterList.find(operator[](i)) < 0)
        {
            // Fail the validation:
            retVal = false;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::reverseFind
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
int gtASCIIString::reverseFind(char character, int searchStartPosition) const
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
// Name:        gtASCIIString::count
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
int gtASCIIString::count(const gtASCIIString& subString, int countStartPosition) const
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
// Name:        gtASCIIString::count
// Description:  Counts how many times does character appear in the string.
// Arguments: character - the character to be searched
//            countStartPosition - where to start counting
// Return Val: int
// Author:      AMD Developer Tools Team
// Date:        30/3/2008
// ---------------------------------------------------------------------------
int gtASCIIString::count(char character, int countStartPosition) const
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
// Name:        gtASCIIString::operator=
// Description: Initialize this string from a single char.
// Author:      AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::operator=(char c)
{
    _impl.operator = (c);

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::operator=
// Description: Copy another string content into me.
// Return Val:  gtASCIIString - Return myself (This enables a user to write a = b = c;).
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::operator=(const char* pOtherString)
{
    if (pOtherString != NULL)
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
// Name:        gtASCIIString::operator=
// Description: Copy another string content into me.
// Return Val:  gtASCIIString - Return myself (This enables a user to write a = b = c;).
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::operator=(const gtASCIIString& otherString)
{
    _impl.operator = (otherString._impl);

    // Retruns a reference to myself:
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::operator[]
// Description: Returns the i'th char in this string.
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
const char& gtASCIIString::operator[](int i) const
{
    return _impl.operator[](i);
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::operator[]
// Description: Returns the i'th char in this string.
// Author:      AMD Developer Tools Team
// Date:        16/11/2003
// ---------------------------------------------------------------------------
char& gtASCIIString::operator[](int i)
{
    return _impl.operator[](i);
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::operator<
// Description: Returns true iff this string is smaller (lexicographic order)
//              than the other string.
// Return Val:  bool - true - Iff I am smaller than other.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::operator<(const gtASCIIString& otherString) const
{
    return _impl < otherString._impl;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::operator>
// Description: Returns true iff this string is bigger (lexicographic order)
//              than the other string.
// Return Val:  bool - true - Iff I am smaller than other.
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::operator>(const gtASCIIString& otherString) const
{
    return _impl > otherString._impl;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::compareNoCase
// Description: Compares this string to another string in lexicographic order.
//              The string character cases (upper or lower) are ignored.
// Return Val:  int - 0 - the strings are identical (ignoring case).
//                   <0 - This string is less than the other string (ignoring case).
//                   >0 - This string is greater than the other string (ignoring case).
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
int gtASCIIString::compareNoCase(const gtASCIIString& otherString) const
{
    int retVal = 0;

    // Get this string and the other string in upper case format:
    gtASCIIString thisUpper = *this;
    thisUpper.toUpperCase();

    gtASCIIString otherUpper = otherString;
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


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::removeTrailing
// Description:
//   Removes trailing char's of a given input char.
//   Example:
//   gtASCIIString foo = "abcdddd";
//   foo.removeTrailing('d');
//   Will yield "abc" string
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::removeTrailing(char c)
{
    if (_impl.length() > 0)
    {
        string::iterator startIter = _impl.begin();
        string::iterator endIter = _impl.end();

        // Look for the position of the last char that is not the input char:
        string::iterator iter = endIter;

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
// Name:        gtASCIIString::getSubString
// Description: Returns a sub string, beginning at startPosition and ending at
//              endPosition.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
void gtASCIIString::getSubString(int startPosition, int endPosition, gtASCIIString& subString) const
{
    string subStr = _impl.substr(startPosition, (endPosition - startPosition + 1));
    subString = subStr.c_str();
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::truncate
// Description: Sets this string to be a substring of itsself starting at
//              startPosition and ending at endPosition (inclusive)
// Author:      AMD Developer Tools Team
// Date:        27/8/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::truncate(int startPosition, int endPosition)
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
// Name:        gtASCIIString::replace
// Description: Replaces a occurrences of a substring with another one.
// Arguments: oldSubString - The sub string to be replaced.
//            replacementString - The sub string that will replace oldSubString.
//            replaceAllOccurrences - true will replace all occurrences of oldSubString.
//                                    false will just replace the first occurrence.
// Return Val: int - The amount of occurrences replaced.
// Author:      AMD Developer Tools Team
// Date:        28/11/2005
// ---------------------------------------------------------------------------
int gtASCIIString::replace(const gtASCIIString& oldSubString, const gtASCIIString& newSubString,
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
// Name:        gtASCIIString::replace
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
int gtASCIIString::replace(int startPos, int endPos, const gtASCIIString& oldSubString, const gtASCIIString& newSubString,
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
// Name:        gtASCIIString::toUpperCase
// Description: Converts a range of string characters to upper case.
// Arguments:   startPosition - The range start position.
//              endPosition - The range end position.
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::toUpperCase(int startPosition, int endPosition)
{
    if (endPosition == -1)
    {
        endPosition = this->length() - 1;
    }

    for (int i = startPosition; i <= endPosition; i++)
    {
        // Sanity check (char is signed):
        if (0 <= _impl[i])
        {
            // Make sure this is English:
            if (isascii(_impl[i]))
            {
                if (islower(_impl[i]))
                {
                    _impl[i] = (char)toupper(_impl[i]);
                }
            }
        }
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::toLowerCase
// Description: Converts a range of string characters to lower case.
// Arguments:   startPosition - The range start position.
//              endPosition - The range end position.
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::toLowerCase(int startPosition, int endPosition)
{
    if (endPosition == -1)
    {
        endPosition = this->length() - 1;
    }

    for (int i = startPosition; i <= endPosition; i++)
    {
        // Sanity check (char is signed):
        if (0 <= _impl[i])
        {
            // Make sure this is English:
            if (isascii(_impl[i]))
            {
                if (isupper(_impl[i]))
                {
                    _impl[i] = (char)tolower(_impl[i]);
                }
            }
        }
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::isIntegerNumber
// Description: Returns true iff this string contains an integer number.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::isIntegerNumber() const
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

        // If the first char is a digit:
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
// Name:        gtASCIIString::toIntNumber
// Description: Converts a string that contains an integer number into an int.
// Arguments:   integerNumber - Will get the output int.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::toIntNumber(int& intNumber) const
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
// Name:        gtASCIIString::toUnsignedIntNumber
// Description: Converts a string that contains an unsigned integer number into
//              an unsigned int.
// Arguments:   uintNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::toUnsignedIntNumber(unsigned int& uintNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtASCIIString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Will get the read number:
    unsigned int readNumber = 0;
    int rc1 = 0;

    // If the string is given in Hex (0x) format:
    int stringLength = this->length();

    if ((3 < stringLength) && (_impl[0] == '0') && ((_impl[1] == 'x') || (_impl[1] == 'X')))
    {
        rc1 = sscanf(clone.asCharArray(), "%x", &readNumber);
    }
    else
    {
        // Convert the string into an unsigned int:
        rc1 = sscanf(clone.asCharArray(), "%u", &readNumber);
    }

    if (rc1 == 1)
    {
        uintNumber = readNumber;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::toLongNumber
// Description: Converts a string that contains an integer number into a long.
// Arguments:   longNumber - Will get the output long.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::toLongNumber(long& longNumber) const
{
    bool retVal = false;

    // Verify that this string represents an integer number
    if (isIntegerNumber())
    {
        // Remove the thousands separators from the string:
        gtASCIIString clone = *this;
        clone.removeChar(GT_THOUSANDS_SEPARATOR);

        // Convert the string to an int:
        longNumber = atol(clone.asCharArray());

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::toUnsignedLongNumber
// Description: Converts a string that contains an unsigned long number into
//              an unsigned long.
// Arguments:   ulongNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtASCIIString::toUnsignedLongNumber(unsigned long& ulongNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtASCIIString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Will get the read number:
    unsigned long readNumber = 0;
    int rc1 = 0;

    // If the string is given in Hex (0x) format:
    int stringLength = this->length();

    if ((3 < stringLength) && (_impl[0] == '0') && ((_impl[1] == 'x') || (_impl[1] == 'X')))
    {
        rc1 = sscanf(clone.asCharArray(), "%lx", &readNumber);
    }
    else
    {
        // Convert the string into an unsigned int:
        rc1 = sscanf(clone.asCharArray(), "%lu", &readNumber);
    }

    if (rc1 == 1)
    {
        ulongNumber = readNumber;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::toLongLongNumber
// Description: Converts a string that contains a long long integer (64 bit integer)
//              number into a long long.
// Arguments:   longLongNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/2/2008
// ---------------------------------------------------------------------------
bool gtASCIIString::toLongLongNumber(long long& longLongNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtASCIIString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Convert the string into an unsigned int:
    long long readNumber = 0;
    int rc = sscanf(clone.asCharArray(), "%lld", &readNumber);

    if (rc == 1)
    {
        longLongNumber = readNumber;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIString::toUnsignedLongLongNumber
// Description: Converts a string that contains an unsigned long long number into
//              an unsigned long long.
// Arguments:   unsignedLongLongNumber - Will get the output number.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/2/2008
// ---------------------------------------------------------------------------
bool gtASCIIString::toUnsignedLongLongNumber(unsigned long long& unsignedLongLongNumber) const
{
    bool retVal = false;

    // Remove the thousands separators from the string:
    gtASCIIString clone = *this;
    clone.removeChar(GT_THOUSANDS_SEPARATOR);

    // Will get the read number:
    unsigned long long readNumber = 0;
    int rc1 = 0;

    // If the string is given in Hex (0x) format:
    int stringLength = this->length();

    if ((3 < stringLength) && (_impl[0] == '0') && ((_impl[1] == 'x') || (_impl[1] == 'X')))
    {
        rc1 = sscanf(clone.asCharArray(), "%llx", &readNumber);
    }
    else
    {
        // Convert the string into an unsigned int:
        rc1 = sscanf(clone.asCharArray(), "%llu", &readNumber);
    }

    if (rc1 == 1)
    {
        unsignedLongLongNumber = readNumber;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::addThousandSeperators
// Description: Adds thousands separators to the string. Note this assumes a
//              string is a legal number eg 12345, 123456.78 or 1.234+e01.
// Author:      AMD Developer Tools Team
// Date:        14/9/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::addThousandSeperators()
{
    gtASCIIString oldString = *this;
    gtASCIIString subString;

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
// Name:        gtASCIIString::fromMemorySize
// Description: Build a memory string
// Author:      AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::fromMemorySize(gtUInt64 memoryInSize)
{
    // Empty the string:
    makeEmpty();

    // Initialize memory size suffix:
    gtASCIIString suffix = GT_STR_BytesShort;
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
    appendFormattedString("%llu", valueAsUInt64);

    // Add ',''s to format the memory as number:
    addThousandSeperators();

    // Add space before the suffix:
    append(" ");

    // Add the suffix to the memory string:
    append(suffix);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        gtASCIIString::removeChar
// Description: Removes all instances of the char c from the string
// Author:      AMD Developer Tools Team
// Date:        14/9/2008
// ---------------------------------------------------------------------------
gtASCIIString& gtASCIIString::removeChar(char c)
{
    gtASCIIString newString;
    gtASCIIString subString;
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
// Name:        gtASCIIString::decodeHTML
// Description: Decodes HTML string to an ASCII
// Return Val:  void
// Author:      AMD Developer Tools Team
// Date:        23/6/2011
// ---------------------------------------------------------------------------
void gtASCIIString::decodeHTML()
{
    replace("&quot;", "\"");
    replace("&gt;", ">");
    replace("&lt;", "<");
    replace("&amp;", "&");
}

// ---------------------------------------------------------------------------
// Name:        operator==
// Description: Comparison operator.
// Return Val:  bool - true - iff the strings are equal.
// Author:      AMD Developer Tools Team
// Date:        14/5/2004
// ---------------------------------------------------------------------------
bool operator==(const gtASCIIString& str1, const gtASCIIString& str2)
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
bool operator==(const gtASCIIString& str1, const char* pString)
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
bool operator==(const char* pString, const gtASCIIString& str2)
{
    return operator==(pString, str2._impl);
}


// ---------------------------------------------------------------------------
// Name:        operator!=
// Description: "Not" Comparison operator.
// Return Val:  bool - true - iff the strings are NOT equal.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool operator!=(const gtASCIIString& str1, const gtASCIIString& str2)
{
    return !(operator==(str1, str2));
}


// ---------------------------------------------------------------------------
// Name:        operator!=
// Description: "Not" Comparison operator.
// Return Val:  bool - true - iff the strings are NOT equal.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool operator!=(const gtASCIIString& str1, const char* pString)
{
    return !(operator==(str1, pString));
}


// ---------------------------------------------------------------------------
// Name:        operator!=
// Description: "Not" Comparison operator.
// Return Val:  bool - true - iff the strings are NOT equal.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool operator!=(const char* pString, const gtASCIIString& str2)
{
    return !(operator==(pString, str2));
}


// ---------------------------------------------------------------------------
// Name:        gtIsDigit
// Description: Inputs a char and returns true iff it represents a digit.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool gtIsDigit(char c)
{
    bool retVal = false;

    // If the char is a digit:
    if (('0' <= c) && (c <= '9'))
    {
        retVal = true;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// Functionality needed for PerfStudio

// ---------------------------------------------------------------------------
// Name:        Split
// Description: Split the string up into a list using rSep as the delimiter
//              (separator)
// Return Val:  std::list of substrings
// Author:      AMD Developer Tools Team
// Date:        28/6/2013
// ---------------------------------------------------------------------------
void gtASCIIString::Split(const gtASCIIString& rSep, bool bCaseSensitive, list<gtASCIIString>& outList) const
{
    (void)(bCaseSensitive); // unused

    unsigned int nSepLen = rSep.length();
    int nPos = 0;
    int nPrevPos = 0;
    bool bDone = false;

    while (!bDone)
    {
        nPrevPos = nPos;
        nPos = find(rSep, nPos);

        if (nPos == -1)
        {
            // no more occurences of separator found, append last substring and return
            nPos = length();
            bDone = true;
        }

        // append substring to list
        gtASCIIString subStr;

        if (nPos > nPrevPos)
        {
            const char* startPtr = &_impl.c_str()[nPrevPos];
            subStr.append(startPtr, nPos - nPrevPos);
        }

        outList.push_back(subStr);
        nPos += nSepLen;          // skip past the separator string
    }
}

// ---------------------------------------------------------------------------
// Name:        find_first_not_of
// Description: Search for first character which is not contained in the string
//              'srcStr'. Duplicate functionality of std::string method of the
//              same name
// Return Val:  the index of the first found occurence of the input string or
//              nPos if none were found.
// Author:      AMD Developer Tools Team
// Date:        28/6/2013
// ---------------------------------------------------------------------------
size_t gtASCIIString::find_first_not_of(const char* srcStr, const size_t startIndex) const
{
    return _impl.find_first_not_of(srcStr, startIndex);
}

// ---------------------------------------------------------------------------
// Name:        find_last_of
// Description: Find last occurrence of src in this string. 'startIndex'
//              specifies the index where to begin search from, if that value
//              is -1, then the search starts from the end of the string
// Return Val:  Location of search result. -1 if not found
// Author:      AMD Developer Tools Team
// Date:        28/6/2013
size_t gtASCIIString::find_last_of(const char* src, const int startIndex) const
{
    return _impl.find_last_of(src, startIndex);
}

// ---------------------------------------------------------------------------
// Name:        substr
// Description: return substring starting at index startPosition with length
//              count. Duplicates functionality of std::string::substr()
// Return Val:  The substring of the original string
// Author:      AMD Developer Tools Team
// Date:        28/6/2013
// ---------------------------------------------------------------------------
gtASCIIString gtASCIIString::substr(int startPosition, int count) const
{
    return _impl.substr(startPosition, count).c_str();
}
