//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/ARBProgramKeyWords.cpp#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================


#include "ARBProgramKeywords.h"
#include <tchar.h>

bool ShaderUtils::GetARBFPKeyWords(ShaderUtils::KeyWordsList& keyWords)
{
    keyWords.clear();

    KeyWords opKeyWords;
    opKeyWords.type = KW_Op;
    opKeyWords.strKeywords = _T("END OPTION ABS ABS_SAT FLR FLR_SAT FRC FRC_SAT LIT LIT_SAT MOV MOV_SAT COS COS_SAT EX2 EX2_SAT LG2 LG2_SAT RCP RCP_SAT ")
                             _T("RSQ RSQ_SAT SIN SIN_SAT SCS SCS_SAT POW POW_SAT ADD ADD_SAT DP3 DP3_SAT DP4 DP4_SAT DPH DPH_SAT DST DST_SAT MAX MAX_SAT ")
                             _T("MIN MIN_SAT MUL MUL_SAT SGE SGE_SAT SLT SLT_SAT SUB SUB_SAT XPD XPD_SAT CMP CMP_SAT LRP LRP_SAT MAD MAD_SAT SWZ SWZ_SAT ")
                             _T("TEX TEX_SAT TXP TXP_SAT TXB TXB_SAT KIL texture1D texture2D texture3D textureCUBE textureRECT ")
                             _T("ATTRIB PARAM TEMP OUTPUT ALIAS ")
                             _T("");
    keyWords.push_back(opKeyWords);

    //    KeyWords functionKeyWords;
    //    functionKeyWords.type = KW_Function;
    //    functionKeyWords.strKeywords = _T("fragment color texcoord fogcoord position state ");
    //    keyWords.push_back(functionKeyWords);

    return true;
}

bool ShaderUtils::GetARBVPKeyWords(ShaderUtils::KeyWordsList& keyWords)
{
    keyWords.clear();

    KeyWords opKeyWords;
    opKeyWords.type = KW_Op;
    opKeyWords.strKeywords = _T("END OPTION ARL ABS FLR FRC LIT MOV EX2 EXP LG2 LOG RCP RSQ POW ADD DP3 DP4 DPH DST MAX MIN MUL SGE SLT SUB XPD MAD SWZ ")
                             //_T("x y xy z xz yz xyz w xw yw xyw zw xzw yzw xyzw r g rg b rb gb rgb a ra ga rga ba rba gba rgba ")
                             _T("ATTRIB PARAM TEMP ADDRESS OUTPUT ALIAS ")
                             _T("");

    keyWords.push_back(opKeyWords);

    //    KeyWords functionKeyWords;
    //    functionKeyWords.type = KW_Function;
    //    functionKeyWords.strKeywords = _T("fragment color texcoord fogcoord position state ");
    //    keyWords.push_back(functionKeyWords);

    return true;
}
