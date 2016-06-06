//=====================================================================
//
// Author:      AMD Developer Tools Team
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// GLSLKeywords.cpp
// File contains <Seth file description here>
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/GLSLKeyWords.cpp#3 $
//
// Last checkin:  $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================
//   ( C ) AMD, Inc. 2009,2010 All rights reserved.
//=====================================================================

#include "GLSLKeywords.h"
#include <tchar.h>

bool ShaderUtils::GetGLSLKeyWords(ShaderUtils::KeyWordsList& keyWords)
{
    keyWords.clear();

    // NOTE: these keywords were generated based on the GLSL 1.3 spec. Deprecated keywords are included, as they have not
    // yet been removed from the language. When we move to GLSL 1.4 or 1.5, we will have to decide how to recognize the removed keywords

    // Key words
    ShaderUtils::KeyWords opKeyWords;
    opKeyWords.type = KW_Op;
    opKeyWords.strKeywords =
        _T("attribute const uniform varying centroid break continue do for while if else in out inout true false ")
        _T("invariant discard return sizeof cast namespace using flat smooth noperspective ")
        _T("lowp mediump highp precision switch case default ");

    //// Reserved words - these result in errors if you use them
    //_T("common partition active asm class union enum typedef template this packed goto inline noinline volatile public static extern external ")
    //_T("interface long short double half fixed unsigned superp input output hvec2 hvec3 hvec4 dvec2 dvec3 dvec4 fvec2 fvec3 fvec4 ")
    //_T("sampler2DRect sampler3DRect sampler2DRectShadow samplerBuffer filter image1D image2D image3D imageCube iimage1D iimage2D iimage3D ")
    //_T("iimageCube uimate1D uimage2D uimage3D uimageCube image1DArray image2DArray iimage1DArray iimage2DArray uimage1DArray uimage2DArray ")
    //_T("image1DShadow image2DShadow image1DArrayShadow image2DArrayShadow imageBuffer iimageBuffer uimageBuffer sizeof cast namespace uint row_major ");

    // Preprocessor stuff
    //_T("#define #if #elif #else #endif #error #if #ifdef #ifndef #include #line #pragma #undef ")
    //_T("pack_matrix row_major column_major warning type once default disable error defined")
    keyWords.push_back(opKeyWords);

    // Types
    ShaderUtils::KeyWords typeKeywords;
    typeKeywords.type = KW_Type;
    typeKeywords.strKeywords =
        _T("mat2 mat3 mat4 mat2x2 mat2x3 mat2x4 mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 ")
        _T("float int void bool vec2 vec3 vec4 ivec2 ivec3 ivec4 bvec2 bvec3 bvec4 struct uint uvec2 uvec3 uvec4")
        _T("sampler1D sampler2D sampler3D samplerCube sampler1DShadow sampler2DShadow samplerCubeShadow ")
        _T("sampler1DArray sampler2DArray sampler1DArrayShadow sampler2DArrayShadow ")
        _T("isampler1D isampler2D isampler3D isamplerCube isampler1DArray isampler2DArray ")
        _T("usampler1D usampler2D usampler3D usamplerCube usampler1DArray usampler2DArray ");
    keyWords.push_back(typeKeywords);

    // Functions
    ShaderUtils::KeyWords functionKeyWords;
    functionKeyWords.type = KW_Function;
    functionKeyWords.strKeywords =
        _T("abs acos all any asin atan ceil clamp cos cross degrees dFdx dFdy distance dot equal exp exp2 ")
        _T("faceforward floor fract ftransform fwidth greaterThan greaterThanEqual inversesqrt length lessThan lessThanEqual log log2 matrixCompMult max min mix mod")
        _T("noise1 noise2 noise3 noise4 normalize not notEqual pow radians reflect refract sign sin smoothstep ")
        _T("sqrt step tan sinh cosh tanh asinh acosh atanh trunc round roundEven isnan isinf ")
        _T("outerProduct transpose ");
    keyWords.push_back(functionKeyWords);

    // Textures
    ShaderUtils::KeyWords textureKeyWords;
    textureKeyWords.type = KW_TextureOp;
    textureKeyWords.strKeywords =
        _T("texture1D texture2D textureCube texture3D shadow1D shadow1DLod shadow1DProj shadow1DProjLod shadow2D shadow2DLod shadow2DProj shadow2DProjLod ")
        _T("texture1DLod texture1DProj texture1DProjLod texture2DLod texture2DProj texture2DProjLod texture3DLod texture3DProj texture3DProjLod textureCubeLod ")
        _T("textureSize texture textureProj textureLod textureOffset texelFetch texelFetchOffset textureProjOffset textureLodOffset textureProjLod ")
        _T("textureProjLodOffset textureGrad textureGradOffset textureProjGrad textureProjGradOffset ")
        _T("texture1DArray texture1DArrayLod texture2DArray texture2DArrayLod shader1DArray shadow1DArrayLod shadow2DArray ");
    keyWords.push_back(textureKeyWords);

    ShaderUtils::KeyWords variableKeyWords;
    variableKeyWords.type = KW_Variable;
    variableKeyWords.strKeywords =
        // uniforms / constants
        _T("gl_BackLightModelProduct glBackLightProduct glBackMaterial gl_ClipPlane gl_DepthRange gl_DepthRangeParameters gl_EyePlaneQ ")
        _T("gl_EyePlaneR gl_EyePlaneS gl_EyePlaneT gl_Fog gl_FogParameters gl_FrontLightModelProduct gl_FrontLightProduct gl_FrontMaterial ")
        _T("gl_LightModel gl_LightModelParameters gl_LightModelProducts gl_LightProducts gl_LightSource gl_LightSourceParameters gl_MaterialParameters ")
        _T("gl_MaxClipPlanes gl_MaxCombinedTextureImageUnits gl_MaxDrawBuffers gl_MaxFragmentUniformComponents gl_MaxLights gl_MaxTextureCoords ")
        _T("gl_MaxTextureImageUnits gl_MaxTextureUnits gl_MaxVaryingFloats gl_MaxVertexAttribs gl_MaxVertexTextureImageUnits gl_MaxVertexUniformComponents ")
        _T("gl_ModelViewMatrix gl_ModelViewMatrixInverse gl_ModelViewMatrixInverseTranspose gl_ModelViewMatrixTranspose gl_ModelViewProjectionMatrix ")
        _T("gl_ModelViewProjectionMatrixInverse gl_ModelViewProjectionMatrixInverseTranspose gl_ModelViewProjectionMatrixTranspose gl_NormalMatrix gl_NormalScale ")
        _T("gl_ObjectPlaneQ gl_ObjectPlaneR gl_ObjectPlaneS gl_ObjectPlaneT gl_Point gl_PointParameters gl_ProjectionMatrix gl_ProjectionMatrixInverse ")
        _T("gl_ProjectionMatrixInverseTranspose gl_ProjectionMatrixTranspose gl_TextureEnvColor gl_TextureMatrix gl_TextureMatrixInverse gl_TextureMatrixInverseTranspose gl_TextureMatrixTranspose ")
        // varyings / specials
        _T("gl_BackColor gl_BackSecondaryColor gl_ClipVertex gl_Color gl_FogCoord gl_FogFragCoord gl_FragColor gl_FragData gl_FragDepth ")
        _T("gl_FrontColor gl_FrontFacing gl_FrontSecondaryColor gl_MultiTexCoord0 gl_MultiTexCoord1 gl_MultiTexCoord2 gl_MultiTexCoord3 ")
        _T("gl_MultiTexCoord4 gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_Normal gl_PointSize gl_Position gl_SecondaryColor ")
        _T("gl_TexCoord gl_Vertex ");
    keyWords.push_back(variableKeyWords);

    return true;
}
