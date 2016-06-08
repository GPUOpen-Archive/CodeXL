//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc.
//
/// \author AMD Developer Tools Team
/// \file D3D10ShaderInfo.cpp
///
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10ShaderInfo.cpp#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author
//=====================================================================

#include <Windows.h>
#include <assert.h>
#include <boost/format.hpp>
#include <D3D10_1.h>
#include "D3D10ShaderInfo.h"
#include "D3D10ShaderUtils.h"

using boost::format;

const char* pszRegSet[] =
{
    "INPUT",
    "OUTPUT",
    "CBUFFER",
    "TBUFFER",
    "TEMP",
    "TEMPARRAY",
    "TEXTURE",
    "SAMPLER",
    "IMMEDIATECBUFFER",
    "LITERAL",
    "UNUSED",
};

const char* pszRegType[] =
{
    "INPUT",
    "OUTPUT",
    "CBUFFER",
    "TBUFFER",
    "TEMP",
    "TEMPARRAY",
    "TEXTURE",
    "SAMPLER",
    "IMMEDIATECBUFFER",
    "LITERAL",
    "UNUSED",
};

const char* pszVarType[] =
{
    "VOID",
    "BOOL ",
    "INT",
    "FLOAT",
    "STRING",
    "TEXTURE",
    "TEXTURE1D",
    "TEXTURE2D",
    "TEXTURE3D",
    "TEXTURECUBE",
    "SAMPLER",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "PIXELSHADER",
    "VERTEXSHADER",
    "UNKNOWN",
    "UNKNOWN",
    "UINT",
    "UINT8",
    "GEOMETRYSHADER",
    "RASTERIZER",
    "DEPTHSTENCIL",
    "BLEND",
    "BUFFER",
    "CBUFFER",
    "TBUFFER",
    "TEXTURE1DARRAY",
    "TEXTURE2DARRAY",
    "RENDERTARGETVIEW",
    "DEPTHSTENCILVIEW",
    "TEXTURE2DMS",
    "TEXTURE2DMSARRAY",
    "TEXTURECUBEARRAY"
};

const char* pszDebugVarType[] =
{
    "VARIABLE",
    "FUNCTION",
};

const char* pszVarClass[] =
{
    "SCALAR",
    "VECTOR",
    "MATRIX_ROWS",
    "MATRIX_COLUMNS",
    "OBJECT",
    "STRUCT",
};

const char* pszScopeType[] =
{
    "GLOBAL",
    "BLOCK",
    "FORLOOP",
    "STRUCT",
    "FUNC_PARAMS",
    "STATEBLOCK",
    "NAMESPACE",
    "ANNOTATION",
};

const char* D3D10ShaderInfo::GetVarType(D3D10_SHADER_VARIABLE_TYPE type)
{
    return pszVarType[ type ];
}

std::string& PrintTokenInfo(const D3D10_SHADER_DEBUG_INFO* pDebugInfo, UINT nToken, std::string& strTokenInfo)
{
    if (nToken >= pDebugInfo->Tokens)
    {
        strTokenInfo = "";
        return strTokenInfo;
    }

    char* pszDebugInfo = (char*) pDebugInfo;
    char* pszDebugDataOffset = pszDebugInfo + pDebugInfo->Size;
    char* pszStringTable = pszDebugDataOffset +  pDebugInfo->StringOffset;
    D3D10_SHADER_DEBUG_FILE_INFO* pFileInfo = (D3D10_SHADER_DEBUG_FILE_INFO*)(pszDebugDataOffset + pDebugInfo->FileInfo);
    D3D10_SHADER_DEBUG_TOKEN_INFO* pTokenInfo = (D3D10_SHADER_DEBUG_TOKEN_INFO*)(pszDebugDataOffset + pDebugInfo->TokenInfo);

    std::string strTokenName, strTokenFile;

    if (pTokenInfo[nToken].TokenLength)
    {
        strTokenName = std::string((pszStringTable + pTokenInfo[nToken].TokenId), pTokenInfo[nToken].TokenLength);
    }
    else
    {
        strTokenName = str(format("Token %i") % pTokenInfo[nToken].TokenId);
    }

    strTokenFile = "";

    if (pTokenInfo[nToken].File < pDebugInfo->Files)
    {
        if (pFileInfo[pTokenInfo[nToken].File].FileNameLen)
        {
            strTokenFile = std::string((pszStringTable + pFileInfo->FileName), pFileInfo[pTokenInfo[nToken].File].FileNameLen);
        }
    }

    strTokenInfo = str(format("%s (\"%s\", Line %i, Column%i)") % strTokenName % strTokenFile % pTokenInfo[nToken].Line % pTokenInfo[nToken].Column);

    return strTokenInfo;
}

