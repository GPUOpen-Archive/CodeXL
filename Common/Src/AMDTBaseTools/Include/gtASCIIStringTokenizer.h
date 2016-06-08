//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtASCIIStringTokenizer.h
///
//=====================================================================

//------------------------------ gtASCIIStringTokenizer.h ------------------------------

#ifndef __GTANSIISTRINGTOKENIZER
#define __GTANSIISTRINGTOKENIZER

// Local:
#include <AMDTBaseTools/Include/gtASCIIString.h>


// ----------------------------------------------------------------------------------
// Class Name:          gtASCIIStringTokenizer
// General Description: Same class as gtStringTokenizer, but working with ANSII strings
// Author:      AMD Developer Tools Team
// Creation Date:       13/9/2010
// ----------------------------------------------------------------------------------
class GT_API gtASCIIStringTokenizer
{
public:
    gtASCIIStringTokenizer(const gtASCIIString& str, const gtASCIIString& delimiters);
    ~gtASCIIStringTokenizer();

    bool getNextToken(gtASCIIString& token);

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    gtASCIIStringTokenizer() = delete;
    gtASCIIStringTokenizer(const gtASCIIStringTokenizer&) = delete;
    gtASCIIStringTokenizer& operator=(const gtASCIIStringTokenizer&) = delete;

    // The string to be "tokenized":
    char* _pString;

    // The string last char:
    char* _pStringLastChar;

    // The current position in the input string:
    char* _pCurrentPosition;

    // The tokens delimiters packed as a string:
    const gtASCIIString _delimitersString;
};


#endif  // __GTSTRINGTOKENIZER
