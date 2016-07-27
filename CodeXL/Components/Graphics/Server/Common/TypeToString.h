//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Functions to convert value to string
//==============================================================================

#ifndef TYPETOSTRING_H
#define TYPETOSTRING_H

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtStringConstants.h>

#include "misc.h"

/// Converts a bool to a string
/// \param bVal
/// \return String version of the input data
inline gtASCIIString BoolToString(DWORD bVal)
{
    BOOL b = (BOOL)bVal;
    return b ? "TRUE" : "FALSE";
}

/// Converts a float to a string
/// \param fVal
/// \return String version of the input data
inline gtASCIIString FloatToString(float fVal)
{
    float* pF = (float*) & fVal;
    return FormatText("%f", *pF);
}

/// Converts a float array of size 4 to a string
/// \param fVal
/// \return String version of the input data
inline gtASCIIString Float4ToString(float fVal[4])
{
    return FormatText("%f, %f, %f, %f", fVal[ 0 ],  fVal[ 1 ], fVal[ 2 ], fVal[ 3 ]);
}

/// Converts a array of floats to a string
/// \param fVal
/// \param N
/// \return String version of the input data
inline gtASCIIString FloatNToString(float* fVal, int N)
{
    if (N < 1)
    {
        return "N/A";
    }

    gtASCIIString out = FloatToString(fVal[0]);

    for (int j = 1; j < N; j++)
    {
        out += FormatText(", %f", fVal[j]);
    }

    return out;
}

/// Converts a DWORD to a string
/// \param dwVal
/// \return String version of the input data
inline gtASCIIString DWORDToString(DWORD dwVal)
{
    return FormatText("%u", dwVal);
}

/// Converts a INT to a string
/// \param nVal
/// \return String version of the input data
inline gtASCIIString IntToString(INT nVal)
{
    return FormatText("%d", nVal);
}

/// Converts a UINT8 to a string
/// \param ui8Val
/// \return String version of the input data
inline gtASCIIString UINT8ToString(UINT8 ui8Val)
{
    return FormatText("0x%02x", ui8Val);
}

/// Converts a UINT to a string
/// \param uVal
/// \return String version of the input data
inline gtASCIIString UINTToString(UINT uVal)
{
    return FormatText("0x%x", uVal);
}

/// Converts a UINT64 to a hex string
/// \param uVal
/// \return String version of the input data
inline gtASCIIString UINT64ToHexString(gtUInt64 uVal)
{
    return FormatText(GT_64_BIT_POINTER_ASCII_FORMAT_LOWERCASE, uVal);
}

/// Converts a bool to a UINT64
/// \param uVal
/// \return String version of the input data
inline gtASCIIString UINT64ToString(UINT64 uVal)
{
    return FormatText("%llu", uVal);
}

/// Converts a double to a string
/// \param uVal
/// \return String version of the input data
inline gtASCIIString DoubleToString(double uVal)
{
    return FormatText("%.16f", uVal);
}
#endif // TYPETOSTRING_H