const std::string& PrintVariableInfo(const D3D10_SHADER_DEBUG_INFO* pDebugInfo, UINT nVariable, std::string& strVariableInfo)
{
    if (nVariable >= pDebugInfo->Variables)
    {
        strVariableInfo = "";
        return strVariableInfo;
    }

    char* pszDebugInfo = (char*) pDebugInfo;
    char* pszDebugDataOffset = pszDebugInfo + pDebugInfo->Size;
    //   char* pszStringTable = pszDebugDataOffset +  pDebugInfo->StringOffset;
    D3D10_SHADER_DEBUG_VAR_INFO* pVariableInfo = (D3D10_SHADER_DEBUG_VAR_INFO*)(pszDebugDataOffset + pDebugInfo->VariableInfo + (nVariable * sizeof(D3D10_SHADER_DEBUG_VAR_INFO)));

    std::string strTokenInfo;
    strVariableInfo = str(format("%s (%s)") % PrintTokenInfo(pDebugInfo, pVariableInfo->TokenId, strTokenInfo) % pszVarType[pVariableInfo->Type]);

    return strVariableInfo;
}

std::string& PrintScopeVariableInfo(const D3D10_SHADER_DEBUG_INFO* pDebugInfo, UINT nVariable, std::string& strVariableInfo)
{
    if (nVariable >= pDebugInfo->Variables)
    {
        strVariableInfo = "";
        return strVariableInfo;
    }

    char* pszDebugInfo = (char*) pDebugInfo;
    char* pszDebugDataOffset = pszDebugInfo + pDebugInfo->Size;
    //   char* pszStringTable = pszDebugDataOffset +  pDebugInfo->StringOffset;
    D3D10_SHADER_DEBUG_SCOPEVAR_INFO* pVariableInfo = (D3D10_SHADER_DEBUG_SCOPEVAR_INFO*)(pszDebugDataOffset + pDebugInfo->ScopeVariableInfo + (nVariable * sizeof(D3D10_SHADER_DEBUG_SCOPEVAR_INFO)));

    std::string strTokenInfo;
    strVariableInfo = str(format("%s (%s, %s)") % PrintTokenInfo(pDebugInfo, pVariableInfo->TokenId, strTokenInfo) % pszDebugVarType[pVariableInfo->VarType] % pszVarClass[pVariableInfo->Class]);

    return strVariableInfo;
}

std::string& PrintScopeInfo(const D3D10_SHADER_DEBUG_INFO* pDebugInfo, UINT nScope, std::string& strScopeInfo)
{
    if (nScope >= pDebugInfo->Scopes)
    {
        strScopeInfo = "";
        return strScopeInfo;
    }

    char* pszDebugInfo = (char*) pDebugInfo;
    char* pszDebugDataOffset = pszDebugInfo + pDebugInfo->Size;
    char* pszStringTable = pszDebugDataOffset +  pDebugInfo->StringOffset;
    D3D10_SHADER_DEBUG_SCOPE_INFO* pScopeInfo = (D3D10_SHADER_DEBUG_SCOPE_INFO*)(pszDebugDataOffset + pDebugInfo->ScopeInfo + (nScope * sizeof(D3D10_SHADER_DEBUG_SCOPE_INFO)));

    std::string strScopeName((pszStringTable + pScopeInfo->Name), pScopeInfo->uNameLen);

    strScopeInfo = str(format("%s (%s, %i Variables)") %  strScopeName  % pszScopeType[pScopeInfo->ScopeType] % pScopeInfo->uVariables);

    return strScopeInfo;
}

