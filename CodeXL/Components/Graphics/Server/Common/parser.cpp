//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support functions used to parse incoming messages frome the client.
//==============================================================================

#include "parser.h"

#include <map>
#include <vector>
#include "Logger.h"
#ifdef _LINUX
    #include "SafeCRT.h"
#endif

/// Test from number
bool IsNumber(char ch) { return (ch >= '0') && (ch <= '9'); }
/// Test for alpha
bool IsAlpha(char ch) { return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')); }

/// Skip space
/// \param sIn Input string
void SkipSpaces(char** sIn)
{
    while ((*(*sIn) != 0) && (*(*sIn) == ' '))
    {
        *sIn += 1;
    }
}

/// Expect
/// \param sIn Input string
/// \param c type
/// \return True or false
bool Expect(char** sIn, char c)
{
    SkipSpaces(sIn);

    if (**sIn == c)
    {
        *sIn += 1;
        return true;
    }

    return false;
}

/// Check for token
/// \param sIn Input string
/// \param sTok type
/// \return True or false
bool IsToken(char** sIn, const char* sTok)
{
    size_t dwTokLen = strlen(sTok);
    size_t nStringLength = strlen(*sIn);

    if (_strnicmp(*sIn, sTok, dwTokLen) == 0)
    {
        // Check to see if we are going to increment the pointer beond the end of the array.
        PsAssert(dwTokLen <= nStringLength);

        if (dwTokLen > nStringLength)
        {
            Log(logERROR, "IsToken: buffer overrun. Str = %s, Tok = %s\n", *sIn, sTok);
            return false;
        }

        *sIn += dwTokLen;
        return true;
    }

    return false;
}

/// Check for variable
/// \param sIn Input string
/// \param sTok type
/// \return True or false
bool IsVariable(char** sIn, const char* sTok)
{
    if (IsToken(sIn, sTok))
    {
        if (Expect(sIn , '='))
        {
            return true;
        }
    }

    return false;
}

/// Get bool variable
/// \param sIn Input string
/// \param bVal Output value
/// \return True or false
bool GetBool(char** sIn, bool* bVal)
{
    if (IsToken(sIn, "true"))
    {
        *bVal = true;
    }
    else if (IsToken(sIn, "false"))
    {
        *bVal = false;
    }
    else
    {
        return false;
    }

    return true;
}

/// Get dword variable
/// \param sIn Input string
/// \param pNum Output value
/// \return True or false
bool GetDWORD(char** sIn, unsigned long* pNum)
{
    unsigned long dwValue = 0;
    unsigned long dwLen = 0;

    while (IsNumber(**sIn))
    {
        dwValue = (dwValue * 10) + (**sIn - '0');
        *sIn += 1;
        dwLen++;
    }

    if (dwLen > 0)
    {
        *pNum = dwValue;
        return true;
    }

    return false;
}

/// Get long variable
/// \param sIn Input string
/// \param pNum Output value
/// \return True or false
bool GetLONG(char** sIn, long* pNum)
{
    unsigned long val = 0;

    bool res = GetDWORD(sIn, &val);

    *pNum = (long)val;

    return res;

}

/// Get float variable
/// \param sIn Input string
/// \param pfVal Output value
/// \return True or false
bool GetFloat(char** sIn, float* pfVal)
{

    unsigned long dwInteger = 0;
    unsigned long dwDecimal = 0;
    unsigned long deExponent = 0;

    int nScanRes = sscanf_s(*sIn, "%f", pfVal);

    if (nScanRes < 1)
    {
        Log(logERROR, "GetFloat: no float data scanned Str = %s\n", *sIn);
        return false;
    }

    // skip over the remainig values in the string

    Expect(sIn, '-');
    Expect(sIn, '+');

    if (GetDWORD(sIn, &dwInteger) == false)
    {
        // This is OK
    }

    if (Expect(sIn, '.'))
    {
        if (GetDWORD(sIn, &dwDecimal) == false)
        {
            return false;
        }
    }

    if (Expect(sIn, 'e') || Expect(sIn, 'E'))
    {
        Expect(sIn, '-');
        Expect(sIn, '+');

        if (GetDWORD(sIn, &deExponent) == false)
        {
            return false;
        }
    }

    return true;
}

/// Get long variable
/// \param sIn Input string
/// \param sTok Item to look for
/// \param pNum Output value
/// \return True or false
bool GetLONGVariable(char** sIn, const char* sTok, long* pNum)
{
    if (IsVariable(sIn, sTok))
    {
        if (GetLONG(sIn, pNum))
        {
            return true;
        }
    }

    return false;
}

/// Get dword variable
/// \param sIn Input string
/// \param sTok Item to look for
/// \param pNum Output value
/// \return True or false
bool GetDWORDVariable(char** sIn, const char* sTok, unsigned long* pNum)
{
    if (IsVariable(sIn, sTok))
    {
        if (GetDWORD(sIn, pNum))
        {
            return true;
        }
    }

    return false;
}

/// Get bool variable
/// \param sIn Input string
/// \param sTok Item to look for
/// \param pbVal Output value
/// \return True or false
bool GetBoolVariable(char** sIn, const char* sTok, bool* pbVal)
{
    if (IsVariable(sIn, sTok))
    {
        if (GetBool(sIn, pbVal))
        {
            return true;
        }
    }

    return false;
}

/// Get float variable
/// \param sIn Input string
/// \param sTok Item to look for
/// \param pfVal Output value
/// \return True or false
bool GetFloatVariable(char** sIn, const char* sTok, float* pfVal)
{
    if (IsVariable(sIn, sTok))
    {
        if (GetFloat(sIn, pfVal))
        {
            return true;
        }
    }

    return false;
}

/// Extract the 4 dwords from the input string
/// \param sIn Input string
/// \param p1 output dword
/// \param p2 output dword
/// \param p3 output dword
/// \param p4 output dword
/// \return True if success, false if fail to parse.
bool Parse4DWORDS(char** sIn, unsigned long* p1, unsigned long* p2, unsigned long* p3, unsigned long* p4)
{
    if (Expect(sIn, '('))
    {
        if (GetDWORD(sIn, p1))
        {
            if (Expect(sIn, ','))
            {
                if (GetDWORD(sIn, p2))
                {
                    if (Expect(sIn, ','))
                    {
                        if (GetDWORD(sIn, p3))
                        {
                            if (Expect(sIn, ','))
                            {
                                if (GetDWORD(sIn, p4))
                                {
                                    if (Expect(sIn, ')'))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------
/// This function checks if argument is constant. If it is it returns true, overwise false
/// \param sArgument Input string
/// \param cKey Key
/// \param nSlotID Slot id
/// \return true if success, false if fail.
//---------------------------------------------------------------------------------------------------------------
static bool GetConstantIDFromArgument(gtASCIIString sArgument, gtASCIIString cKey, unsigned int& nSlotID)
{
    int nKeyPosition = sArgument.find(cKey);

    // Check if cKey is not end of other argument
    if (nKeyPosition == gtASCIIString::npos || (nKeyPosition != 0
                                                && sArgument[nKeyPosition - 1] != '>' // code can be in HTML
                                                && sArgument[nKeyPosition - 1] != ' '))

    {
        return false;
    } // End of if

    // Looking for not digital symbols in argument
    int nNextNotDigit = (int)sArgument.find_first_not_of("0123456789", nKeyPosition + cKey.length());

    gtASCIIString sSlotID; // this variable contains string with the slot number

    if (nNextNotDigit == gtASCIIString::npos)   // All character after cKey are digits
    {
        sSlotID = sArgument.substr(nKeyPosition + cKey.length());
    } // End of if

    // Check if the variable is finished with digit
    else if ((sArgument[nNextNotDigit] != '<' && sArgument[nNextNotDigit] != ' ' && sArgument[nNextNotDigit] != ',' &&
              sArgument[nNextNotDigit] != '.' && sArgument[nNextNotDigit] != '['  && sArgument[nNextNotDigit] != '\n')
             || nNextNotDigit <= nKeyPosition + 1)
    {
        return false;
    } // End of else if
    else
    {
        sSlotID = sArgument.substr(nKeyPosition + cKey.length(), nNextNotDigit - nKeyPosition - cKey.length());
    } // End of else

    int nScanRes = sscanf_s(sSlotID.asCharArray(), "%d", &nSlotID);

    if (nScanRes < 1)
    {
        Log(logERROR, "%s: Failed to read integer from Str = %s\n", __FUNCTION__, sSlotID.asCharArray());
        return false;
    }

    return true;
}// End of GetConstantIDFromArgument

//---------------------------------------------------------------------------------------------------------------
/// This function returns of slots used in code
/// cKey is symbol which shows what kind of slots are requested: 'c' for float constants, 'b' for boolean
/// constants etc. Results are kept in usedSlots.
/// \param code Input code
/// \param cKey Key
/// \param usedSlots Output used slots list
/// \return Error code
//---------------------------------------------------------------------------------------------------------------
long GetConstantsFromCode(gtASCIIString code, gtASCIIString cKey, std::list<unsigned long>& usedSlots)
{
    long hr = 0;
    typedef std::map<  gtASCIIString, std::vector<unsigned long> > ArgumentType;
    typedef std::map<  gtASCIIString, std::vector<unsigned long> >::iterator ArgumentIter;

    ArgumentType ArgumentLength;
    ArgumentIter ArgIter;

    // This map contains infoemation about arguments lenth
    // i.e the third argument of the m4x4 itstruction is 4x4 matrix,
    // which uses 4 slots, so if shader has instruction m4x4 oPos, v0, c0 it means
    // c0, c1, c3 and c4 are used, that is why ArgumentLength["m4x4"][2] = 4

    ArgumentLength["m3x2"].resize(3);
    ArgumentLength["m3x2"][0] = 1;
    ArgumentLength["m3x2"][1] = 1;
    ArgumentLength["m3x2"][2] = 2;
    ArgumentLength["m3x3"].resize(3);
    ArgumentLength["m3x3"][0] = 1;
    ArgumentLength["m3x3"][1] = 1;
    ArgumentLength["m3x3"][2] = 3;
    ArgumentLength["m3x4"].resize(3);
    ArgumentLength["m3x4"][0] = 1;
    ArgumentLength["m3x4"][1] = 1;
    ArgumentLength["m3x4"][2] = 4;
    ArgumentLength["m4x3"].resize(3);
    ArgumentLength["m4x3"][0] = 1;
    ArgumentLength["m4x3"][1] = 1;
    ArgumentLength["m4x3"][2] = 3;
    ArgumentLength["m4x4"].resize(3);
    ArgumentLength["m4x4"][0] = 1;
    ArgumentLength["m4x4"][1] = 1;
    ArgumentLength["m4x4"][2] = 4;

    ArgumentLength["texm3x2depth"].resize(2);
    ArgumentLength["texm3x2depth"][0] = 3;
    ArgumentLength["texm3x2depth"][1] = 2;
    ArgumentLength["texm3x2pad"].resize(2);
    ArgumentLength["texm3x2pad"][0] = 3;
    ArgumentLength["texm3x2pad"][1] = 2;
    ArgumentLength["texm3x2tex"].resize(2);
    ArgumentLength["texm3x2tex"][0] = 3;
    ArgumentLength["texm3x2tex"][1] = 2;
    ArgumentLength["texm3x3"].resize(2);
    ArgumentLength["texm3x3"][0] = 3;
    ArgumentLength["texm3x3"][1] = 3;
    ArgumentLength["texm3x3pad"].resize(2);
    ArgumentLength["texm3x3pad"][0] = 3;
    ArgumentLength["texm3x3pad"][1] = 3;
    ArgumentLength["texm3x3spec"].resize(2);
    ArgumentLength["texm3x3spec"][0] = 3;
    ArgumentLength["texm3x3spec"][1] = 3;
    ArgumentLength["texm3x3tex"].resize(2);
    ArgumentLength["texm3x3tex"][0] = 3;
    ArgumentLength["texm3x3tex"][1] = 3;
    ArgumentLength["texm3x3vspec"].resize(2);
    ArgumentLength["texm3x3vspec"][0] = 3;
    ArgumentLength["texm3x3vspec"][1] = 3;


    int nStartOfToken = 0;

    // This loop splits code to tokens
    while (nStartOfToken != gtASCIIString::npos)
    {
        int nNextToken = code.find('\n', nStartOfToken + 1);

        // comment
        gtASCIIString sToken = (nNextToken == gtASCIIString::npos) ? code.substr(nStartOfToken) : code.substr(nStartOfToken, nNextToken - nStartOfToken);

        if (sToken.length() > 1 && sToken.substr(0, 2) != "//")      // Skip comments and empty strings
        {
            int nStartOfInstruction = (int)sToken.find_first_not_of(" \n", 0);      // comment
            int nEndOfInstruction = (int)sToken.find(' ', nStartOfInstruction);

            if (nEndOfInstruction == gtASCIIString::npos)
            {
                nStartOfToken = nNextToken;
                continue;
            }

            gtASCIIString sCommand = sToken.substr(nStartOfInstruction, nEndOfInstruction - nStartOfInstruction);

            unsigned int nArgument = 0;
            int nStartOfArgument = nEndOfInstruction + 1;
            int nEndOfArgument;

            do // This separates arguments of command
            {
                nEndOfArgument = sToken.find(',', nStartOfArgument);
                gtASCIIString sArgument = (nEndOfArgument == gtASCIIString::npos) ?
                                          sToken.substr(nStartOfArgument) :
                                          sToken.substr(nStartOfArgument, nEndOfArgument - nStartOfArgument);
                unsigned int nSlotID;

                if (GetConstantIDFromArgument(sArgument, cKey,  nSlotID) == false)
                {
                    nArgument++;
                    nStartOfArgument = nEndOfArgument + 1;
                    continue;
                }

                // Calculation of used constants. Default is 1. Next 2 lines check if the command in "special cases map"
                ArgIter = ArgumentLength.find(sCommand);

                int nArgsCount = (ArgIter == ArgumentLength.end()) ? 1 :   // If command has not been found the length of argument is supposed 1
                                 (ArgIter->second.size() > nArgument ? ArgIter->second[nArgument] :
                                  1);  // If there no information for considered argument its length is supposed 1

                // This loop adds variables in the used constants list
                for (unsigned int i = nSlotID; i < nSlotID + nArgsCount; i++)
                {
                    bool nNotFind = true;

                    for (std::list<unsigned long>::const_iterator iter = usedSlots.begin(); iter != usedSlots.end(); ++iter)
                    {
                        if (*iter == i)
                        {
                            nNotFind = false;
                            break;
                        }
                    }

                    if (nNotFind)
                    {
                        usedSlots.push_back(i);
                    } // End of if
                }// End of for

                nArgument++;
                nStartOfArgument = nEndOfArgument + 1;
            }
            while (nEndOfArgument != gtASCIIString::npos);
        } // End of if ( sToken.size() > 1 && sToken.substr( 0, 1 ) != "//" )

        nStartOfToken = nNextToken;
    }// End of while ( nFound  != string::npos )

    return hr;
}// End of GetConstantsFromCode



