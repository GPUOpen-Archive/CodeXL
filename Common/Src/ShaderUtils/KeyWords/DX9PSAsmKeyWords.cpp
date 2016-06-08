//=====================================================================
//
// Author:      AMD Developer Tools Team
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DX9PSAsmKeywords.cpp
// File contains <Seth file description here>
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX9PSAsmKeyWords.cpp#3 $
//
// Last checkin:  $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================
//   ( C ) AMD, Inc. 2009,2010 All rights reserved.
//=====================================================================

#include "DX9PSAsmKeywords.h"
#include <tchar.h>
#include <boost/format.hpp>
#include "d3d9types.h"
#include "Swizzles.h"

using boost::format;

/// Struct type definition mapping between shader model version number & Shader Model enum/index.
typedef struct
{
    WORD  wVersionMajor; ///< Shader model version major.
    WORD  wVersionMinor; ///< Shader model version minor.
    DWORD dwModels;      ///< Shader model enum.
    DWORD dwModelIndex;  ///< Shader model index.
} VersionModels;

typedef enum
{
    PS_Unknown  = 0x0,
    PS_1_1      = 0x1,
    PS_1_2      = 0x2,
    PS_1_3      = 0x4,
    PS_1_4      = 0x8,
    PS_2_0      = 0x10,
    PS_2_X      = 0x20,
    PS_2_SW     = 0x40,
    PS_3_0      = 0x80,
    PS_3_SW     = 0x100,
} PS_Model;

static const VersionModels versionModels[] =
{
    {  1, 1, PS_1_1, 0 },
    {  1, 2, PS_1_2, 0 },
    {  1, 3, PS_1_3, 0 },
    {  1, 4, PS_1_4, 1 },
    {  2, 0, PS_2_0, 2 },
    {  2, 1, PS_2_X, 3 },
    {  3, 0, PS_3_0, 4 },
};
static DWORD g_dwVersionModelsTableSize = (sizeof(versionModels) / sizeof(versionModels[0]));

static DWORD GetModels(WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    for (DWORD i = 0; i < g_dwVersionModelsTableSize; i++)
    {
        if (versionModels[i].wVersionMajor == wShaderModelMajor && versionModels[i].wVersionMinor == wShaderModelMinor)
        {
            return versionModels[i].dwModels;
        }
    }

    return 0;
}

static DWORD GetModelIndex(WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    for (DWORD i = 0; i < g_dwVersionModelsTableSize; i++)
    {
        if (versionModels[i].wVersionMajor == wShaderModelMajor && versionModels[i].wVersionMinor == wShaderModelMinor)
        {
            return versionModels[i].dwModelIndex;
        }
    }

    return 0;
}

#define PS_1         PS_1_1|PS_1_2|PS_1_3|PS_1_4
#define PS_1_1_1_3   PS_1_1|PS_1_2|PS_1_3
#define PS_1_2_PLUS  PS_1_2|PS_1_3|PS_1_4|PS_2_0|PS_2_X|PS_2_SW|PS_3_0|PS_3_SW
#define PS_1_4_PLUS  PS_1_4|PS_2_0|PS_2_X|PS_2_SW|PS_3_0|PS_3_SW
#define PS_2         PS_2_0|PS_2_X|PS_2_SW
#define PS_2_PLUS    PS_2_0|PS_2_X|PS_2_SW|PS_3_0|PS_3_SW
#define PS_2_X_PLUS  PS_2_X|PS_2_SW|PS_3_0|PS_3_SW
#define PS_3         PS_3_0|PS_3_SW
#define PS_ALL       PS_1_1|PS_1_2|PS_1_3|PS_1_4|PS_2_0|PS_2_X|PS_2_SW|PS_3_0|PS_3_SW

/// Type definition for a Struct describing Pixel Shader instructions.
typedef struct
{
    TCHAR* pszText;         ///< Text name of the instruction.
    DWORD dwPSModels;       ///< Bitfield describing which shader models support this instruction.
    bool bSupportsPP;       ///< Does this instruction support the Partial Precision modifier?
    bool bSupportsCentroid; ///< Does this instruction support the Centroid Sampling modifier?
    bool bSupportsModifers; ///< Does this instruction support saturation & other modifiers?
    DWORD dwDCLIndexCount;  ///< Maximum number of decl's using this instruction.
} PSInstKeyWords;

