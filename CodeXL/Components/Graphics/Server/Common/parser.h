//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support functions used to parse incoming messages frome the client.
//==============================================================================

#ifndef GPS_PARSER_H
#define GPS_PARSER_H

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <list>

bool IsToken(char** sIn, const char* sTok);
bool IsNumber(char ch);
bool IsAlpha(char ch);
bool GetDWORD(char** sIn, unsigned long* pNum);
void SkipSpaces(char** sIn);
bool Expect(char** sIn, char c);
bool GetBool(char** sIn, bool* bVal);
bool GetFloat(char** sIn, float* pfVal);
bool IsVariable(char** sIn, const char* sTok);
bool Parse4DWORDS(char** sIn, unsigned long* p1, unsigned long* p2, unsigned long* p3, unsigned long* p4);

bool GetLONGVariable(char** sIn, const char* sTok, long* pNum);
bool GetDWORDVariable(char** sIn, const char* sTok, unsigned long* pNum);
bool GetBoolVariable(char** sIn, const char* sTok, bool* pbVal);
bool GetFloatVariable(char** sIn, const char* sTok, float* pbVal);

long GetConstantsFromCode(gtASCIIString code, gtASCIIString cKey, std::list<unsigned long>& usedSlots);

#endif // GPS_PARSER_H