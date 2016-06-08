//=====================================================================
//
// Author:      AMD Developer Tools Team
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DX10Keywords.cpp
// File contains <Seth file description here>
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX10KeyWords.cpp#3 $
//
// Last checkin:  $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================
//   ( C ) AMD, Inc. 20010 All rights reserved.
//=====================================================================

#include "DX10Keywords.h"
#include <set>
#include <tchar.h>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include "D3D10ShaderUtils.h"
#include "DX10AsmKeyWords.h"

using namespace D3D10ShaderUtils;

bool ShaderUtils::GetDX10AsmKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader)
{
    keyWords.clear();

    GetDX10InstKeyWords(keyWords, shader);
    GetDX10RegKeyWords(keyWords, shader);

    return !keyWords.empty();
}

bool ShaderUtils::GetDX10HLSLKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& /*shader*/)
{
    keyWords.clear();

    KeyWords opKeyWords;
    opKeyWords.type = KW_Op;
    opKeyWords.strKeywords =
        _T("Break Compile Const Continue DepthStencilView Discard Do Else ")
        _T("Extern False FALSE For If In Inline Inout Namespace Nointerpolation NULL Out  ")
        _T("RenderTargetView Return Register ")
        _T("Shared Stateblock Stateblock_state Static String Switch ")
        _T("True TRUE Typedef Uniform Void Volatile While ")
        _T("break compile const continue discard do else ")
        _T("extern false for if in inline inout namespace nointerpolation out ")
        _T("return register ")
        _T("shared stateblock stateblock_state static string switch ")
        _T("true typedef uniform void volatile while maxvertexcount ")
        _T("MIN_MAG_MIP_POINT MIN_MAG_POINT_MIP_LINEAR MIN_POINT_MAG_LINEAR_MIP_POINT MIN_POINT_MAG_MIP_LINEAR MIN_LINEAR_MAG_MIP_POINT MIN_LINEAR_MAG_POINT_MIP_LINEAR ")
        _T("MIN_MAG_LINEAR_MIP_POINT MIN_MAG_MIP_LINEAR ANISOTROPIC COMPARISON_MIN_MAG_MIP_POINT COMPARISON_MIN_MAG_POINT_MIP_LINEAR COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT ")
        _T("COMPARISON_MIN_POINT_MAG_MIP_LINEAR COMPARISON_MIN_LINEAR_MAG_MIP_POINT COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR COMPARISON_MIN_MAG_LINEAR_MIP_POINT ")
        _T("COMPARISON_MIN_MAG_MIP_LINEAR COMPARISON_ANISOTROPIC TEXT_1BIT wrap clamp ")
        _T("ZERO ONE SRC_COLOR INV_SRC_COLOR SRC_ALPHA INV_SRC_ALPHA DEST_ALPHA INV_DEST_ALPHA DEST_COLOR INV_DEST_COLOR SRC_ALPHA_SAT ")
        _T("BLEND_FACTOR INV_BLEND_FACTOR SRC1_COLOR INV_SRC1_COLOR SRC1_ALPHA INV_SRC1_ALPHA")
        _T("vs_1_1 vs_2_0 vs_2_x vs_3_0 vs_4_0 vs_4_1 vs_5_0 gs_4_0 gs_4_1 gs_5_0 ps_1_1 ps_1_2 ps_1_3 ps_1_4 ps_2_0 ps_2_x ps_3_0 ps_4_0 ps_4_1 ps_5_0 cs_4_0 cs_4_1 cs_5_0 hs_5_0 ds_5_0");

    // These were the old pre SM4.0 keywords. I'm leaving these here for now until we can be sure that are all deprecated

    //       _T("asm asm_fragment bool column_major compile compile_fragment const discard decl DECL ")
    //       _T("do double else extern false for half if in inline inout int matrix out ")
    //       _T("pass PASS pixelfragment return register row_major sampler sampler1D sampler2D ")
    //       _T("sampler3D samplerCUBE sampler_state shared stateblock stateblock_state static ")
    //       _T("string struct technique TECHNIQUE texture texture1D texture2D texture3D textureCUBE true ")
    //       _T("typedef uniform vector vertexfragment void volatile while")
    //       _T("vs_1_1 vs_2_0 vs_2_x vs_3_0 ps_1_1 ps_1_2 ps_1_3 ps_1_4 ps_2_0 ps_2_x ps_3_0");
    keyWords.push_back(opKeyWords);

    KeyWords typeKeyWords;
    typeKeyWords.type = KW_Type;
    typeKeyWords.strKeywords =
        _T("bool bool1 bool2 bool3 bool4 bool1x1 bool1x2 bool1x3 bool1x4 bool2x1 bool2x2 bool2x3 bool2x4 bool3x1 bool3x2 bool3x3 bool3x4 bool4x1 bool4x2 bool4x3 bool4x4 ")
        _T("BOOL BOOL1 BOOL2 BOOL3 BOOL4 BOOL1x1 BOOL1x2 BOOL1x3 BOOL1x4 BOOL2x1 BOOL2x2 BOOL2x3 BOOL2x4 BOOL3x1 BOOL3x2 BOOL3x3 BOOL3x4 BOOL4x1 BOOL4x2 BOOL4x3 BOOL4x4 ")
        _T("int int1 int2 int3 int4 int1x1 int1x2 int1x3 int1x4 int2x1 int2x2 int2x3 int2x4 int3x1 int3x2 int3x3 int3x4 int4x1 int4x2 int4x3 int4x4 ")
        _T("uint uint1 uint2 uint3 uint4 uint1x1 uint1x2 uint1x3 uint1x4 uint2x1 uint2x2 uint2x3 uint2x4 uint3x1 uint3x2 uint3x3 uint3x4 uint4x1 uint4x2 uint4x3 uint4x4 ")
        _T("half half1 half2 half3 half4 half1x1 half1x2 half1x3 half1x4 half2x1 half2x2 half2x3 half2x4 half3x1 half3x2 half3x3 half3x4 half4x1 half4x2 half4x3 half4x4 ")
        _T("double double1 double2 double3 double4 double1x1 double1x2 double1x3 double1x4 double2x1 double2x2 double2x3 double2x4 double3x1 double3x2 double3x3 double3x4 double4x1 double4x2 double4x3 double4x4 ")
        _T("float float1 float2 float3 float4 float1x1 float1x2 float1x3 float1x4 float2x1 float2x2 float2x3 float2x4 float3x1 float3x2 float3x3 float3x4 float4x1 float4x2 float4x3 float4x4 ")
        _T("snorm unorm Buffer Vector matrix Matrix Struct struct Sampler Sampler1D Sampler2D Sampler3D SamplerCUBE GeometryShader PixelShader VertexShader HullShader DomainShader ComputeShader ")
        _T("BlendState blendstate CBuffer cbuffer DepthStencilState depthstencilstate RasterizerState rasterizerstate SamplerComparisonState samplercomparisonstate ")
        _T("SamplerState samplerstate Sampler_State sampler_state TBuffer tbuffer Technique technique Technique10 technique10 Pass pass depthstencilview rendertargetview ")
        _T("texture1 Texture1D Texture1DArray Texture2D Texture2DArray Texture2DMS Texture2DMSArray Texture3D TextureCube ")
        _T("TextureCubeArray");
    keyWords.push_back(typeKeyWords);

    KeyWords semanticsKeyWords;
    semanticsKeyWords.type = KW_Semantics;
    semanticsKeyWords.strKeywords =
        // For types where multiple instances are supported the number I've included is arbitrary in some cases
        _T("BINORMAL BINORMAL0 BINORMAL1 BINORMAL2 BINORMAL3 ")
        _T("BLENDINDICES BLENDINDICES0 BLENDINDICES1 BLENDINDICES2 BLENDINDICES3 BLENDINDICES4 BLENDINDICES5 BLENDINDICES6 BLENDINDICES7 ")
        _T("BLENDWEIGHT BLENDWEIGHT0 BLENDWEIGHT1 BLENDWEIGHT2 BLENDWEIGHT3 BLENDWEIGHT4 BLENDWEIGHT5 BLENDWEIGHT6 BLENDWEIGHT7 ")
        _T("COLOR COLOR0 COLOR1 COLOR2 COLOR3 COLOR4 COLOR5 COLOR6 COLOR7 ")
        _T("NORMAL NORMAL0 NORMAL POSITION POSITION0 POSITION1 POSITIONT PSIZE PSIZE0 PSIZE1 PSIZE2 PSIZE3 RTARRAYINDEX SV_InstanceID SV_POSITION SV_Target ")
        _T("TANGENT TANGENT0 TANGENT1 TANGENT2 TANGENT3 TESSFACTOR TESSFACTOR0 TESSFACTOR1 TESSFACTOR2 TESSFACTOR3 ")
        _T("TEXCOORD TEXCOORD0 TEXCOORD1 TEXCOORD2 TEXCOORD3 TEXCOORD4 TEXCOORD5 TEXCOORD6 TEXCOORD7 ")
        _T("VFACE VPOS DEPTH DEPTH0 DEPTH1 DEPTH2 DEPTH3 ")
        _T("WORLD WORLDVIEW WORLDVIEWPROJECTION VIEW PROJECTION ");
    keyWords.push_back(semanticsKeyWords);

    KeyWords reservedKeyWords;
    reservedKeyWords.type = KW_Reserved;
    reservedKeyWords.strKeywords =
        _T("auto case catch char class const_cast default delete dynamic_cast enum explicit friend goto long mutable new operator ")
        _T("private protected public reinterpret_cast short signed sizeof static_cast template this throw try typename union ")
        _T("unsigned using virtual ");
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
        _T("abs acos all any asfloat asin asint asuint atan atan2 ceil clamp clip cos cosh cross D3DCOLORtoUBYTE4 ddx ddy degrees ")
        _T("determinant distance dot exp exp2 faceforward floor fmod frac frexp fwidth GetRenderTargetSampleCount ")
        _T("GetRenderTargetSamplePosition isfinite isinf isnan ldexp length lerp lit log log10 log2 max min modf mul noise normalize ")
        _T("pow radians reflect refract round rsqrt saturate sign sin sincos sinh smoothstep sqrt step tan tanh tex1D tex1Dbias ")
        _T("tex1Dgrad tex1Dlod tex1Dproj tex2D tex2Dbias tex2Dgrad tex2Dlod tex2Dproj tex3D tex3Dbias tex3Dgrad tex3Dlod tex3Dproj ")
        _T("texCUBE texCUBEbias texCUBEgrad texCUBElod texCUBEproj transpose trunc ")
        _T("Append RestartStrip ")   // SM4 Stream-Output Object
        _T("CalculateLevelOfDetail CalculateLevelOfDetailUnclamped Gather GetDimensions GetSamplePosition Load Sample SampleBias ")   // SM4 Texture Object
        _T("SampleCmp SampleCmpLevelZero SampleGrad SampleLevel ")   // SM4 Texture Object
        _T("SetVertexShader SetGeometryShader SetPixelShader SetBlendState SetDepthStencilState SetRasterizerState SetRenderTargets CompileShader ConstructGSWithSO ")   // FX
        ;
    keyWords.push_back(functionKeyWords);

    // Disabling the code for gettting variable keywords for now since it appears that variables that are optimized out do not appear.
    // This looks wrong.

    //    const std::vector<std::string>& strTokens = shader.GetDebugStrTokens();
    //    size_t nTokens = strTokens.size();
    //    if(nTokens != 0)
    //    {
    //       std::set<std::string> strKeyWords;
    //
    // //       const std::vector<D3D10_SHADER_DEBUG_VAR_INFO>& variables = shader.GetDebugVariables();
    // //       BOOST_FOREACH(D3D10_SHADER_DEBUG_VAR_INFO var, variables)
    // //       {
    // //          if(var.TokenId < nTokens)
    // //          {
    // //             std::string strKeyWord = strTokens[var.TokenId];
    // //
    // //             size_t Pos;
    // //             while((Pos = strKeyWord.find("::")) != std::string::npos)
    // //             {
    // //                strKeyWord.replace(Pos, 2, ".");
    // //             }
    // //             strKeyWords.insert(strKeyWord);
    // //          }
    // //       }
    // //
    // //       KeyWords variableKeyWords;
    // //       variableKeyWords.type = KW_Variable;
    // //       BOOST_FOREACH(std::string strKeyWord, strKeyWords)
    // //       {
    // //          variableKeyWords.strKeywords += strKeyWord + " ";
    // //       }
    // //       keyWords.push_back(variableKeyWords);
    //
    //       KeyWords variableKeyWords;
    //       variableKeyWords.type = KW_Variable;
    //       BOOST_FOREACH(std::string strKeyWord, strTokens)
    //       {
    //          size_t Pos;
    //          while((Pos = strKeyWord.find("::")) != std::string::npos)
    //          {
    //             strKeyWord.replace(Pos, 2, ".");
    //          }
    //          variableKeyWords.strKeywords += strKeyWord + " ";
    //       }
    //
    //       keyWords.push_back(variableKeyWords);
    //    }

    return true;
}