bool D3D10ShaderInfo::DumpDX10ShaderDebugInfo(const D3D10ShaderObject::D3D10_ChunkHeader* pSDBGChunk, std::string& strDebugInfo)
{
    if (pSDBGChunk == NULL)
    {
        return false;
    }

    char* pszDebugInfo = (char*)(pSDBGChunk + 1);
    D3D10_SHADER_DEBUG_INFO* pDebugInfo = (D3D10_SHADER_DEBUG_INFO*)(pszDebugInfo);
    char* pszDebugDataOffset = pszDebugInfo + pDebugInfo->Size;
    char* pszStringTable = pszDebugDataOffset +  pDebugInfo->StringOffset;
    UINT* puiUintTable = (UINT*)(pszDebugDataOffset +  pDebugInfo->UintOffset);

    D3D10_SHADER_DEBUG_FILE_INFO* pFileInfo = (D3D10_SHADER_DEBUG_FILE_INFO*)(pszDebugDataOffset + pDebugInfo->FileInfo);
    D3D10_SHADER_DEBUG_INST_INFO* pInstructionInfo = (D3D10_SHADER_DEBUG_INST_INFO*)(pszDebugDataOffset + pDebugInfo->InstructionInfo);
    D3D10_SHADER_DEBUG_VAR_INFO* pVariableInfo = (D3D10_SHADER_DEBUG_VAR_INFO*)(pszDebugDataOffset + pDebugInfo->VariableInfo);
    D3D10_SHADER_DEBUG_INPUT_INFO* pInputVariableInfo = (D3D10_SHADER_DEBUG_INPUT_INFO*)(pszDebugDataOffset + pDebugInfo->InputVariableInfo);
    D3D10_SHADER_DEBUG_TOKEN_INFO* pTokenInfo = (D3D10_SHADER_DEBUG_TOKEN_INFO*)(pszDebugDataOffset + pDebugInfo->TokenInfo);
    D3D10_SHADER_DEBUG_SCOPE_INFO* pScopeInfo = (D3D10_SHADER_DEBUG_SCOPE_INFO*)(pszDebugDataOffset + pDebugInfo->ScopeInfo);
    D3D10_SHADER_DEBUG_SCOPEVAR_INFO* pScopeVariableInfo = (D3D10_SHADER_DEBUG_SCOPEVAR_INFO*)(pszDebugDataOffset + pDebugInfo->ScopeVariableInfo);

    std::string strLine, strTemp;
    strDebugInfo += "ShaderDebugInfo\n";
    strLine = str(format("Size = %i\n") % pDebugInfo->Size);
    strDebugInfo += strLine;
    strLine = str(format("Creator = \"%s\"\n") % (pszStringTable + pDebugInfo->Creator));
    strDebugInfo += strLine;
    strLine = str(format("EntrypointName = \"%s\"\n") % (pszStringTable + pDebugInfo->EntrypointName));
    strDebugInfo += strLine;
    strLine = str(format("ShaderTarget = \"%s\"\n") % (pszStringTable + pDebugInfo->ShaderTarget));
    strDebugInfo += strLine;
    strLine = str(format("CompileFlags = %08x\n") % pDebugInfo->CompileFlags);
    strDebugInfo += strLine;

    strLine = str(format("Files = %i\n") % pDebugInfo->Files);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->Files; i++)
    {
        strLine = str(format("\tFile %i\n") % i);
        strDebugInfo += strLine;

        if (pFileInfo[i].FileNameLen)
        {
            strLine = str(format("\t\tFileName = \"%s\"\n") % (pszStringTable + pFileInfo[i].FileName));
        }
        else
        {
            strLine = "\t\tFileName = NULL\n";
        }

        strLine = str(format("\t\tFileLen = %i\n") % pFileInfo[i].FileLen);
        strDebugInfo += strLine;
    }

    strLine = str(format("Instructions = %i\n") % pDebugInfo->Instructions);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->Instructions; i++)
    {

        strLine = str(format("\tInstruction %i\n") % i);
        strDebugInfo += strLine;
        strLine = str(format("\t\tId = %i\n") % pInstructionInfo[i].Id);
        strDebugInfo += strLine;
        strLine = str(format("\t\tOp = %s\n") % D3D10ShaderUtils::GetOpCodeName(DECODE_D3D10_SB_OPCODE_TYPE(pInstructionInfo[i].Opcode)));
        strDebugInfo += strLine;

        strLine = str(format("\t\tuOutputs = %i\n") % pInstructionInfo[i].uOutputs);
        strDebugInfo += strLine;

        for (UINT j = 0; j < pInstructionInfo[i].uOutputs; j++)
        {
            strLine = str(format("\t\t\tOutput %i\n") % j);
            strDebugInfo += strLine;
            strLine = str(format("\t\t\t\tOutputRegisterSet = %s\n") % pszRegSet[pInstructionInfo[i].pOutputs[j].OutputRegisterSet]);
            strDebugInfo += strLine;
            strLine = str(format("\t\t\t\tOutputReg = %i\n") % pInstructionInfo[i].pOutputs[j].OutputReg);
            strDebugInfo += strLine;
            strLine = str(format("\t\t\t\tTempArrayReg = %i\n") % pInstructionInfo[i].pOutputs[j].TempArrayReg);
            strDebugInfo += strLine;
            strLine = str(format("\t\t\t\tOutputComponents = {%i, %i, %i, %i}\n") % pInstructionInfo[i].pOutputs[j].OutputComponents[0] % pInstructionInfo[i].pOutputs[j].OutputComponents[1] % pInstructionInfo[i].pOutputs[j].OutputComponents[2] % pInstructionInfo[i].pOutputs[j].OutputComponents[3]);
            strDebugInfo += strLine;

            for (UINT k = 0; k < 4; k++)
            {
                UINT nVar = 0;
                //                     if(pInstructionInfo[i].pOutputs[j].OutputReg == -1)
                //                     {
                //                     }
                //                     else
                nVar = pInstructionInfo[i].pOutputs[j].OutputVars[k].Var;

                if (nVar != 0xffffffff)
                {
                    std::string strVariableInfo("");
                    PrintVariableInfo(pDebugInfo, nVar, strVariableInfo);

                    strLine = str(format("\t\t\t\t\tVar %i = %i - %s\n") % k % nVar % strVariableInfo);
                    strDebugInfo += strLine;

                    if (pVariableInfo[nVar].Type == D3D10_SVT_UINT)
                    {
                        strLine = str(format("\t\t\t\t\t\tUint Range = %u - %u\n") % pInstructionInfo[i].pOutputs[j].OutputVars[k].uValueMin % pInstructionInfo[i].pOutputs[j].OutputVars[k].uValueMax);
                        strDebugInfo += strLine;
                    }
                    else if (pVariableInfo[nVar].Type == D3D10_SVT_INT)
                    {
                        strLine = str(format("\t\t\t\t\t\tInt Range = %i - %i\n") % pInstructionInfo[i].pOutputs[j].OutputVars[k].iValueMin % pInstructionInfo[i].pOutputs[j].OutputVars[k].iValueMax);
                        strDebugInfo += strLine;
                    }
                    else if (pVariableInfo[nVar].Type == D3D10_SVT_FLOAT)
                    {
                        strLine = str(format("\t\t\t\t\t\tFloat Range = %f - %f\n\t\t\t\t\t\tbNaNPossible = %s, bInfPossible = %s\n") %
                                      pInstructionInfo[i].pOutputs[j].OutputVars[k].fValueMin % pInstructionInfo[i].pOutputs[j].OutputVars[k].fValueMax %
                                      (pInstructionInfo[i].pOutputs[j].OutputVars[k].bNaNPossible ? "TRUE" : "FALSE") % (pInstructionInfo[i].pOutputs[j].OutputVars[k].bInfPossible ? "TRUE" : "FALSE"));
                        strDebugInfo += strLine;
                    }
                }
                else
                {
                    strLine = str(format("\t\t\t\t\tVar %i = %i\n") % k % nVar);
                    strDebugInfo += strLine;
                }
            }

            strLine = str(format("\t\t\t\tIndexReg = %i\n") % pInstructionInfo[i].pOutputs[j].IndexReg);
            strDebugInfo += strLine;
            strLine = str(format("\t\t\t\tIndexComp = %i\n") % pInstructionInfo[i].pOutputs[j].IndexComp);
            strDebugInfo += strLine;
        }

        strLine = str(format("\t\tToken = %s\n") % PrintTokenInfo(pDebugInfo, pInstructionInfo[i].TokenId, strTemp));
        strDebugInfo += strLine;
        strLine = str(format("\t\tuNestingLevel = %i\n") % pInstructionInfo[i].NestingLevel);
        strDebugInfo += strLine;

        strLine = str(format("\t\tScopes = %i\n") % pInstructionInfo[i].Scopes);
        strDebugInfo += strLine;
        UINT* pScopeNum = (UINT*)(((char*) puiUintTable) + pInstructionInfo[i].ScopeInfo);

        for (UINT j = 0; j < pInstructionInfo[i].Scopes; j++)
        {
            std::string strScopeInfo;
            PrintScopeInfo(pDebugInfo, *pScopeNum, strScopeInfo);

            strLine = str(format("\t\t\tScope %i = %i - %s\n") % j % *pScopeNum % strScopeInfo);
            strDebugInfo += strLine;
            pScopeNum++;
        }

        strLine = str(format("\t\tAccessedVars = %i\n") % pInstructionInfo[i].AccessedVars);
        strDebugInfo += strLine;
        UINT* pAccessedVarNum = (UINT*)(((char*) puiUintTable) + pInstructionInfo[i].AccessedVarsInfo);

        for (UINT j = 0; j < pInstructionInfo[i].AccessedVars; j++)
        {
            std::string strScopeVarInfo;
            PrintScopeVariableInfo(pDebugInfo, *pAccessedVarNum, strScopeVarInfo);

            strLine = str(format("\t\t\tAccessedVars %i = %i - %s\n") % j % *pAccessedVarNum % strScopeVarInfo);
            strDebugInfo += strLine;
            *pAccessedVarNum++;
        }
    }

    strLine = str(format("Variables = %i\n") % pDebugInfo->Variables);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->Variables; i++)
    {
        std::string strTokenInfo;
        PrintTokenInfo(pDebugInfo, pVariableInfo[i].TokenId, strTokenInfo);

        strLine = str(format("\tVariable %i\n") % i);
        strDebugInfo += strLine;
        strLine = str(format("\t\tToken = %s\n") % strTokenInfo);
        strDebugInfo += strLine;
        strLine = str(format("\t\tType = %s\n") % pszVarType[pVariableInfo[i].Type]);
        strDebugInfo += strLine;

        if (pVariableInfo[i].Type == D3D10_SVT_TEXTURE1DARRAY || pVariableInfo[i].Type == D3D10_SVT_TEXTURE2DARRAY ||
            pVariableInfo[i].Type == D3D10_SVT_TEXTURE2DMSARRAY || pVariableInfo[i].Type == D3D10_SVT_TEXTURECUBEARRAY)
        {
            strLine = str(format("\t\tRegister = %i\n") % pVariableInfo[i].Register);
            strDebugInfo += strLine;
            strLine = str(format("\t\tComponent = %i\n") % pVariableInfo[i].Component);
            strDebugInfo += strLine;
        }

        strLine = str(format("\t\tScopeVar = %i\n") % pVariableInfo[i].ScopeVar);
        strDebugInfo += strLine;
        strLine = str(format("\t\tScopeVarOffset = %i\n") % pVariableInfo[i].ScopeVarOffset);
        strDebugInfo += strLine;
    }

    strLine = str(format("InputVariables = %i\n") % pDebugInfo->InputVariables);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->InputVariables; i++)
    {
        std::string strTokenInfo;
        PrintTokenInfo(pDebugInfo, pVariableInfo[pInputVariableInfo[i].Var].TokenId, strTokenInfo);

        strLine = str(format("\tInputVariable %i\n") % i);
        strDebugInfo += strLine;
        strLine = str(format("\t\tVar = %s\n") % strTokenInfo);
        strDebugInfo += strLine;
        strLine = str(format("\t\tInitialRegisterSet = %s\n") % pszRegSet[pInputVariableInfo[i].InitialRegisterSet]);
        strDebugInfo += strLine;

        if (pInputVariableInfo[i].InitialBank != -1)
        {
            strLine = str(format("\t\tInitialBank = %i\n") % pInputVariableInfo[i].InitialBank);
            strDebugInfo += strLine;
        }

        if (pInputVariableInfo[i].InitialRegister != -1)
        {
            strLine = str(format("\t\tInitialRegister = %i\n") % pInputVariableInfo[i].InitialRegister);
            strDebugInfo += strLine;
        }

        if (pInputVariableInfo[i].InitialComponent != -1)
        {
            strLine = str(format("\t\tInitialComponent = %i\n") % pInputVariableInfo[i].InitialComponent);
            strDebugInfo += strLine;
        }

        if (pInputVariableInfo[i].InitialRegisterSet == D3D10_SHADER_DEBUG_REG_LITERAL)
        {
            strLine = str(format("\t\tInitialValue = %i\n") % pInputVariableInfo[i].InitialValue);
            strDebugInfo += strLine;
        }
    }

    strLine = str(format("Tokens = %i\n") % pDebugInfo->Tokens);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->Tokens; i++)
    {
        strLine = str(format("\tToken %i\n") % i);
        strDebugInfo += strLine;
        strLine = str(format("\t\tFile = %i\n") % pTokenInfo[i].File);
        strDebugInfo += strLine;
        strLine = str(format("\t\tLine = %i\n") % pTokenInfo[i].Line);
        strDebugInfo += strLine;
        strLine = str(format("\t\tColumn = %i\n") % pTokenInfo[i].Column);
        strDebugInfo += strLine;
        strLine = str(format("\t\tTokenLength = %i\n") % pTokenInfo[i].TokenLength);
        strDebugInfo += strLine;

        std::string strTokenName;

        if (pTokenInfo[i].TokenLength)
        {
            strTokenName = std::string((pszStringTable + pTokenInfo[i].TokenId), pTokenInfo[i].TokenLength);
        }
        else
        {
            strTokenName = "NULL";
        }

        strLine = str(format("\t\tTokenId = %s\n") % strTokenName);
        strDebugInfo += strLine;
    }

    strLine = str(format("Scopes = %i\n") % pDebugInfo->Scopes);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->Scopes; i++)
    {
        strLine = str(format("\tScope %i\n") % i);
        strDebugInfo += strLine;
        strLine = str(format("\t\tScopeType = %s\n") % pszScopeType[pScopeInfo[i].ScopeType]);
        strDebugInfo += strLine;
        strLine = str(format("\t\tuName = %s\n") % std::string((pszStringTable + pScopeInfo[i].Name), pScopeInfo[i].uNameLen));
        strDebugInfo += strLine;

        strLine = str(format("\t\tuVariables = %i\n") % pScopeInfo[i].uVariables);
        strDebugInfo += strLine;
        strLine = str(format("\t\tuVariableData = %i\n") % pScopeInfo[i].VariableData);
        strDebugInfo += strLine;
        //             for(UINT j = 0; j < pScopeInfo[i].uVariables; j++)
        //             {
        //                 strLine = str(format("\t\t\tVariable %i = %s\n") % j ,PrintVariableInfo(pScopeInfo[i], puiUintTable[j], strTemp));
        //                 strDebugInfo += strLine;
        //             }
    }

    strLine = str(format("ScopeVariables = %i\n") % pDebugInfo->ScopeVariables);
    strDebugInfo += strLine;

    for (UINT i = 0; i < pDebugInfo->ScopeVariables; i++)
    {
        std::string strTokenInfo;
        PrintTokenInfo(pDebugInfo, pScopeVariableInfo[i].TokenId, strTokenInfo);

        strLine = str(format("\tScopeVariable %i\n") % i);
        strDebugInfo += strLine;
        strLine = str(format("\t\tToken = %s\n") % PrintTokenInfo(pDebugInfo, pScopeVariableInfo[i].TokenId, strTemp));
        strDebugInfo += strLine;
        strLine = str(format("\t\tVarType = %s\n") % pszDebugVarType[pScopeVariableInfo[i].VarType]);
        strDebugInfo += strLine;
        strLine = str(format("\t\tClass = %s\n") % pszVarClass[pScopeVariableInfo[i].Class]);
        strDebugInfo += strLine;
        strLine = str(format("\t\tRows = %i\n") % pScopeVariableInfo[i].Rows);
        strDebugInfo += strLine;
        strLine = str(format("\t\tColumns = %i\n") % pScopeVariableInfo[i].Columns);
        strDebugInfo += strLine;
        strLine = str(format("\t\tStructMemberScope = %i\n") % pScopeVariableInfo[i].StructMemberScope);
        strDebugInfo += strLine;

        if (pScopeVariableInfo[i].uArrayIndices > 0)
        {
            strLine = str(format("\t\tuArrayIndices = %i\n") % pScopeVariableInfo[i].uArrayIndices);
            strDebugInfo += strLine;
            strLine = str(format("\t\tArrayElements = NOT DONE\n"/* % pScopeVariableInfo[i].ArrayElements*/));
            strDebugInfo += strLine;
            strLine = str(format("\t\tArrayStrides = NOT DONE\n"/* % pScopeVariableInfo[i].ArrayStrides*/));
            strDebugInfo += strLine;
        }

        strLine = str(format("\t\tuVariables = %i\n") % pScopeVariableInfo[i].uVariables);
        strDebugInfo += strLine;

        if (pScopeVariableInfo[i].uVariables > 0)
        {
            strLine = str(format("\t\tuFirstVariable = %i\n") % pScopeVariableInfo[i].uFirstVariable);
            strDebugInfo += strLine;

            std::string strVariableInfo;
            PrintVariableInfo(pDebugInfo, pScopeVariableInfo[i].uFirstVariable, strVariableInfo);

            strLine = str(format("\t\tVariables = %s\n") % strVariableInfo);
            strDebugInfo += strLine;
        }
    }

    return true;
}
