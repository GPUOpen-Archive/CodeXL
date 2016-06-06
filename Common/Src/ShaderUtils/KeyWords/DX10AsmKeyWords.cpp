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
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX10AsmKeyWords.cpp#3 $
//
// Last checkin:  $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================
//   ( C ) AMD, Inc. 2009,2010 All rights reserved.
//=====================================================================

#include "DX10AsmKeywords.h"
#include <tchar.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include "D3D10ShaderUtils.h"
#include "Swizzles.h"

using namespace D3D10ShaderUtils;

using boost::format;

bool ShaderUtils::GetDX10InstKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader)
{
    // Ops
    KeyWords opKeyWords;
    opKeyWords.type = KW_Op;
    opKeyWords.strKeywords = "constant linear centroid linearCentroid linearNoperspective linearNoperspectiveCentroid dynamicIndexed immediateIndexed mode_default float noperspective position refactoringAllowed  "
                             "ClipDistance CullDistance Coverage Depth IsFrontFace Position RenderTargetArrayIndex SampleIndex Target ViewportArrayIndex InstanceID PrimitiveID VertexID "
                             "Buffer Texture1D Texture1DArray Texture2D Texture2DArray Texture3D TextureCube Texture2DMS Texture2DMSArray TextureCubeArray "
                             "buffer texture1d texture1darray texture2d texture2darray texture3d texturecube texture2dms texture2dmsarray texturecubearray "
                             "UNORM SNORM SINT UINT FLOAT unorm snorm sint uint float default mode_default comparison mode_comparison mono mode_mono ";

    KeyWords texOpKeyWords;
    texOpKeyWords.type = KW_TextureOp;
    texOpKeyWords.strKeywords = "";

    KeyWords intOpKeyWords;
    intOpKeyWords.type = KW_IntOp;
    intOpKeyWords.strKeywords = "";

    KeyWords doubleOpKeyWords;
    doubleOpKeyWords.type = KW_DoubleOp;
    doubleOpKeyWords.strKeywords = "";

    D3D10ShaderUtils::ShaderType shaderType = D3D10ShaderUtils::None;

    switch (shader.GetShaderType())
    {
        case D3D10_SB_VERTEX_SHADER:
            opKeyWords.strKeywords += "vs_4_0 vs_4_1 vs_5_0 dcl_input_vertexID dcl_input_instanceID instance_id ";
            shaderType = VS;
            break;

        case D3D10_SB_GEOMETRY_SHADER:
            opKeyWords.strKeywords += "gs_4_0 gs_4_1 gs_5_0 dcl_input_primitiveID dcl_output_primitiveID dcl_output_isFrontFace dcl_maxout rendertarget_array_index ";
            shaderType = GS;
            break;

        case D3D10_SB_PIXEL_SHADER:
            opKeyWords.strKeywords += "ps_4_0 ps_4_1 ps_5_0 dcl_input_ps dcl_input_primitiveID dcl_input_isFrontFace dcl_input_clipDistance  dcl_input_cullDistance dcl_input_position dcl_input_renderTargetArrayIndex  dcl_input_viewportArrayIndex ";
            shaderType = PS;
            break;

        case D3D11_SB_HULL_SHADER:
            opKeyWords.strKeywords += "hs_5_0 ";
            shaderType = HS;
            break;

        case D3D11_SB_DOMAIN_SHADER:
            opKeyWords.strKeywords += "ds_5_0 ";
            shaderType = DS;
            break;

        case D3D11_SB_COMPUTE_SHADER:
            opKeyWords.strKeywords += "cs_4_0 cs_4_1 cs_5_0 ";
            shaderType = CS;
            break;
    }

    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfoTable();

    for (DWORD i = 0; i < GetOpCodeInfoCount(); i++)
    {
        if ((pOpCodeInfo[i].shaderTypes & shaderType) != 0)
        {
            switch (pOpCodeInfo[i].instructionType)
            {
                case IT_Integer:     // Fall-through
                case IT_Atomic:      intOpKeyWords.strKeywords += pOpCodeInfo[i].pszKeyWords; intOpKeyWords.strKeywords += _T(" "); break;

                case IT_Double:      doubleOpKeyWords.strKeywords += pOpCodeInfo[i].pszKeyWords; doubleOpKeyWords.strKeywords += _T(" "); break;

                case IT_Load:        // Fall-through
                case IT_Store:       texOpKeyWords.strKeywords += pOpCodeInfo[i].pszKeyWords; texOpKeyWords.strKeywords += _T(" "); break;

                case IT_Declaration: // Fall-through
                case IT_ALU:         // Fall-through
                case IT_FlowControl: // Fall-through
                case IT_Other:       // Fall-through
                default:             opKeyWords.strKeywords += pOpCodeInfo[i].pszKeyWords; opKeyWords.strKeywords += _T(" "); break;
            }
        }
    }

    keyWords.push_back(opKeyWords);
    keyWords.push_back(texOpKeyWords);
    keyWords.push_back(intOpKeyWords);
    keyWords.push_back(doubleOpKeyWords);
    return !keyWords.empty();
}

const std::string GenerateRegKeyWords(const TCHAR* pszFormat, DWORD dwCount)
{
    std::string strRegKeyWords;

    for (DWORD i = 0; i < dwCount; i++)
    {
        strRegKeyWords += str(format(pszFormat) % i);
    }

    return strRegKeyWords;
}

