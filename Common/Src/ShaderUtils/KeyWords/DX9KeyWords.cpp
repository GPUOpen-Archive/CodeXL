//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DX9KeyWords.cpp
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX9KeyWords.cpp#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#include "DX9Keywords.h"
#include <tchar.h>
#include <boost/format.hpp>
#include "d3d9types.h"
#include "DX9PSAsmKeywords.h"
#include "DX9VSAsmKeywords.h"

bool ShaderUtils::GetDX9AsmKeyWords(ShaderUtils::KeyWordsList& keyWords, const DWORD* pdwShader)
{
    keyWords.clear();

    if (pdwShader == NULL)
    {
        return false;
    }

    return GetDX9AsmKeyWords(keyWords, DECODE_D3D9_SHADER_TYPE(*pdwShader), (WORD) D3DSHADER_VERSION_MAJOR(*pdwShader), (WORD) D3DSHADER_VERSION_MINOR(*pdwShader));
}

bool ShaderUtils::GetDX9AsmKeyWords(ShaderUtils::KeyWordsList& keyWords, D3D9ShaderUtils::DX9ShaderType shaderType, WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    keyWords.clear();

    if (shaderType != D3D9ShaderUtils::DX9PixelShader && shaderType != D3D9ShaderUtils::DX9VertexShader)
    {
        return false;
    }

    KeyWords opKeyWords;
    opKeyWords.type = KW_Op;

    if (shaderType == D3D9ShaderUtils::DX9PixelShader)
    {
        GetDX9PSInstKeyWords(opKeyWords.strKeywords, wShaderModelMajor, wShaderModelMinor, true);
    }
    else if (shaderType == D3D9ShaderUtils::DX9VertexShader)
    {
        GetDX9VSInstKeyWords(opKeyWords.strKeywords, wShaderModelMajor, wShaderModelMinor);
    }

    if (!opKeyWords.strKeywords.empty())
    {
        keyWords.push_back(opKeyWords);
    }

    KeyWords regKeyWords;
    regKeyWords.type = KW_Reg;

    if (shaderType == D3D9ShaderUtils::DX9PixelShader)
    {
        GetDX9PSRegKeyWords(regKeyWords.strKeywords, wShaderModelMajor, wShaderModelMinor);
    }
    else if (shaderType == D3D9ShaderUtils::DX9VertexShader)
    {
        GetDX9VSRegKeyWords(regKeyWords.strKeywords, wShaderModelMajor, wShaderModelMinor);
    }

    if (!regKeyWords.strKeywords.empty())
    {
        keyWords.push_back(regKeyWords);
    }

    return !keyWords.empty();
}


#pragma message(__FILE__  "TODO: Improve GetDX9HLSLKeyWords to consider shader type & shader model")
/// \todo Improve GetDX9HLSLKeyWords to consider shader type & shader model

