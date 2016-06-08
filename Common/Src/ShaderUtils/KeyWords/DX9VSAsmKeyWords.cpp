//=====================================================================
//
// Author:      AMD Developer Tools Team
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DX9VSAsmKeyWords.cpp
// File contains <Seth file description here>
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX9VSAsmKeyWords.cpp#3 $
//
// Last checkin:  $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================
//   ( C ) AMD, Inc. 2009,2010 All rights reserved.
//=====================================================================

#include "DX9VSAsmKeywords.h"
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

typedef enum _VS_Model
{
    VS_Unknown  = 0x0,
    VS_1_1      = 0x1,
    VS_2_0      = 0x10,
    VS_2_X      = 0x20,
    VS_2_SW     = 0x40,
    VS_3_0      = 0x80,
    VS_3_SW     = 0x100,
} VS_Model;

static const VersionModels versionModels[] =
{
    {  1, 1, VS_1_1, 0 },
    {  2, 0, VS_2_0, 1 },
    {  2, 1, VS_2_X, 2 },
    {  3, 0, VS_3_0, 3 },
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

#define VS_1         VS_1_1
#define VS_2         VS_2_0|VS_2_X|VS_2_SW
#define VS_2_PLUS    VS_2_0|VS_2_X|VS_2_SW|VS_3_0|VS_3_SW
#define VS_2_X_PLUS  VS_2_X|VS_2_SW|VS_3_0|VS_3_SW
#define VS_3         VS_3_0|VS_3_SW
#define VS_ALL       VS_1_1|VS_2_0|VS_2_X|VS_2_SW|VS_3_0|VS_3_SW

/// Type definition for a Struct describing Vertex Shader instructions.
typedef struct _VSInstKeyWords
{
    TCHAR* pszText;         ///< Text name of the instruction.
    DWORD dwVSModels;       ///< Bitfield describing which shader models support this instruction.
    bool bSupportsModifers; ///< Does this instruction support modifiers?
    DWORD dwDCLIndexCount;  ///< Maximum number of decl's using this instruction.
} VSInstKeyWords;

/// Table describing all DX9 Vertex Shader Instructions.
static VSInstKeyWords g_VSInstKeyWordTable[] =
{
    _T("vs"),            VS_ALL,        false,   0,
    _T("vs_1_1"),        VS_1_1,        false,   0,
    _T("vs.1.1"),        VS_1_1,        false,   0,
    _T("vs_2_0"),        VS_2_0,        false,   0,
    _T("vs.2.0"),        VS_2_0,        false,   0,
    _T("vs_2_x"),        VS_2_X,        false,   0,
    _T("vs.2.x"),        VS_2_X,        false,   0,
    _T("vs_2_sw"),       VS_2_SW,    false,   0,
    _T("vs.2.sw"),       VS_2_SW,    false,   0,
    _T("vs_3_0"),        VS_3_0,        false,   0,
    _T("vs.3.0"),        VS_3_0,        false,   0,
    _T("vs_3_sw"),       VS_3_SW,    false,   0,
    _T("vs.3.sw"),       VS_3_SW,    false,   0,
    _T("abs"),           VS_2_PLUS,     true, 0,
    _T("add"),           VS_ALL,        true, 0,
    _T("break"),         VS_2_X_PLUS,   false,   0,
    _T("break_gt"),         VS_2_X_PLUS,   false,   0,
    _T("break_eq"),         VS_2_X_PLUS,   false,   0,
    _T("break_ge"),         VS_2_X_PLUS,   false,   0,
    _T("break_lt"),         VS_2_X_PLUS,   false,   0,
    _T("break_ne"),         VS_2_X_PLUS,   false,   0,
    _T("break_le"),         VS_2_X_PLUS,   false,   0,
    _T("breakp"),        VS_2_X_PLUS,   false,   0,
    _T("call"),          VS_2_X_PLUS,   false,   0,
    _T("callnz"),        VS_2_X_PLUS,   false,   0,
    _T("crs"),           VS_2_PLUS,     false,   0,
    // _T("dcl"),           VS_ALL,        false,   0,
    _T("dcl_2d"),        VS_3,       false,   0,
    _T("dcl_cube"),         VS_3,       false,   0,
    _T("dcl_volume"),    VS_3,       false,   0,
    _T("dcl_position"),     VS_ALL,        false,   2,
    _T("dcl_blendweight"),  VS_ALL,        false,   16,
    _T("dcl_blendindices"), VS_ALL,        false,   16,
    _T("dcl_normal"),    VS_ALL,        false,   2,
    _T("dcl_texcoord"),     VS_ALL,        false,   16,
    _T("dcl_tangent"),      VS_ALL,        false,   16,
    _T("dcl_binormal"),     VS_ALL,        false,   16,
    _T("dcl_tessfactor"),   VS_ALL,        false,   16,
    _T("dcl_positiont"), VS_ALL,        false,   16,
    _T("dcl_color"),     VS_ALL,        false,   16,
    _T("dcl_fog"),       VS_ALL,        false,   16,
    _T("dcl_depth"),     VS_ALL,        false,   16,
    _T("dcl_sample"),    VS_ALL,        false,   16,
    _T("def"),           VS_ALL,        false,   0,
    _T("defb"),          VS_2_PLUS,     false,   0,
    _T("defi"),          VS_2_PLUS,     false,   0,
    _T("dp3"),           VS_ALL,        true, 0,
    _T("dp4"),           VS_ALL,        true, 0,
    _T("dst"),           VS_ALL,        true, 0,
    _T("else"),          VS_2_PLUS,     false,   0,
    _T("endif"),         VS_2_PLUS,     false,   0,
    _T("endloop"),       VS_2_PLUS,     false,   0,
    _T("endrep"),        VS_2_PLUS,     false,   0,
    _T("exp"),           VS_ALL,        true, 0,
    _T("expp"),          VS_ALL,        true, 0,
    _T("false"),         VS_2_PLUS,     false,   0,
    _T("frc"),           VS_ALL,        false,   0,
    _T("if"),            VS_2_PLUS,     false,   0,
    _T("if_gt"),         VS_2_X_PLUS,   false,   0,
    _T("if_eq"),         VS_2_X_PLUS,   false,   0,
    _T("if_ge"),         VS_2_X_PLUS,   false,   0,
    _T("if_lt"),         VS_2_X_PLUS,   false,   0,
    _T("if_ne"),         VS_2_X_PLUS,   false,   0,
    _T("if_le"),         VS_2_X_PLUS,   false,   0,
    _T("label"),         VS_2_X_PLUS,   false,   0,
    _T("lit"),           VS_ALL,        true, 0,
    _T("log"),           VS_ALL,        true, 0,
    _T("log"),           VS_ALL,        true, 0,
    _T("loop"),          VS_2_PLUS,     false,   0,
    _T("lrp"),           VS_2_PLUS,     true, 0,
    _T("m3x2"),          VS_ALL,        true, 0,
    _T("m3x3"),          VS_ALL,        true, 0,
    _T("m3x4"),          VS_ALL,        true, 0,
    _T("m4x3"),          VS_ALL,        true, 0,
    _T("m4x4"),          VS_ALL,        true, 0,
    _T("mad"),           VS_ALL,        true, 0,
    _T("min"),           VS_ALL,        true, 0,
    _T("max"),           VS_ALL,        true, 0,
    _T("mov"),           VS_ALL,        true, 0,
    _T("mova"),          VS_2_PLUS,     true, 0,
    _T("mul"),           VS_ALL,        true, 0,
    _T("nop"),           VS_ALL,        false,   0,
    _T("nrm"),           VS_2_PLUS,     true, 0,
    _T("pow"),           VS_2_PLUS,     true, 0,
    _T("rcp"),           VS_ALL,        true, 0,
    _T("rep"),           VS_2_PLUS,     false,   0,
    _T("ret"),           VS_2_PLUS,     false,   0,
    _T("rsq"),           VS_ALL,        true, 0,
    _T("setp_gt"),       VS_2_X_PLUS,   false,   0,
    _T("setp_eq"),       VS_2_X_PLUS,   false,   0,
    _T("setp_ge"),       VS_2_X_PLUS,   false,   0,
    _T("setp_lt"),       VS_2_X_PLUS,   false,   0,
    _T("setp_ne"),       VS_2_X_PLUS,   false,   0,
    _T("setp_le"),       VS_2_X_PLUS,   false,   0,
    _T("sge"),           VS_ALL,        true, 0,
    _T("sgn"),           VS_2_PLUS,     true, 0,
    _T("sincos"),        VS_2_PLUS,     false,   0,
    _T("slt"),           VS_ALL,        true, 0,
    _T("sub"),           VS_ALL,        true, 0,
    _T("texldl"),        VS_3_0,        false,   0,
    _T("true"),          VS_2_PLUS,     false,   0,
};
static DWORD g_VSInstKeyWordTableSize = (sizeof(g_VSInstKeyWordTable) / sizeof(g_VSInstKeyWordTable[0]));

bool ShaderUtils::GetDX9VSInstKeyWords(std::string& strKeyWords, WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    strKeyWords.clear();

    DWORD dwVSModel = GetModels(wShaderModelMajor, wShaderModelMinor);

    if (dwVSModel == 0)
    {
        return false;
    }

    for (DWORD i = 0; i < g_VSInstKeyWordTableSize; i++)
    {
        if (g_VSInstKeyWordTable[i].dwVSModels & dwVSModel)
        {
            strKeyWords += g_VSInstKeyWordTable[i].pszText;
            strKeyWords += _T(" ");

            if (dwVSModel & VS_2_PLUS)
            {
                if (g_VSInstKeyWordTable[i].bSupportsModifers)
                {
                    strKeyWords += str(format(_T("%s_sat ")) % g_VSInstKeyWordTable[i].pszText);
                }
            }

            if (g_VSInstKeyWordTable[i].bSupportsModifers)
            {
                if (dwVSModel & VS_1_1)
                {
                    strKeyWords += str(format(_T("%s_sat %s_x2 %s_x2_sat %s_x4 %s_x4_sat %s_d2 %s_d2_sat ")) %
                                       g_VSInstKeyWordTable[i].pszText % g_VSInstKeyWordTable[i].pszText % g_VSInstKeyWordTable[i].pszText %
                                       g_VSInstKeyWordTable[i].pszText % g_VSInstKeyWordTable[i].pszText %
                                       g_VSInstKeyWordTable[i].pszText % g_VSInstKeyWordTable[i].pszText);
                }
            }

            for (DWORD j = 0; j < g_VSInstKeyWordTable[i].dwDCLIndexCount; j++)
            {
                strKeyWords += str(format(_T("%s%i ")) % g_VSInstKeyWordTable[i].pszText % j);

                if (dwVSModel & VS_2_PLUS)
                {
                    if (g_VSInstKeyWordTable[i].bSupportsModifers)
                    {
                        strKeyWords += str(format(_T("%s%i_sat ")) % g_VSInstKeyWordTable[i].pszText % j);
                    }
                }
            }
        }
    }

    return !strKeyWords.empty();
}

/// Type definition for a Struct describing Vertex Shader registers.
typedef struct _VSRegKeyWords
{
    TCHAR* pszText;               ///< Text name of the register.
    DWORD dwCount[4];             ///< The number of registers of this type supported for shader models VS_1_1, VS_2_0, VS_2_X & VS_3_0 respectively.
    bool bSupportsSwizzles;       ///< Does the register support swizzles?
    bool bSupportsWriteMasks;     ///< Does the register support write-masks?
    bool bSupportsVS1Modifiers;   ///< Does the register support VS1.x modifiers?
    bool bSupportsVS3Modifiers;   ///< Does the register support VS3.x modifiers?
    bool bIgnoreIndex;            ///< Does the register skip the index part of the name?
} VSRegKeyWords;

static VSRegKeyWords g_VSRegKeyWordTable[] =
{
    // Input Registers
    _T("c"),  96,  256,  256,  256,  true, false,   false,   true, false,
    _T("r"),  12,   12,   32,   32,  true, true, true, true, false,
    _T("a"),   1,    1,    1,    1,  true, false,   false,   false,   false,
    _T("v"),  16,   16,   16,   16,  true, false,   true, true, false,
    _T("b"),   0,   16,   16,   16,  true, false,   false,   true, false,
    _T("i"),   0,   16,   16,   16,  true, false,   false,   true, false,
    _T("al"),     0,    1,    1,    1,  true, false,   false,   true, true,
    _T("p"),   0,    1,    1,    1,  true, false,   false,   false,   false,
    _T("s"),   0,    0,    0,    4,  true, false,   false,   false,   false,

    // Output Registers
    _T("oPos"),   1,    1,    1,    0,  false,   true, false,   false,   true,
    _T("oFog"),   1,    1,    1,    0,  false,   true, false,   false,   true,
    _T("oPts"),   1,    1,    1,    0,  false,   true, false,   false,   true,
    _T("oD"),     2,    2,    2,    0,  false,   true, false,   false,   false,
    _T("oT"),     8,    8,    8,    0,  false,   true, false,   false,   false,
    _T("o"),   0,    0,    0,    12, false,   true, false,   false,   false,
};
static DWORD g_VSRegKeyWordTableSize = (sizeof(g_VSRegKeyWordTable) / sizeof(g_VSRegKeyWordTable[0]));

/// Type definition for a Struct describing Vertex Shader masks and swizzles.
typedef struct _VSMaskSwizzle
{
    TCHAR* pszText1;           ///< The swizzle text in rgba nomenclature.
    TCHAR* pszText2;           ///< The swizzle text in xyzw nomenclature.
    DWORD dwWriteMaskVSModels; ///< The shader models supporting this as a write-mask.
    DWORD dwSwizzleVSModels;   ///< The shader models supporting this as a swizzle.
} VSMaskSwizzle;

bool ShaderUtils::GetDX9VSRegKeyWords(std::string& strKeyWords, WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    strKeyWords.clear();

    DWORD dwVSModel = GetModels(wShaderModelMajor, wShaderModelMinor);

    if (dwVSModel == 0)
    {
        return false;
    }

    DWORD dwModelIndex = GetModelIndex(wShaderModelMajor, wShaderModelMinor);

    for (DWORD i = 0; i < g_VSRegKeyWordTableSize; i++)
    {
        if (g_VSRegKeyWordTable[i].bIgnoreIndex)
        {
            strKeyWords += str(format(_T("%s ")) % g_VSRegKeyWordTable[i].pszText);

            if (dwModelIndex == 0 && g_VSRegKeyWordTable[i].bSupportsVS1Modifiers)
            {
                strKeyWords += str(format(_T("%s_bias %s_bx2 ")) % g_VSRegKeyWordTable[i].pszText % g_VSRegKeyWordTable[i].pszText);
            }
            else if (dwModelIndex == 4 && g_VSRegKeyWordTable[i].bSupportsVS3Modifiers)
            {
                strKeyWords += str(format(_T("%s_abs ")) % g_VSRegKeyWordTable[i].pszText);
            }
        }
        else
            for (DWORD dwRegister = 0; dwRegister < g_VSRegKeyWordTable[i].dwCount[dwModelIndex]; dwRegister++)
            {
                strKeyWords += str(format(_T("%s%i ")) % g_VSRegKeyWordTable[i].pszText % dwRegister);

                if (dwModelIndex == 0 && g_VSRegKeyWordTable[i].bSupportsVS1Modifiers)
                {
                    strKeyWords += str(format(_T("%s%i_bias %s%i_bx2 ")) % g_VSRegKeyWordTable[i].pszText % dwRegister % g_VSRegKeyWordTable[i].pszText % dwRegister);
                }
                else if (dwModelIndex == 4 && g_VSRegKeyWordTable[i].bSupportsVS3Modifiers)
                {
                    strKeyWords += str(format(_T("%s%i_abs ")) % g_VSRegKeyWordTable[i].pszText % dwRegister);
                }
            }
    }

    strKeyWords += GetXYZWSwizzles();

    return !strKeyWords.empty();
}