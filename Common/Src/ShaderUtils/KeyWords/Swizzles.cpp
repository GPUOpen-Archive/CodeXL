//=====================================================================
//
// Author:      AMD Developer Tools Team
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DX10AsmKeywords.cpp
// File contains <Seth file description here>
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/Swizzles.cpp#3 $
//
// Last checkin:  $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================
//   ( C ) AMD, Inc. 2009,2010 All rights reserved.
//=====================================================================

#include "Swizzles.h"
#include <tchar.h>
#include <boost/format.hpp>

using boost::format;

/// Simple type definition defining the alternative names for channels.
typedef struct
{
    TCHAR* pszText1;  ///< The channel name in rgba nomenclature.
    TCHAR* pszText2;  ///< The channel name in xyzw nomenclature.
} ArbitrarySwizzleChannel;

/// Simple struct defining the alternative names for channels.
ArbitrarySwizzleChannel g_ArbitrarySwizzleChannelTable[] =
{
    _T(""), _T(""),
    _T("r"), _T("x"),
    _T("g"), _T("y"),
    _T("b"), _T("z"),
    _T("a"), _T("w"),
};
DWORD g_ArbitrarySwizzleChannelTableSize = (sizeof(g_ArbitrarySwizzleChannelTable) / sizeof(g_ArbitrarySwizzleChannelTable[0]));

const std::string GenerateRGBASwizzles()
{
    std::string strSwizzles;

    for (DWORD i0 = 0; i0 < g_ArbitrarySwizzleChannelTableSize; i0++)
    {
        for (DWORD i1 = 0; i1 < g_ArbitrarySwizzleChannelTableSize; i1++)
        {
            for (DWORD i2 = 0; i2 < g_ArbitrarySwizzleChannelTableSize; i2++)
            {
                for (DWORD i3 = 0; i3 < g_ArbitrarySwizzleChannelTableSize; i3++)
                {
                    strSwizzles += str(format(_T("%s%s%s%s ")) % g_ArbitrarySwizzleChannelTable[i0].pszText1 %
                                       g_ArbitrarySwizzleChannelTable[i1].pszText1 %
                                       g_ArbitrarySwizzleChannelTable[i2].pszText1 %
                                       g_ArbitrarySwizzleChannelTable[i3].pszText1);
                }
            }
        }
    }

    return strSwizzles;
}

const std::string GenerateXYZWSwizzles()
{
    std::string strSwizzles;

    for (DWORD i0 = 0; i0 < g_ArbitrarySwizzleChannelTableSize; i0++)
    {
        for (DWORD i1 = 0; i1 < g_ArbitrarySwizzleChannelTableSize; i1++)
        {
            for (DWORD i2 = 0; i2 < g_ArbitrarySwizzleChannelTableSize; i2++)
            {
                for (DWORD i3 = 0; i3 < g_ArbitrarySwizzleChannelTableSize; i3++)
                {
                    strSwizzles += str(format(_T("%s%s%s%s ")) % g_ArbitrarySwizzleChannelTable[i0].pszText2 %
                                       g_ArbitrarySwizzleChannelTable[i1].pszText2 %
                                       g_ArbitrarySwizzleChannelTable[i2].pszText2 %
                                       g_ArbitrarySwizzleChannelTable[i3].pszText2);
                }
            }
        }
    }

    return strSwizzles;
}

const std::string& ShaderUtils::GetRGBASwizzles()
{
    static const std::string strSwizzles = GenerateRGBASwizzles();

    return strSwizzles;
}
const std::string& ShaderUtils::GetXYZWSwizzles()
{
    static const std::string strSwizzles = GenerateXYZWSwizzles();

    return strSwizzles;
}