//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtASCIIStringTokenizer.cpp
///
//=====================================================================

//------------------------------ gtASCIIStringTokenizer.cpp ------------------------------

// Standard C:
#include <string.h>

// Local:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>



// ---------------------------------------------------------------------------
// Name:        gtASCIIStringTokenizer::gtASCIIStringTokenizer
// Description: Constructor
// Arguments:   string - The string to be "tokenized" (to break into tokens).
//              delimiters - The tokens delimiters packed as a string.
//                           (Each delimiter is a single char)
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
gtASCIIStringTokenizer::gtASCIIStringTokenizer(const gtASCIIString& str, const gtASCIIString& delimiters)
    : _pString(NULL), _pStringLastChar(NULL), _pCurrentPosition(NULL),
      _delimitersString(delimiters)
{
    // Get the input string size:
    int strSize = str.length();

    if (strSize > 0)
    {
        // Allocate a member string:
        _pString = new char[strSize + 1];
        GT_ASSERT(_pString);

        if (_pString)
        {
            // Copy the input string into it:
            strcpy(_pString, str.asCharArray());

            // Initialize the current position:
            _pCurrentPosition = _pString;

            // Initialize the last char position:
            _pStringLastChar = _pString + strSize - 1;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIStringTokenizer::~gtASCIIStringTokenizer
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
gtASCIIStringTokenizer::~gtASCIIStringTokenizer()
{
    delete[] _pString;
}


// ---------------------------------------------------------------------------
// Name:        gtASCIIStringTokenizer::getNextToken
// Description: Returns the next token.
// Arguments:   token - Will get the next token.
// Return Val:  bool - true - the next token was found.
//                     false - the end of the input string was reached.
// Author:      AMD Developer Tools Team
// Date:        22/8/2004
// ---------------------------------------------------------------------------
bool gtASCIIStringTokenizer::getNextToken(gtASCIIString& token)
{
    bool retVal = false;
    token.makeEmpty();

    if (_pCurrentPosition != NULL)
    {
        // Look for the next token:
        char* pTokenInStr = strtok(_pCurrentPosition, _delimitersString.asCharArray());

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


