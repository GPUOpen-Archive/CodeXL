//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtStringTokenizer.cpp
///
//=====================================================================

//------------------------------ gtStringTokenizer.cpp ------------------------------

// Standard C:
#include <string.h>

// Local:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>



// ---------------------------------------------------------------------------
// Name:        gtStringTokenizer::gtStringTokenizer
// Description: Constructor
// Arguments:   string - The string to be "tokenized" (to break into tokens).
//              delimiters - The tokens delimiters packed as a string.
//                           (Each delimiter is a single char)
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
gtStringTokenizer::gtStringTokenizer(const gtString& str, const gtString& delimiters)
    : _pString(NULL), _pStringLastChar(NULL), _pCurrentPosition(NULL),
      _delimitersString(delimiters), _firstNextTokenCall(true)
{
    // Get the input string size:
    int strSize = str.length();

    if (strSize > 0)
    {
        // Allocate a member string:
        _pString = new wchar_t[strSize + 1];
        GT_ASSERT(_pString);

        if (_pString)
        {
            // Copy the input string into it:
            wcscpy(_pString, str.asCharArray());

            // Initialize the current position:
            _pCurrentPosition = _pString;

            // Initialize the last wchar position:
            _pStringLastChar = _pString + strSize - 1;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gtStringTokenizer::~gtStringTokenizer
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
gtStringTokenizer::~gtStringTokenizer()
{
    delete[] _pString;
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// ---------------------------------------------------------------------------
// Name:        gtStringTokenizer::getNextToken
// Description: Returns the next token.
// Arguments:   token - Will get the next token.
// Return Val:  bool - true - the next token was found.
//                     false - the end of the input string was reached.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
bool gtStringTokenizer::getNextToken(gtString& token)
{
    bool retVal = false;
    token.makeEmpty();

    if (_pCurrentPosition != NULL)
    {
        // Look for the next token:
        wchar_t* pTokenInStr = wcstok(_pCurrentPosition, _delimitersString.asCharArray());

        if (pTokenInStr)
        {
            size_t tokenStartOffset = (pTokenInStr - _pCurrentPosition);

            // Fill the token:
            token = pTokenInStr;

            // Verify that we got a token:
            int tokenSize = token.length();

            if (tokenSize > 0)
            {
                // Progress the current string position:
                _pCurrentPosition += tokenStartOffset + tokenSize + 1;

                // Did we reach the end of the string:
                if (_pCurrentPosition > _pStringLastChar)
                {
                    _pCurrentPosition = NULL;
                }

                retVal = true;
            }
        }
    }

    return retVal;
}

#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
// ---------------------------------------------------------------------------
// Name:        gtStringTokenizer::getNextToken
// Description: Returns the next token.
// Arguments:   token - Will get the next token.
// Return Val:  bool - true - the next token was found.
//                     false - the end of the input string was reached.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
bool gtStringTokenizer::getNextToken(gtString& token)
{
    bool retVal = false;
    token.makeEmpty();

    if (_pCurrentPosition != NULL)
    {
        // Look for the next token:
        wchar_t* pTokenInStr = NULL;

        if (_firstNextTokenCall)
        {
            pTokenInStr = wcstok(_pString, _delimitersString.asCharArray(), &_pCurrentPosition);
        }
        else
        {
            pTokenInStr = wcstok(NULL, _delimitersString.asCharArray(), &_pCurrentPosition);
        }

        _firstNextTokenCall = false;

        if (pTokenInStr)
        {
            token = pTokenInStr;

            retVal = true;
        }
    }


    return retVal;
}
#else
#error Unknown build target!
#endif