/// Table describing all DX9 Pixel Shader Instructions.
static PSInstKeyWords g_PSInstKeyWordTable[] =
{
    _T("ps"),            PS_ALL,         false,   false,   false,   0,
    _T("ps_1_1"),         PS_1_1,         false,   false,   false,   0,
    _T("ps.1.1"),         PS_1_1,         false,   false,   false,   0,
    _T("ps_1_2"),         PS_1_2,         false,   false,   false,   0,
    _T("ps.1.2"),         PS_1_2,         false,   false,   false,   0,
    _T("ps_1_3"),         PS_1_3,         false,   false,   false,   0,
    _T("ps.1.3"),         PS_1_3,         false,   false,   false,   0,
    _T("ps_1_4"),         PS_1_4,         false,   false,   false,   0,
    _T("ps.1.4"),         PS_1_4,         false,   false,   false,   0,
    _T("ps_2_0"),         PS_2_0,         false,   false,   false,   0,
    _T("ps.2.0"),         PS_2_0,         false,   false,   false,   0,
    _T("ps_2_x"),         PS_2_X,         false,   false,   false,   0,
    _T("ps.2.x"),         PS_2_X,         false,   false,   false,   0,
    _T("ps_2_sw"),         PS_2_SW,      false,   false,   false,   0,
    _T("ps.2.sw"),         PS_2_SW,      false,   false,   false,   0,
    _T("ps_3_0"),         PS_3_0,         false,   false,   false,   0,
    _T("ps.3.0"),         PS_3_0,         false,   false,   false,   0,
    _T("ps_3_sw"),         PS_3_SW,      false,   false,   false,   0,
    _T("ps.3.sw"),         PS_3_SW,      false,   false,   false,   0,
    _T("abs"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("add"),            PS_ALL,         true,   false,   true,   0,
    _T("bem"),            PS_1_4,         false,   false,   false,   0,
    _T("break"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("break_gt"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("break_eq"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("break_ge"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("break_lt"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("break_ne"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("break_le"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("breakp"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("call"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("callnz"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("cmp"),            PS_1_2_PLUS,   true,   false,   true,   0,
    _T("cnd"),            PS_1,         false,   false,   false,   0,
    _T("crs"),            PS_2_PLUS,      true,   false,   false,   0,
    _T("dcl"),            PS_2_PLUS,      true,   false,   false,   0,
    _T("dcl_2d"),         PS_2_PLUS,      true,   false,   false,   0,
    _T("dcl_cube"),         PS_2_PLUS,      true,   false,   false,   0,
    _T("dcl_volume"),      PS_2_PLUS,      true,   false,   false,   0,
    _T("dcl_position"),     PS_3,         true,   false,   false,   16,
    _T("dcl_blendweight"),  PS_3,         true,   false,   false,   16,
    _T("dcl_blendindices"), PS_3,         true,   false,   false,   16,
    _T("dcl_normal"),      PS_3,         true,   false,   false,   16,
    _T("dcl_texcoord"),      PS_3,         true,   true,   false,   16,
    _T("dcl_tangent"),      PS_3,         true,   false,   false,   16,
    _T("dcl_binormal"),      PS_3,         true,   false,   false,   16,
    _T("dcl_color"),      PS_3,         true,   false,   false,   16,
    _T("dcl_fog"),         PS_3,         true,   false,   false,   16,
    _T("dcl_depth"),      PS_3,         true,   false,   false,   16,
    _T("def"),            PS_ALL,         false,   false,   false,   0,
    _T("defi"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("defb"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("dp2add"),         PS_2_PLUS,      true,   false,   true,   0,
    _T("dp3"),            PS_ALL,         true,   false,   true,   0,
    _T("dp4"),            PS_1_2_PLUS,   true,   false,   true,   0,
    _T("dsx"),            PS_2_X_PLUS,   true,   false,   true,   0,
    _T("dsy"),            PS_2_X_PLUS,   true,   false,   true,   0,
    _T("else"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("endif"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("endloop"),         PS_3,         false,   false,   false,   0,
    _T("endrep"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("exp"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("false"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("frc"),            PS_2_PLUS,      true,   false,   false,   0,
    _T("if"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("if_gt"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("if_eq"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("if_ge"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("if_lt"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("if_ne"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("if_le"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("label"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("log"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("loop"),            PS_3,         false,   false,   false,   0,
    _T("lrp"),            PS_ALL,         true,   false,   true,   0,
    _T("m3x2"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("m3x3"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("m3x4"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("m4x3"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("m4x4"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("mad"),            PS_ALL,         true,   false,   true,   0,
    _T("min"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("max"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("mov"),            PS_ALL,         true,   false,   true,   0,
    _T("mul"),            PS_ALL,         true,   false,   true,   0,
    _T("nop"),            PS_ALL,         false,   false,   false,   0,
    _T("nrm"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("phase"),         PS_1_4,         false,   false,   false,   0,
    _T("pow"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("rcp"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("rep"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("ret"),            PS_2_X_PLUS,   false,   false,   false,   0,
    _T("rsq"),            PS_2_PLUS,      true,   false,   true,   0,
    _T("setp_gt"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("setp_eq"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("setp_ge"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("setp_lt"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("setp_ne"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("setp_le"),         PS_2_X_PLUS,   false,   false,   false,   0,
    _T("sincos"),         PS_2_PLUS,      true,   false,   false,   0,
    _T("sub"),            PS_ALL,         true,   false,   true,   0,
    _T("tex"),            PS_1_1_1_3,      false,   false,   false,   0,
    _T("texbem"),         PS_1_1_1_3,      false,   false,   false,   0,
    _T("texbeml"),         PS_1_1_1_3,      false,   false,   false,   0,
    _T("texcoord"),         PS_1_1_1_3,      false,   false,   false,   0,
    _T("texcrd"),         PS_1_4,         false,   false,   false,   0,
    _T("texdepth"),         PS_1_4,         false,   false,   false,   0,
    _T("texdp3"),         PS_1_1 | PS_1_2,   false,   false,   false,   0,
    _T("texdp3tex"),      PS_1_1 | PS_1_2,   false,   false,   false,   0,
    _T("texkill"),         PS_ALL,         true,   false,   false,   0,
    _T("texld"),         PS_1_4_PLUS,   true,   false,   false,   0,
    _T("texldb"),         PS_2_PLUS,      true,   false,   false,   0,
    _T("texldd"),         PS_2_X_PLUS,   true,   false,   false,   0,
    _T("texldl"),         PS_3_0,         true,   false,   false,   0,
    _T("texldp"),         PS_2_PLUS,      true,   false,   false,   0,
    _T("texm3x2depth"),  PS_1_3,         false,   false,   false,   0,
    _T("texm3x2pad"),    PS_1_1_1_3,      false,   false,   false,   0,
    _T("texm3x2tex"),    PS_1_1_1_3,      false,   false,   false,   0,
    _T("texm3x3"),       PS_1_1 | PS_1_2,   false,   false,   false,   0,
    _T("texm3x3pad"),    PS_1_1_1_3,      false,   false,   false,   0,
    _T("texm3x3spec"),   PS_1_1_1_3,      false,   false,   false,   0,
    _T("texm3x3tex"),    PS_1_1_1_3,      false,   false,   false,   0,
    _T("texm3x3vspec"),  PS_1_1_1_3,      false,   false,   false,   0,
    _T("texreg2ar"),     PS_1_1_1_3,      false,   false,   false,   0,
    _T("texreg2gb"),     PS_1_2 | PS_1_3,   false,   false,   false,   0,
    _T("texreg2rgb"),    PS_1_2 | PS_1_3,   false,   false,   false,   0,
    _T("true"),          PS_2_X_PLUS,   false,   false,   false,   0,
};
static DWORD g_PSInstKeyWordTableSize = (sizeof(g_PSInstKeyWordTable) / sizeof(g_PSInstKeyWordTable[0]));

bool ShaderUtils::GetDX9PSInstKeyWords(std::string& strKeyWords, WORD wShaderModelMajor, WORD wShaderModelMinor, bool bPartialPrecision)
{
    strKeyWords.clear();

    DWORD dwPSModel = GetModels(wShaderModelMajor, wShaderModelMinor);

    if (dwPSModel == 0)
    {
        return false;
    }

    for (DWORD i = 0; i < g_PSInstKeyWordTableSize; i++)
    {
        if (g_PSInstKeyWordTable[i].dwPSModels & dwPSModel)
        {
            if (bPartialPrecision)
            {
                if ((dwPSModel & PS_2_PLUS) && (g_PSInstKeyWordTable[i].bSupportsPP))
                {
                    strKeyWords += g_PSInstKeyWordTable[i].pszText;
                    strKeyWords += _T("_pp ");

                    if (g_PSInstKeyWordTable[i].bSupportsCentroid)
                    {
                        strKeyWords += str(format(_T("%s_centroid_pp %s_pp_centroid ")) % g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText);
                    }
                    else if (g_PSInstKeyWordTable[i].bSupportsModifers)
                    {
                        strKeyWords += str(format(_T("%s_sat_pp %s_pp_sat ")) % g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText);
                    }

                    for (DWORD j = 0; j < g_PSInstKeyWordTable[i].dwDCLIndexCount; j++)
                    {
                        strKeyWords += str(format(_T("%s%i_pp ")) % g_PSInstKeyWordTable[i].pszText % j);

                        if (g_PSInstKeyWordTable[i].bSupportsCentroid)
                        {
                            strKeyWords += str(format(_T("%s%i_centroid_pp %s%i_pp_centroid ")) % g_PSInstKeyWordTable[i].pszText % j % g_PSInstKeyWordTable[i].pszText % j);
                        }
                        else if (g_PSInstKeyWordTable[i].bSupportsModifers)
                        {
                            strKeyWords += str(format(_T("%s%i_sat_pp %s%i_pp_sat ")) % g_PSInstKeyWordTable[i].pszText % j % g_PSInstKeyWordTable[i].pszText % j);
                        }
                    }
                }
            }
            else
            {
                strKeyWords += g_PSInstKeyWordTable[i].pszText;
                strKeyWords += _T(" ");

                if (dwPSModel & PS_2_PLUS)
                {
                    if (g_PSInstKeyWordTable[i].bSupportsCentroid)
                    {
                        strKeyWords += str(format(_T("%s_centroid ")) % g_PSInstKeyWordTable[i].pszText);
                    }
                    else if (g_PSInstKeyWordTable[i].bSupportsModifers)
                    {
                        strKeyWords += str(format(_T("%s_sat ")) % g_PSInstKeyWordTable[i].pszText);
                    }
                }

                if (g_PSInstKeyWordTable[i].bSupportsModifers)
                {
                    if (dwPSModel & PS_1_4)
                    {
                        strKeyWords += str(format(_T("%s_sat %s_x2 %s_x2_sat %s_x4 %s_x4_sat %s_x8 %s_x8_sat ")
                                                  _T("%s_d2 %s_d2_sat %s_d4 %s_d4_sat %s_d8 %s_d8_sat ")) %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText);
                    }
                    else if (dwPSModel & PS_1_1_1_3)
                    {
                        strKeyWords += str(format(_T("%s_sat %s_x2 %s_x2_sat %s_x4 %s_x4_sat %s_d2 %s_d2_sat ")) %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText %
                                           g_PSInstKeyWordTable[i].pszText % g_PSInstKeyWordTable[i].pszText);
                    }
                }

                for (DWORD j = 0; j < g_PSInstKeyWordTable[i].dwDCLIndexCount; j++)
                {
                    strKeyWords += str(format(_T("%s%i ")) % g_PSInstKeyWordTable[i].pszText % j);

                    if (dwPSModel & PS_2_PLUS)
                    {
                        if (g_PSInstKeyWordTable[i].bSupportsCentroid)
                        {
                            strKeyWords += str(format(_T("%s%i_centroid ")) % g_PSInstKeyWordTable[i].pszText % j);
                        }
                        else if (g_PSInstKeyWordTable[i].bSupportsModifers)
                        {
                            strKeyWords += str(format(_T("%s%i_sat ")) % g_PSInstKeyWordTable[i].pszText % j);
                        }
                    }
                }
            }
        }
    }

    return !strKeyWords.empty();
}

/// Type definition for a Struct describing Pixel Shader registers.
typedef struct _PSRegKeyWords
{
    TCHAR* pszText;                  ///< Text name of the register.
    DWORD dwCount[5];                ///< The number of registers of this type supported for shader models PS_1_1_1_3, PS_1_4, PS_2_0, PS_2_X & PS_3_0 respectively.
    bool bSupportsSwizzles;          ///< Does the register support swizzles?
    bool bSupportsWriteMasks;        ///< Does the register support write-masks?
    bool bSupportsPS1Modifiers;      ///< Does the register support PS1.x modifiers?
    bool bSupportsPS14TexModifiers;  ///< Does the register support PS1.4 modifiers?
    bool bSupportsPS3Modifiers;      ///< Does the register support PS3.x modifiers?
} PSRegKeyWords;

static PSRegKeyWords g_PSRegKeyWordTable[] =
{
    _T("c"), 8, 8, 32,   32,   224,  true, false,   false,   false,   true,
    _T("r"), 2, 6, 32,   32,   32,      true, true, true, false,   true,
    _T("t"), 4, 6, 8, 8, 0,    true, false,   false,   true, true,
    _T("v"), 2, 2, 2, 2, 10,      true, false,   true, false,   true,
    _T("i"), 0, 0, 0, 16,   16,      true, false,   false,   false,   true,
    _T("b"), 0, 0, 0, 16,   16,      true, false,   false,   false,   true,
    _T("p"), 0, 0, 0, 1, 1,    true, false,   false,   false,   false,
    _T("s"), 0, 0, 16,   16,   16,      true, false,   false,   false,   false,
    _T("oC"),   0, 0, 4, 4, 4,    false,   true, false,   false,   false,
    _T("l"), 0, 0, 0, 15,   2048, false,   false,   false,   false,   false,
};
static DWORD g_PSRegKeyWordTableSize = (sizeof(g_PSRegKeyWordTable) / sizeof(g_PSRegKeyWordTable[0]));

/// Type definition for a Struct describing Pixel Shader masks and swizzles.
typedef struct _PSMaskSwizzle
{
    TCHAR* pszText1;           ///< The swizzle text in rgba nomenclature.
    TCHAR* pszText2;           ///< The swizzle text in xyzw nomenclature.
    DWORD dwWriteMaskPSModels; ///< The shader models supporting this as a write-mask.
    DWORD dwSwizzlePSModels;   ///< The shader models supporting this as a swizzle.
} PSMaskSwizzle;

static PSMaskSwizzle g_PSMaskSwizzleTable[] =
{
    // First the write masks
    // Some of these will also be swizzles that may or may not be legal for this shader model
    _T("r"), _T("x"), PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("rr"),   _T("xx"),   0,          PS_1_4_PLUS,
    _T("rrr"),  _T("xxx"),  0,          PS_1_4_PLUS,
    _T("rrrr"), _T("xxxx"), 0,          PS_1_4_PLUS,
    _T("g"), _T("y"), PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("gg"),   _T("yy"),   0,          PS_1_4_PLUS,
    _T("ggg"),  _T("yyy"),  0,          PS_1_4_PLUS,
    _T("gggg"), _T("yyyy"), 0,          PS_1_4_PLUS,
    _T("b"), _T("z"), PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("bb"),   _T("zz"),   0,          PS_1_4_PLUS,
    _T("bbb"),  _T("zzz"),  0,          PS_1_4_PLUS,
    _T("bbbb"), _T("zzzz"), 0,          PS_1_4_PLUS,
    _T("a"), _T("w"), PS_ALL,        PS_ALL,
    _T("aa"),   _T("ww"),   0,          PS_ALL,
    _T("aaa"),  _T("www"),  0,          PS_ALL,
    _T("aaaa"), _T("wwww"), 0,           PS_ALL,
    _T("rg"),   _T("xy"),   PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("rb"),   _T("xz"),   PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("ra"),   _T("xw"),   PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("gb"),   _T("yz"),   PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("ga"),   _T("yw"),   PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("ba"),   _T("zw"),   PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("rgb"),  _T("xyz"),  PS_ALL,     PS_1_4_PLUS,
    _T("rga"),  _T("xyw"),  PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("rba"),  _T("xzw"),  PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("gba"),  _T("yzw"),  PS_1_4_PLUS,   PS_1_4_PLUS,
    _T("rgba"), _T("xyzw"), PS_ALL,     PS_1_4_PLUS,

    // Now the swizzles
    // Some of these will also be illegal write masks
    _T("gbra"), _T("yzxw"), 0,          PS_1_4_PLUS,
    _T("brga"), _T("zxyw"), 0,          PS_1_4_PLUS,
    _T("abgr"), _T("wzyx"), 0,          PS_1_4_PLUS,
};
static DWORD g_PSMaskSwizzleTableSize = (sizeof(g_PSMaskSwizzleTable) / sizeof(g_PSMaskSwizzleTable[0]));

bool ShaderUtils::GetDX9PSRegKeyWords(std::string& strKeyWords, WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    strKeyWords.clear();

    DWORD dwPSModel = GetModels(wShaderModelMajor, wShaderModelMinor);

    if (dwPSModel == 0)
    {
        return false;
    }

    DWORD dwModelIndex = GetModelIndex(wShaderModelMajor, wShaderModelMinor);

    for (DWORD i = 0; i < g_PSRegKeyWordTableSize; i++)
    {
        for (DWORD dwRegister = 0; dwRegister < g_PSRegKeyWordTable[i].dwCount[dwModelIndex]; dwRegister++)
        {
            strKeyWords += str(format(_T("%s%i ")) % g_PSRegKeyWordTable[i].pszText % dwRegister);

            if (dwModelIndex == 0 && g_PSRegKeyWordTable[i].bSupportsPS1Modifiers)
            {
                strKeyWords += str(format(_T("%s%i_bias %s%i_bx2 ")) % g_PSRegKeyWordTable[i].pszText % dwRegister % g_PSRegKeyWordTable[i].pszText % dwRegister);
            }
            else if (dwModelIndex == 1 && g_PSRegKeyWordTable[i].bSupportsPS1Modifiers)
            {
                strKeyWords += str(format(_T("%s%i_bias %s%i_bx2 %s%i_x2 ")) % g_PSRegKeyWordTable[i].pszText % dwRegister % g_PSRegKeyWordTable[i].pszText % dwRegister % g_PSRegKeyWordTable[i].pszText % dwRegister);
            }
            else if (dwModelIndex == 1 && g_PSRegKeyWordTable[i].bSupportsPS14TexModifiers)
            {
                strKeyWords += str(format(_T("%s%i_dz %s%i_db %s%i_dw %s%i_da ")) % g_PSRegKeyWordTable[i].pszText % dwRegister % g_PSRegKeyWordTable[i].pszText % dwRegister % g_PSRegKeyWordTable[i].pszText % dwRegister % g_PSRegKeyWordTable[i].pszText % dwRegister);
            }
            else if (dwModelIndex == 4 && g_PSRegKeyWordTable[i].bSupportsPS3Modifiers)
            {
                strKeyWords += str(format(_T("%s%i_abs ")) % g_PSRegKeyWordTable[i].pszText % dwRegister);
            }
        }
    }

    if (dwModelIndex >= 3)   // SM2 ?
    {
        strKeyWords += _T("oDepth ");

        if (dwModelIndex >= 4)   // SM3 ?
        {
            strKeyWords += _T("vFace vPos aL ");
        }
    }

    if (dwModelIndex >= 3)   // Arbitrary swizzle
    {
        strKeyWords += GetRGBASwizzles() + GetXYZWSwizzles();
    }
    else
    {
        for (DWORD i = 0; i < g_PSMaskSwizzleTableSize; i++)
        {
            if ((g_PSMaskSwizzleTable[i].dwSwizzlePSModels & dwPSModel) || (g_PSMaskSwizzleTable[i].dwWriteMaskPSModels & dwPSModel))
            {
                strKeyWords += str(format(_T("%s %s ")) % g_PSMaskSwizzleTable[i].pszText1 % g_PSMaskSwizzleTable[i].pszText2);
            }
        }
    }

    return !strKeyWords.empty();
}