bool ShaderUtils::GetDX9HLSLKeyWords(ShaderUtils::KeyWordsList& keyWords, D3D9ShaderUtils::DX9ShaderType shaderType, WORD wShaderModelMajor, WORD wShaderModelMinor)
{
    UNREFERENCED_PARAMETER(shaderType);
    UNREFERENCED_PARAMETER(wShaderModelMajor);
    UNREFERENCED_PARAMETER(wShaderModelMinor);

    keyWords.clear();

    KeyWords opKeyWords;
    opKeyWords.type = KW_Op;
    opKeyWords.strKeywords =
        _T("asm asm_fragment bool column_major compile compile_fragment const discard decl DECL ")
        _T("do double else extern false for half if in inline inout int matrix out ")
        _T("pass PASS pixelfragment return register row_major sampler sampler1D sampler2D ")
        _T("sampler3D samplerCUBE sampler_state shared stateblock stateblock_state static ")
        _T("string struct technique TECHNIQUE texture texture1D texture2D texture3D textureCUBE true ")
        _T("typedef uniform vector vertexfragment void volatile while")
        _T("vs_1_1 vs_2_0 vs_2_x vs_3_0 ps_1_1 ps_1_2 ps_1_3 ps_1_4 ps_2_0 ps_2_x ps_3_0");
    keyWords.push_back(opKeyWords);

    KeyWords typeKeyWords;
    typeKeyWords.type = KW_Type;
    typeKeyWords.strKeywords =
        _T("bool bool1 bool2 bool3 bool4 bool1x1 bool1x2 bool1x3 bool1x4 bool2x1 bool2x2 bool2x3 bool2x4 bool3x1 bool3x2 bool3x3 bool3x4 bool4x1 bool4x2 bool4x3 bool4x4 ")
        _T("BOOL BOOL1 BOOL2 BOOL3 BOOL4 BOOL1x1 BOOL1x2 BOOL1x3 BOOL1x4 BOOL2x1 BOOL2x2 BOOL2x3 BOOL2x4 BOOL3x1 BOOL3x2 BOOL3x3 BOOL3x4 BOOL4x1 BOOL4x2 BOOL4x3 BOOL4x4 ")
        _T("int int1 int2 int3 int4 int1x1 int1x2 int1x3 int1x4 int2x1 int2x2 int2x3 int2x4 int3x1 int3x2 int3x3 int3x4 int4x1 int4x2 int4x3 int4x4 ")
        _T("half half1 half2 half3 half4 half1x1 half1x2 half1x3 half1x4 half2x1 half2x2 half2x3 half2x4 half3x1 half3x2 half3x3 half3x4 half4x1 half4x2 half4x3 half4x4 ")
        _T("double double1 double2 double3 double4 double1x1 double1x2 double1x3 double1x4 double2x1 double2x2 double2x3 double2x4 double3x1 double3x2 double3x3 double3x4 double4x1 double4x2 double4x3 double4x4 ")
        _T("float float1 float2 float3 float4 float1x1 float1x2 float1x3 float1x4 float2x1 float2x2 float2x3 float2x4 float3x1 float3x2 float3x3 float3x4 float4x1 float4x2 float4x3 float4x4 ")
        _T("matrix vector pixelshader vertexshader struct typedef ");
    keyWords.push_back(typeKeyWords);

    KeyWords semanticsKeyWords;
    semanticsKeyWords.type = KW_Semantics;
    semanticsKeyWords.strKeywords =
        // For types where multiple instances are supported the number I've included is arbitrary in some cases
        _T("BINORMAL BINORMAL0 BINORMAL1 BINORMAL2 BINORMAL3 ")
        _T("BLENDINDICES BLENDINDICES0 BLENDINDICES1 BLENDINDICES2 BLENDINDICES3 BLENDINDICES4 BLENDINDICES5 BLENDINDICES6 BLENDINDICES7 ")
        _T("BLENDWEIGHT BLENDWEIGHT0 BLENDWEIGHT1 BLENDWEIGHT2 BLENDWEIGHT3 BLENDWEIGHT4 BLENDWEIGHT5 BLENDWEIGHT6 BLENDWEIGHT7 ")
        _T("COLOR COLOR0 COLOR1 COLOR2 COLOR3 COLOR4 COLOR5 COLOR6 COLOR7 ")
        _T("NORMAL NORMAL0 NORMAL POSITION POSITION0 POSITION1 POSITIONT PSIZE PSIZE0 PSIZE1 PSIZE2 PSIZE3 ")
        _T("TANGENT TANGENT0 TANGENT1 TANGENT2 TANGENT3 TESSFACTOR TESSFACTOR0 TESSFACTOR1 TESSFACTOR2 TESSFACTOR3 ")
        _T("TEXCOORD TEXCOORD0 TEXCOORD1 TEXCOORD2 TEXCOORD3 TEXCOORD4 TEXCOORD5 TEXCOORD6 TEXCOORD7 ")
        _T("VFACE VPOS DEPTH DEPTH0 DEPTH1 DEPTH2 DEPTH3 ");
    keyWords.push_back(semanticsKeyWords);

    KeyWords reservedKeyWords;
    reservedKeyWords.type = KW_Reserved;
    reservedKeyWords.strKeywords =
        // Reserved words
        _T("auto break case catch char class default delete dynamic_cast dynamic_cast enum explicit ")
        _T("friend goto long mutable namespace new operator private protected public ")
        _T("reinterpret_cast short signed sizeof static_cast switch template this throw try ")
        _T("typename union unsigned using virtual ");
    keyWords.push_back(reservedKeyWords);

    //    KeyWords opKeyWords;
    //    opKeyWords.type = KW_Op;
    //    opKeyWords.strKeywords =
    //    Preprocessor stuff
    //    _T("#define #if #elif #else #endif #error #if #ifdef #ifndef #include #line #pragma #undef ")
    //    _T("pack_matrix row_major column_major warning type once default disable error defined")//
    //    keyWords.push_back(opKeyWords);

    KeyWords functionKeyWords;
    functionKeyWords.type = KW_Function;
    functionKeyWords.strKeywords =
        _T("abs acos all any asin atan atan2 ceil clamp clip cos cosh cross D3DCOLORtoUBYTE ")
        _T("ddx ddy degrees determinant distance dot exp exp2 faceforward floor fmod frac ")
        _T("frexp fwidth isfinite isinf isnan ldexp length lerp lit log log2 log10 max min ")
        _T("modf mul noise normalize pow radians reflect refract round rsqrt saturate sign ")
        _T("sin sincos sinh smoothstep sqrt step tan tanh tex1D tex1Dbias tex1Dlod tex1Dgrad tex1Dproj ")
        _T("tex2D tex2Dbias tex2Dlod tex2Dgrad tex2Dproj tex3D tex3Dbias tex3Dlod tex3Dgrad tex3Dproj ")
        _T("texCUBEproj texCUBEbias texCUBElod texCUBEgrad texCUBE transpose");
    keyWords.push_back(functionKeyWords);

    return true;
}