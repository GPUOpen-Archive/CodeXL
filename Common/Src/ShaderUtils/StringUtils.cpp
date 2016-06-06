//=====================================================================
// Copyright 2007-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StringUtils.cpp
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/StringUtils.cpp#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#include <cstring>
#include "StringUtils.h"

size_t StringUtils::SplitStringIntoLines(const std::string& strString, std::vector<std::string>& vLines, bool bTrimLines)
{
    size_t nLineBegin = 0;
    size_t nLineEnd = 0;

    do
    {
        nLineEnd = strString.find('\n', nLineBegin);
        std::string strLine(strString, nLineBegin, nLineEnd - nLineBegin);

        if (bTrimLines)
        {
            TrimString(strLine);
        }

        vLines.push_back(strLine);
        nLineBegin = nLineEnd + 1;
    }
    while (nLineEnd != std::string::npos);

    return vLines.size();
}

void StringUtils::TrimString(std::string& strString)
{
    const char* pszWhiteSpace = " \t\r\n";

    size_t nStartPos = strString.find_first_not_of(pszWhiteSpace);

    if ((nStartPos == std::string::npos))
    {
        strString = "";
    }
    else
    {
        size_t nEndPos = strString.find_last_not_of(pszWhiteSpace);
        strString = strString.substr(nStartPos, nEndPos + 1);
    }
}

size_t StringUtils::FindFunctionEntryInString(const std::string& strFunctionEntry, const std::string& strSource)
{
    std::vector<std::string> lines;
    size_t nLines = StringUtils::SplitStringIntoLines(strSource, lines);
    int scopeLevel = 0;

    // The algorithm to find the function definition goes as follows:
    // 1. Scan the source line by line to find the function name.
    // 2. If the function name is found, this line contains the function definition only if
    //    the scope level is equal to 0 (at the top level).
    // 3. Use the open and closing brackets to compute the scope level.
    // 4. Skip C and C++ comments as necessary.
    // Note we don't take account for preprocessing yet.

    bool bCommentLine = false;

    for (size_t i = 0; i < nLines; ++i)
    {
        std::string line(lines[ i ]);

        //-------------------- HANDLING C comment blocks -----------------------------
        size_t foundEndComment = line.find("*/");

        if (bCommentLine && std::string::npos != foundEndComment)
        {
            bCommentLine = false;
            line.erase(0, foundEndComment + 2);    // chop out the string until the */ tag
        }

        if (bCommentLine)
        {
            // this entire line is part of the C comment line, skip it
            continue;
        }

        // let's chop out the substring in between all C comment tags(/* and */)
        bool bLoop = true;

        while (bLoop)
        {
            bLoop = false;
            size_t foundStartComment = line.find("/*");

            if (std::string::npos != foundStartComment)
            {
                foundEndComment = line.find("*/", foundStartComment + 2);

                if (std::string::npos != foundEndComment)
                {
                    // chop out the string from /* till */ tags
                    line.erase(foundStartComment, foundEndComment - foundStartComment + 2);
                    bLoop = true;
                }
                else
                {
                    bCommentLine = true;
                    // chop out the string from the /* tag till the end of the line
                    line.erase(foundStartComment);
                }
            }
        }

        //--------------- END OF HANDLING C comment blocks ---------------------------

        size_t foundComment = line.find("//");

        if (std::string::npos != foundComment)
        {
            // chop out the C++ comment block
            line.erase(foundComment);
        }

        if (line.empty())
        {
            continue;
        }

        size_t startPos = 0;
        size_t foundFunction = std::string::npos;

        // loop over the line once for each time the func name appears.
        // here's an exmaple of a problematic line: "HS_CONTROL_POINT_OUTPUT HS( InputPatch<VS_OUTPUT_HS_INPUT, 3> inputPatch,"
        while ((foundFunction = line.find(strFunctionEntry, startPos)) != std::string::npos)
        {
            // the func name was found, but it may be a substring of a different string
            // need to make sure the character before and after are expected                            // preceeding character:
            if (((foundFunction == 0) ||                                                                // func name is the first word on the line OR
                 (foundFunction > 0 && line.at(foundFunction - 1) == ' ')) &&                         // preceding character is a space
                // following character:
                ((line.length() == foundFunction + strFunctionEntry.length()) ||                       // function name ends at end of line OR
                 (strchr(" (", line.at(foundFunction + strFunctionEntry.length())) != NULL)))         // char after func name is space or '('
            {
                // make sure we calculate the scope level prior to the function name at the same line

                size_t foundOpenBracket = line.find_first_of("{", startPos);

                // count the open bracket prior to the function name and update the scope level
                while (std::string::npos != foundOpenBracket &&
                       foundOpenBracket < foundFunction)
                {
                    ++scopeLevel;
                    foundOpenBracket = line.find_first_of("{", foundOpenBracket + 1);
                }

                size_t foundCloseBracket = line.find_first_of("}", startPos);

                // count the closing bracket prior to the function name and update the scope level
                while (std::string::npos != foundCloseBracket &&
                       foundCloseBracket < foundFunction)
                {
                    --scopeLevel;
                    foundCloseBracket = line.find_first_of("}", foundCloseBracket + 1);
                }

                // the function name is at the top level so it must be the function definition and not a call to the function
                if (0 == scopeLevel)
                {
                    // this is the function definition, because it is at the top scope level
                    return (i + 1);
                }
            }

            // update the start position so we don't match the same funcname instance again
            startPos = foundFunction + 1;
        }

        size_t foundOpenBracket = line.find_first_of("{", startPos);

        // count the open bracket and update the scope level
        while (std::string::npos != foundOpenBracket)
        {
            ++scopeLevel;
            foundOpenBracket = line.find_first_of("{", foundOpenBracket + 1);
        }

        size_t foundCloseBracket = line.find_first_of("}", startPos);

        // count the closing bracket and update the scope level
        while (std::string::npos != foundCloseBracket)
        {
            --scopeLevel;
            foundCloseBracket = line.find_first_of("}", foundCloseBracket + 1);
        }
    }

    return 0;
}