const std::string GenerateArrayRegKeyWords(const TCHAR* pszFormat, DWORD dwRegIndex, DWORD dwArraySize)
{
    std::string strRegKeyWords;

    for (DWORD i = 0; i < dwArraySize; i++)
    {
        strRegKeyWords += str(format(pszFormat) % dwRegIndex % i);
    }

    return strRegKeyWords;
}

const std::string GenerateRegKeyWords(const TCHAR* pszFormat, DWORD dwStart, DWORD dwCount)
{
    std::string strRegKeyWords;

    for (DWORD i = dwStart; i < dwStart + dwCount; i++)
    {
        strRegKeyWords += str(format(pszFormat) % i);
    }

    return strRegKeyWords;
}

bool ShaderUtils::GetDX10RegKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader)
{
    KeyWords regKeyWords;
    regKeyWords.type = KW_Reg;
    regKeyWords.strKeywords = _T("null l icb ") + GenerateRegKeyWords(_T("icb%i "), shader.GetImmediateConstantCount()) +
                              GenerateRegKeyWords(_T("r%i "), shader.GetStats().TempRegisterCount);

    if (shader.GetShaderModel() >= SM_5_0)
    {
        regKeyWords.strKeywords += "this ";
    }

    if (shader.GetShaderType() == D3D11_SB_COMPUTE_SHADER)
    {
        regKeyWords.strKeywords += "vThreadID vThreadGroupID vThreadIDInGroupFlattened vThreadIDInGroup ";
    }

    BOOST_FOREACH(D3D10ShaderObject::D3D10_ResourceBinding resource, shader.GetBoundResources())
    {
        if (resource.Type == D3D10_SIT_CBUFFER)
        {
            regKeyWords.strKeywords += GenerateRegKeyWords(_T("cb%i "), resource.dwBindPoint, resource.dwBindCount);
        }
        else if (resource.Type == D3D10_SIT_SAMPLER)
        {
            regKeyWords.strKeywords += GenerateRegKeyWords(_T("s%i "), resource.dwBindPoint, resource.dwBindCount);
        }
        else if (resource.Type == D3D10_SIT_TBUFFER || resource.Type == D3D10_SIT_TEXTURE || resource.Type == D3D11_SIT_STRUCTURED ||
                 resource.Type == D3D11_SIT_BYTEADDRESS || resource.Type == D3D11_SIT_UAV_CONSUME_STRUCTURED)
        {
            regKeyWords.strKeywords += GenerateRegKeyWords(_T("t%i "), resource.dwBindPoint, resource.dwBindCount);
        }
        else if (resource.Type == D3D11_SIT_UAV_RWTYPED || resource.Type == D3D11_SIT_UAV_RWSTRUCTURED || resource.Type == D3D11_SIT_UAV_RWBYTEADDRESS ||
                 resource.Type == D3D11_SIT_UAV_APPEND_STRUCTURED || resource.Type == D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
        {
            regKeyWords.strKeywords += GenerateRegKeyWords(_T("t%i "), resource.dwBindPoint, resource.dwBindCount) + GenerateRegKeyWords(_T("u%i "), resource.dwBindPoint, resource.dwBindCount);
        }
        else if (resource.Type == D3D11_SIT_STRUCTURED)
        {
            regKeyWords.strKeywords += GenerateRegKeyWords(_T("g%i "), resource.dwBindPoint, resource.dwBindCount);
        }
        else
        {
            regKeyWords.strKeywords += GenerateRegKeyWords(_T("u%i "), resource.dwBindPoint, resource.dwBindCount);
        }
    }

    regKeyWords.strKeywords += "v ";
    BOOST_FOREACH(D3D10ShaderObject::D3D10_Signature inputSignature, shader.GetInputSignatures())
    {
        regKeyWords.strKeywords += str(format(_T("v%i ")) % inputSignature.dwRegister);
    }

    BOOST_FOREACH(std::string strInputSemanticName, shader.GetInputSemanticNames())
    {
        regKeyWords.strKeywords += strInputSemanticName + " ";
    }

    BOOST_FOREACH(D3D10ShaderObject::D3D10_Signature outputSignature, shader.GetOutputSignatures())
    {
        regKeyWords.strKeywords += str(format(_T("o%i ")) % outputSignature.dwRegister);
    }

    // Just hack in oDepth & oMask for all pixel shaders for now
    if (shader.GetShaderType() == D3D10_SB_PIXEL_SHADER)
    {
        regKeyWords.strKeywords += "oDepth oMask ";
    }

    BOOST_FOREACH(std::string strOutputSemanticName, shader.GetOutputSemanticNames())
    {
        regKeyWords.strKeywords += strOutputSemanticName + " ";
    }

    BOOST_FOREACH(D3D10ShaderObject::D3D10_IndexableTempRegister indexableTempRegister, shader.GetIndexableTempRegisters())
    {
        regKeyWords.strKeywords += str(format(_T("x%i ")) % indexableTempRegister.dwIndex);
    }

    BOOST_FOREACH(D3D10ShaderObject::D3D10_GlobalMemoryRegister globalMemoryRegister, shader.GetGlobalMemoryRegisters())
    {
        regKeyWords.strKeywords += str(format(_T("g%i ")) % globalMemoryRegister.dwIndex);
    }

    regKeyWords.strKeywords += GetRGBASwizzles() + GetXYZWSwizzles();

    keyWords.push_back(regKeyWords);

    return true;
}
