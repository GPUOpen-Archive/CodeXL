//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JvmsParser.h
///
//==================================================================================

#ifndef _JVMSPARSER_H_
#define _JVMSPARSER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Java Class name format: L<java/class/name>;
// Parser to:
//    - Skip 'L' and ';' in the signature
//    - Replace '/' with '.'
bool parseClassSignature(const char* jniStr, char* outStr)
{
    bool rc = false;
    // index to jniStr[]
    int inIndex = 0;
    // index to outStr[]
    int outIndex = 0;

    if (outStr != nullptr)
    {
        if (jniStr != nullptr)
        {
            while (jniStr[inIndex] != '\0' && outIndex < (OS_MAX_PATH - 1))
            {
                if (jniStr[inIndex] != 'L' && jniStr[inIndex] != ';')
                {
                    outStr[outIndex++] = (jniStr[inIndex] == '/') ? '.' : jniStr[inIndex];
                }

                ++inIndex;
            }

            if (outIndex > 0)
            {
                rc = true;
            }
        }

        outStr[outIndex] = '\0';
    }

    return rc;
}

// Parser to convert Java method descriptors into readable format.
// {Java Virtual Machine Specification, Chapter 4, Method Descriptors}
// Usually Recursive Descent Predictive Parser is suitable for such parsing,
// but as we just need to parse the parameter descriptors, linear parser is used here.
// Only parse the method parameters, skip the return types.
// Ex: converts (D[I[Ljava/lang/Object;) to (double, int[], java.lang.Object[])
// Note: outStr maximum length assumed to be OS_MAX_PATH
bool parseMethodSignature(const char* nameStr, const char* jniStr, char* outStr)
{
    bool rc = false;
    // index to jniStr[]
    int i = 0;
    // index to outStr[]
    int m = 0;
    // counts consecutive '['
    int countArray = 0;

    // Lambda to parse object types = L<class-name>;
    auto parseObjectType = [&]()
    {
        if (jniStr[i] == 'L')
        {
            // skip 'L'
            ++i;

            while (jniStr[i] != '\0' && jniStr[i] != ';' && m < (OS_MAX_PATH - 1))
            {
                outStr[m++] = (jniStr[i] == '/') ? '.' : jniStr[i];
                i++;
            }
        }

        if (jniStr[i] == ';')
        {
            // skip ';'
            ++i;
        }
    };

    // Lambda to parse base types
    auto appendBaseType = [&](char t)
    {
        int idx = 0;
        std::string typeStr;

        switch (t)
        {
            case 'B':
                typeStr = "byte";
                break;

            case 'C':
                typeStr = "char";
                break;

            case 'D':
                typeStr = "double";
                break;

            case 'F':
                typeStr = "float";
                break;

            case 'I':
                typeStr = "int";
                break;

            case 'J':
                typeStr = "long";
                break;

            case 'S':
                typeStr = "short";
                break;

            case 'V':
                typeStr = "void";
                break;

            case 'Z':
                typeStr = "boolean";
                break;

            default:
                // Use unknown type as it is
                typeStr = t;
                break;
        }

        while (typeStr[idx] != '\0' && m < (OS_MAX_PATH - 1))
        {
            outStr[m++] = typeStr[idx++];
        }
    };

    auto appendChar = [&](char ch)
    {
        if (m < (OS_MAX_PATH - 1))
        {
            outStr[m++] = ch;
        }
    };

    // Append parameter separator
    auto appendCommaSep = [&]()
    {
        if (jniStr[i] != ')')
        {
            if (m < (OS_MAX_PATH - 1))
            {
                outStr[m++] = ',';
            }
        }
    };

    auto appendSqrBkts = [&]()
    {
        while (countArray > 0)
        {
            if (m < (OS_MAX_PATH - 2))
            {
                outStr[m++] = '[';
                outStr[m++] = ']';
            }

            --countArray;
        }
    };

    if (outStr != nullptr)
    {
        // Append method name as it is to outStr
        if (nameStr != nullptr)
        {
            int j = 0;

            while (nameStr[j] != '\0' && m < (OS_MAX_PATH - 1))
            {
                outStr[m++] = nameStr[j++];
            }
        }

        // parse method parameter descriptor
        if (jniStr != nullptr)
        {
            bool isFinished = false;

            while (jniStr[i] != '\0' && !isFinished)
            {
                switch (jniStr[i])
                {
                    case 'L':
                        parseObjectType();
                        appendSqrBkts();
                        appendCommaSep();
                        break;

                    case '[':
                        ++countArray;
                        ++i;
                        break;

                    case '(':
                        appendChar('(');
                        ++i;
                        break;

                    case ')':
                        appendChar(')');
                        isFinished = true;
                        break;

                    default:
                        appendBaseType(jniStr[i++]);
                        appendSqrBkts();
                        appendCommaSep();
                        break;
                }
            }

            if (m > 0)
            {
                rc = true;
            }
        }

        outStr[m] = '\0';
    }

    return rc;
}
#endif // _JVMSPARSER_H_