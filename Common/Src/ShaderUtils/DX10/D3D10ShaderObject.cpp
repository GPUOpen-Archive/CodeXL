//=====================================================================
// Copyright 2007-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file D3D10ShaderObject.cpp
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10ShaderObject.cpp#7 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================


#include <Windows.h>
#include <assert.h>
#include <D3D11.h>
#include <ddraw.h>
#include "StringUtils.h"
#include "D3D10ShaderObject.h"
#include "D3D10ShaderUtils.h"
#include "DXBCChecksum.h"
#include "d3d11tokenizedprogramformat.hpp"
#include "boost/format.hpp"
#include "boost/foreach.hpp"
#include "../GDT_Memory.h"

using namespace std;
using boost::format;

// TODO - Move these definitions to a common file.

#define SAFE_FREE( p ) { if ( p != NULL ) { free(p); p = NULL; } }

#define PAD_DWORD(n) ((n+3) & 0xfffc)

using namespace D3D10ShaderObject;

CShaderChunk::CShaderChunk() : m_pChunkHeader(NULL)
{
}

CShaderChunk::CShaderChunk(const D3D10_ChunkHeader* pHeader)
{
    Initialise(pHeader);
}

CShaderChunk::~CShaderChunk()
{
    SAFE_FREE(m_pChunkHeader);
}

CShaderChunk::CShaderChunk(const CShaderChunk& obj)
{
    Initialise(obj.m_pChunkHeader);
}

CShaderChunk& CShaderChunk::operator=(const CShaderChunk& obj)
{
    if (this != &obj)
    {
        SAFE_FREE(m_pChunkHeader);
        Initialise(obj.m_pChunkHeader);
    }

    return *this;
}

void CShaderChunk::Initialise(const D3D10_ChunkHeader* pHeader)
{
    if (pHeader != NULL)
    {
        DWORD dwSize = pHeader->dwChunkDataSize + sizeof(D3D10_ChunkHeader);
        m_pChunkHeader = (D3D10_ChunkHeader*) malloc(dwSize);
        memcpy(m_pChunkHeader, pHeader, dwSize);
    }
    else
    {
        m_pChunkHeader = NULL;
    }
}


CD3D10ShaderObject::CD3D10ShaderObject()
{
    InitialiseData();
}

CD3D10ShaderObject::CD3D10ShaderObject(const void* pShaderData, size_t nShaderDataSize)
{
    InitialiseData();

    SetShaderObject(pShaderData, nShaderDataSize);
}

CD3D10ShaderObject::CD3D10ShaderObject(const IBlob* pShaderBlob)
{
    InitialiseData();

    assert(pShaderBlob);

    if (pShaderBlob)
    {
        SetShaderObject(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());
    }
}

CD3D10ShaderObject::CD3D10ShaderObject(const CD3D10ShaderObject& shaderObject)
{
    InitialiseData();

    m_strCheckSum = shaderObject.GetCheckSum();

    SetRDEFChunk(shaderObject.GetRDEFChunk());
    SetISGNChunk(shaderObject.GetISGNChunk());
    SetOSGNChunk(shaderObject.GetOSGNChunk());
    SetShaderCodeChunk(shaderObject.GetShaderCodeChunk());
    SetSTATChunk(shaderObject.GetSTATChunk());
    SetSDBGChunk(shaderObject.GetSDBGChunk());
    SetAon9Chunk(shaderObject.GetAon9Chunk());
    SetSFI0Chunk(shaderObject.GetSFI0Chunk());
    SetIFCEChunk(shaderObject.GetIFCEChunk());
    SetPCSGChunk(shaderObject.GetPCSGChunk());
    SetOSG5Chunk(shaderObject.GetOSG5Chunk());
}


CD3D10ShaderObject& CD3D10ShaderObject::operator=(const CD3D10ShaderObject& shaderObject)
{
    if (this != &shaderObject)
    {
        FreeData();

        InitialiseData();

        m_strCheckSum = shaderObject.GetCheckSum();

        SetRDEFChunk(shaderObject.GetRDEFChunk());
        SetISGNChunk(shaderObject.GetISGNChunk());
        SetOSGNChunk(shaderObject.GetOSGNChunk());
        SetShaderCodeChunk(shaderObject.GetShaderCodeChunk());
        SetSTATChunk(shaderObject.GetSTATChunk());
        SetSDBGChunk(shaderObject.GetSDBGChunk());
        SetAon9Chunk(shaderObject.GetAon9Chunk());
        SetSFI0Chunk(shaderObject.GetSFI0Chunk());
        SetIFCEChunk(shaderObject.GetIFCEChunk());
        SetPCSGChunk(shaderObject.GetPCSGChunk());
        SetOSG5Chunk(shaderObject.GetOSG5Chunk());
    }

    return *this;
}


CD3D10ShaderObject::~CD3D10ShaderObject()
{
    FreeData();
}

void CD3D10ShaderObject::InitialiseData()
{
    m_pShader = NULL;
    m_nShaderDataSize = 0;

    m_strCheckSum.clear();

    m_pRDEFChunk = NULL;
    m_pISGNChunk = NULL;
    m_pOSGNChunk = NULL;
    m_pSHDRChunk = NULL;
    m_pSHEXChunk = NULL;
    m_pSTATChunk = NULL;
    m_pSDBGChunk = NULL;
    m_pAon9Chunk = NULL;
    m_pSFI0Chunk = NULL;
    m_pIFCEChunk = NULL;
    m_pPCSGChunk = NULL;
    m_pOSG5Chunk = NULL;

    m_pStats = NULL;
    memset(&m_EmptyStats, 0, sizeof(m_EmptyStats));

    ClearResourceDefinitions();
    ClearInputSignatures();
    ClearOutputSignatures();
    ClearShaderBytecode();
    ClearStatistics();
    ClearDebugInfo();
}

void CD3D10ShaderObject::FreeData()
{
    ClearResourceDefinitions();
    ClearInputSignatures();
    ClearOutputSignatures();
    ClearShaderBytecode();
    ClearStatistics();
    ClearDebugInfo();

    SAFE_FREE(m_pShader);
    m_nShaderDataSize = 0;

    SAFE_FREE(m_pRDEFChunk);
    SAFE_FREE(m_pISGNChunk);
    SAFE_FREE(m_pOSGNChunk);
    SAFE_FREE(m_pSHDRChunk);
    SAFE_FREE(m_pSHEXChunk);
    SAFE_FREE(m_pSTATChunk);
    SAFE_FREE(m_pSDBGChunk);
    SAFE_FREE(m_pAon9Chunk);
    SAFE_FREE(m_pSFI0Chunk);
    SAFE_FREE(m_pIFCEChunk);
    SAFE_FREE(m_pPCSGChunk);
    SAFE_FREE(m_pOSG5Chunk);
}

bool CD3D10ShaderObject::IsValid() const
{
    return (m_pISGNChunk != NULL &&
            (m_pSHDRChunk != NULL || m_pSHEXChunk != NULL)
           );
}

DWORD CD3D10ShaderObject::CalculateChunkSize(const D3D10_ChunkHeader* pChunkHeader) const
{
    if (pChunkHeader != NULL)
    {
        return pChunkHeader->dwChunkDataSize + sizeof(D3D10_ChunkHeader);
    }

    return 0;
}

bool CD3D10ShaderObject::SetShaderObject(const IBlob* pShaderBlob)
{
    assert(pShaderBlob);

    if (pShaderBlob)
    {
        return SetShaderObject(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());
    }

    return false;
}

bool CD3D10ShaderObject::SetShaderObject(const void* pShaderData, size_t nShaderDataSize)
{
    FreeData();

    assert(pShaderData != NULL);
    assert(nShaderDataSize != 0);

    if (pShaderData == NULL || nShaderDataSize == 0)
    {
        return false;
    }

    /// Extract the shader object header from the data buffer
    D3D10_ShaderObjectHeader* pShaderHeader = (D3D10_ShaderObjectHeader*) pShaderData;

    m_strCheckSum = str(format("%08x%08x%08x%08x") % pShaderHeader->dwCheckSum[0] %
                        pShaderHeader->dwCheckSum[1] %
                        pShaderHeader->dwCheckSum[2] %
                        pShaderHeader->dwCheckSum[3]);

    /// Extract the chunk offset table from the data buffer
    DWORD* pdwChunkOffsets = (DWORD*)(((char*) pShaderData) + sizeof(D3D10_ShaderObjectHeader));

    // Iterate through each chunk in the chunk offset table
    for (DWORD i = 0; i < pShaderHeader->dwNumChunks; i++)
    {
        // Get a pointer to the chunk
        D3D10_ChunkHeader* pChunkHeader = (D3D10_ChunkHeader*)(((char*) pShaderHeader) + pdwChunkOffsets[i]);


        // Decode the chunk type & set our internal copy for that chunk type
        switch (pChunkHeader->dwChunkType)
        {
            case MAKEFOURCC('R', 'D', 'E', 'F'): SetRDEFChunk(pChunkHeader); break;

            case MAKEFOURCC('I', 'S', 'G', 'N'): SetISGNChunk(pChunkHeader); break;

            case MAKEFOURCC('O', 'S', 'G', 'N'): SetOSGNChunk(pChunkHeader); break;

            case MAKEFOURCC('S', 'H', 'D', 'R'): SetSHDRChunk(pChunkHeader); break;

            case MAKEFOURCC('S', 'H', 'E', 'X'): SetSHEXChunk(pChunkHeader); break;

            case MAKEFOURCC('S', 'T', 'A', 'T'): SetSTATChunk(pChunkHeader); break;

            case MAKEFOURCC('S', 'D', 'B', 'G'): SetSDBGChunk(pChunkHeader); break;

            case MAKEFOURCC('A', 'o', 'n', '9'): SetAon9Chunk(pChunkHeader); break;

            case MAKEFOURCC('S', 'F', 'I', '0'): SetSFI0Chunk(pChunkHeader); break;

            case MAKEFOURCC('I', 'F', 'C', 'E'): SetIFCEChunk(pChunkHeader); break;

            case MAKEFOURCC('P', 'C', 'S', 'G'): SetPCSGChunk(pChunkHeader); break;

            case MAKEFOURCC('O', 'S', 'G', '5'): SetOSG5Chunk(pChunkHeader); break;

            default:
            {
                char szFourCC[5];
                strncpy_s(szFourCC, 5, (char*) &pChunkHeader->dwChunkType, 4);

                char szTemp[256];
                sprintf_s(szTemp, 256, "Unknown ChunkType - %s\n", szFourCC);
                OutputDebugStringA(szTemp);
            }

#ifdef SD_DEV
            assert(0);
#endif // SD_DEV
            break;
        }
    }

    return IsValid();
}

IBlob* CD3D10ShaderObject::GetShaderObject(UINT nChunks) const
{
    assert(IsValid());

    if (!IsValid())
    {
        return NULL;
    }

    // Calculate the number of chunks & the object size
    DWORD dwChunkCount = 0;
    size_t nObjectSize = sizeof(D3D10_ShaderObjectHeader);

    if ((nChunks & RDEF) && GetRDEFChunk())
    {
        dwChunkCount++;
        nObjectSize += GetRDEFChunkSize();
    }

    if ((nChunks & ISGN) && GetISGNChunk())
    {
        dwChunkCount++;
        nObjectSize += GetISGNChunkSize();
    }

    if ((nChunks & OSGN) && GetOSGNChunk())
    {
        dwChunkCount++;
        nObjectSize += GetOSGNChunkSize();
    }

    if ((nChunks & ShaderCode) && GetShaderCodeChunk())
    {
        dwChunkCount++;
        nObjectSize += GetShaderCodeChunkSize();
    }

    if ((nChunks & STAT) && GetSTATChunk())
    {
        dwChunkCount++;
        nObjectSize += GetSTATChunkSize();
    }

    if ((nChunks & SDBG) && GetSDBGChunk())
    {
        dwChunkCount++;
        nObjectSize += GetSDBGChunkSize();
    }

    if ((nChunks & Aon9) && GetAon9Chunk())
    {
        dwChunkCount++;
        nObjectSize += GetAon9ChunkSize();
    }

    if ((nChunks & SFI0) && GetSFI0Chunk())
    {
        dwChunkCount++;
        nObjectSize += GetSFI0ChunkSize();
    }

    if ((nChunks & IFCE) && GetIFCEChunk())
    {
        dwChunkCount++;
        nObjectSize += GetIFCEChunkSize();
    }

    if ((nChunks & PCSG) && GetPCSGChunk())
    {
        dwChunkCount++;
        nObjectSize += GetPCSGChunkSize();
    }

    if ((nChunks & OSG5) && GetOSG5Chunk())
    {
        dwChunkCount++;
        nObjectSize += GetOSG5ChunkSize();
    }

    // Account for the chunk table
    const DWORD dwChunkTableSize = dwChunkCount * sizeof(DWORD);
    nObjectSize += dwChunkTableSize;

    // Create the data blob
    IBlob* pBlob = CreateBlob(nObjectSize);
    assert(pBlob);

    if (pBlob == NULL)
    {
        return NULL;
    }

    BYTE* pBlobData = (BYTE*) pBlob->GetBufferPointer();

    // Initialize the object header
    D3D10_ShaderObjectHeader* pObjectHeader = (D3D10_ShaderObjectHeader*) pBlobData;
    pObjectHeader->dwMagicNumber = MAKEFOURCC('D', 'X', 'B', 'C');
    pObjectHeader->dwCheckSum[0] = pObjectHeader->dwCheckSum[1] = pObjectHeader->dwCheckSum[2] = pObjectHeader->dwCheckSum[3] = 0;
    pObjectHeader->dwReserved = 1;
    pObjectHeader->dwSize = (DWORD) nObjectSize;
    pObjectHeader->dwNumChunks = dwChunkCount;

    // Get the chunk table
    DWORD* pChunkTable = (DWORD*)(pBlobData + sizeof(D3D10_ShaderObjectHeader));
    DWORD dwChunkIndex = 0;

    // Calculate the chunk offset
    DWORD dwChunkOffset = sizeof(D3D10_ShaderObjectHeader) + dwChunkTableSize;

    // Copy the chunks into the blob, updating the chunk table as we go
    if ((nChunks & Aon9) && GetAon9Chunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetAon9Chunk(), GetAon9ChunkSize());
        dwChunkOffset += GetAon9ChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & RDEF) && GetRDEFChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetRDEFChunk(), GetRDEFChunkSize());
        dwChunkOffset += GetRDEFChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & ISGN) && GetISGNChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetISGNChunk(), GetISGNChunkSize());
        dwChunkOffset += GetISGNChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & OSGN) && GetOSGNChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetOSGNChunk(), GetOSGNChunkSize());
        dwChunkOffset += GetOSGNChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & ShaderCode) && GetShaderCodeChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetShaderCodeChunk(), GetShaderCodeChunkSize());
        dwChunkOffset += GetShaderCodeChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & STAT) && GetSTATChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetSTATChunk(), GetSTATChunkSize());
        dwChunkOffset += GetSTATChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & SDBG) && GetSDBGChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetSDBGChunk(), GetSDBGChunkSize());
        dwChunkOffset += GetSDBGChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & SFI0) && GetSFI0Chunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetSFI0Chunk(), GetSFI0ChunkSize());
        dwChunkOffset += GetSFI0ChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & IFCE) && GetIFCEChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetIFCEChunk(), GetIFCEChunkSize());
        dwChunkOffset += GetIFCEChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & PCSG) && GetPCSGChunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetPCSGChunk(), GetPCSGChunkSize());
        dwChunkOffset += GetPCSGChunkSize();
        dwChunkIndex++;
    }

    if ((nChunks & OSG5) && GetOSG5Chunk())
    {
        pChunkTable[dwChunkIndex] = dwChunkOffset;
        memcpy((pBlobData + dwChunkOffset), GetOSG5Chunk(), GetOSG5ChunkSize());
    }

    // Update the check sum
    CalculateDXBCChecksum(pBlobData, (DWORD) nObjectSize, pObjectHeader->dwCheckSum);

    return pBlob;
}

D3D10_ChunkHeader* CD3D10ShaderObject::CopyChunk(const D3D10_ChunkHeader* pChunk) const
{
    // Calculate the size of the chunk to copy. This will be 0 if the chunk pointer is null.
    DWORD dwChunkSize = CalculateChunkSize(pChunk);

    if (dwChunkSize == 0)
    {
        return NULL;
    }

    // Allocate memory for the chunk copy & copy the data.
    D3D10_ChunkHeader* pNewChunk = (D3D10_ChunkHeader*) malloc(dwChunkSize);
    assert(pNewChunk != NULL);

    if (pNewChunk != NULL)
    {
        memcpy(pNewChunk, pChunk, dwChunkSize);
    }

    // Return a pointer to freshly-created chunk copy.
    return pNewChunk;
}

bool CD3D10ShaderObject::SetRDEFChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pRDEFChunk);

    m_pRDEFChunk = CopyChunk(pChunk);

    return ParseRDEFChunk();
}

bool CD3D10ShaderObject::SetISGNChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pISGNChunk);

    m_pISGNChunk = CopyChunk(pChunk);
    assert(m_pISGNChunk != NULL || pChunk == NULL);

    return ParseISGNChunk();
}

bool CD3D10ShaderObject::SetOSGNChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pOSGNChunk);

    m_pOSGNChunk = CopyChunk(pChunk);
    assert(m_pOSGNChunk != NULL || pChunk == NULL);

    return ParseOSGNChunk();
}

bool CD3D10ShaderObject::SetSTATChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pSTATChunk);

    m_pSTATChunk = CopyChunk(pChunk);
    assert(m_pSTATChunk != NULL || pChunk == NULL);

    return ParseSTATChunk();
}

bool CD3D10ShaderObject::SetSHDRChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pSHDRChunk);
    SAFE_FREE(m_pSHEXChunk);

    m_pSHDRChunk = CopyChunk(pChunk);
    assert(m_pSHDRChunk != NULL || pChunk == NULL);

    return ParseSHDRChunk();
}

bool CD3D10ShaderObject::SetSHEXChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pSHDRChunk);
    SAFE_FREE(m_pSHEXChunk);

    m_pSHEXChunk = CopyChunk(pChunk);
    assert(m_pSHEXChunk != NULL || pChunk == NULL);

    return ParseSHEXChunk();
}

bool CD3D10ShaderObject::SetShaderCodeChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pSHDRChunk);
    SAFE_FREE(m_pSHEXChunk);

    if (pChunk == NULL)
    {
        return false;
    }
    else if (pChunk->dwChunkType == MAKEFOURCC('S', 'H', 'D', 'R'))
    {
        return SetSHDRChunk(pChunk);
    }
    else if (pChunk->dwChunkType == MAKEFOURCC('S', 'H', 'E', 'X'))
    {
        return SetSHEXChunk(pChunk);
    }

    return false;
}

bool CD3D10ShaderObject::SetSDBGChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pSDBGChunk);

    m_pSDBGChunk = CopyChunk(pChunk);
    assert(m_pSDBGChunk != NULL || pChunk == NULL);

    return ParseSDBGChunk();
}

bool CD3D10ShaderObject::SetAon9Chunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pAon9Chunk);

    m_pAon9Chunk = CopyChunk(pChunk);
    assert(m_pAon9Chunk != NULL || pChunk == NULL);

    return ParseAon9Chunk();
}

bool CD3D10ShaderObject::SetSFI0Chunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pSFI0Chunk);

    m_pSFI0Chunk = CopyChunk(pChunk);
    assert(m_pSFI0Chunk != NULL || pChunk == NULL);

    return ParseSFI0Chunk();
}

bool CD3D10ShaderObject::SetIFCEChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pIFCEChunk);

    m_pIFCEChunk = CopyChunk(pChunk);
    assert(m_pIFCEChunk != NULL || pChunk == NULL);

    return ParseIFCEChunk();
}

bool CD3D10ShaderObject::SetPCSGChunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pPCSGChunk);

    m_pPCSGChunk = CopyChunk(pChunk);
    assert(m_pPCSGChunk != NULL || pChunk == NULL);

    return ParsePCSGChunk();
}

bool CD3D10ShaderObject::SetOSG5Chunk(const D3D10_ChunkHeader* pChunk)
{
    SAFE_FREE(m_pOSG5Chunk);

    m_pOSG5Chunk = CopyChunk(pChunk);
    assert(m_pOSG5Chunk != NULL || pChunk == NULL);

    return ParseOSG5Chunk();
}

void CD3D10ShaderObject::ClearResourceDefinitions()
{
    m_dwConstantBuffers = 0;
    m_dwTextures = 0;
    m_dwSamplers = 0;
    m_dwComputeBuffers = 0;
    m_dwVersion = 0;
    m_dwFlags = 0;
    m_dwConstants = 0;
    m_strRDEFCreator.clear();
    m_BoundResources.clear();
    m_ConstantBuffers.clear();
    m_Constants.clear();
    m_strRDEFNames.clear();
    m_ConstantTypes.clear();
    m_pRDEF11_Header = NULL;
}

bool CD3D10ShaderObject::ParseRDEFChunk()
{
    ClearResourceDefinitions();

    if (m_pRDEFChunk == NULL)
    {
        return false;
    }

    // Initialize pointers to the chunk data & RDEF chunk header
    const char* pRDEFChunkData = (char*)(m_pRDEFChunk + 1);
    const D3D10_RDEF_Header& RDEF_Header = *(D3D10_RDEF_Header*) pRDEFChunkData;
    const char* pszStringTable = pRDEFChunkData;

    // Parse the RDEF chunk header
    m_dwConstantBuffers = RDEF_Header.dwConstantBuffers;
    DWORD dwBoundResources = RDEF_Header.dwBoundResources;
    m_dwVersion = RDEF_Header.dwVersion;
    m_dwFlags = RDEF_Header.dwCompilerFlags;
    m_strRDEFCreator = &pszStringTable[RDEF_Header.dwCreatorOffset];

    if (LOWORD(m_dwVersion) >= 0x0500)
    {
        m_pRDEF11_Header = (D3D11_RDEF_Header*) pRDEFChunkData;
    }

    // Iterate through each bound-resource
    const D3D10_ResourceBinding* pResourceBindings = (D3D10_ResourceBinding*)(pRDEFChunkData + RDEF_Header.dwBoundResourceOffset);

    for (UINT i = 0; i < dwBoundResources; i++)
    {
        // Read the resource name
        AddRDEFName(pszStringTable, pResourceBindings[i].dwNameOffset);

        switch (pResourceBindings[i].Type)
        {
            case D3D10_SIT_TEXTURE:                         m_dwTextures++; break;

            case D3D10_SIT_TBUFFER:                         m_dwTextures++; break;

            case D3D10_SIT_SAMPLER:                         m_dwSamplers++; break;

            case D3D11_SIT_UAV_RWTYPED:                     m_dwComputeBuffers; break;

            case D3D11_SIT_STRUCTURED:                      m_dwComputeBuffers; break;

            case D3D11_SIT_UAV_RWSTRUCTURED:                m_dwComputeBuffers; break;

            case D3D11_SIT_BYTEADDRESS:                     m_dwComputeBuffers; break;

            case D3D11_SIT_UAV_RWBYTEADDRESS:               m_dwComputeBuffers; break;

            case D3D11_SIT_UAV_APPEND_STRUCTURED:           m_dwComputeBuffers; break;

            case D3D11_SIT_UAV_CONSUME_STRUCTURED:          m_dwComputeBuffers; break;

            case D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:   m_dwComputeBuffers; break;

            default: break;
        }

        // Read the D3D10_ResourceBinding structure
        m_BoundResources.push_back(pResourceBindings[i]);
    }

    // Iterate through each constant-buffer
    const D3D10_ConstantBuffer* pConstBuffers = (D3D10_ConstantBuffer*)(pRDEFChunkData + RDEF_Header.dwConstBufferOffset);

    for (UINT i = 0; i < m_dwConstantBuffers; i++)
    {
        // Read the constant-buffer name
        m_dwConstants += pConstBuffers[i].dwVariables;
        AddRDEFName(pszStringTable, pConstBuffers[i].dwNameOffset);

        // Read the D3D10_ConstantBuffer structure
        m_ConstantBuffers.push_back(pConstBuffers[i]);

        const char* pConstantData = pRDEFChunkData + pConstBuffers[i].dwOffset;

        if (m_pRDEF11_Header != NULL)
        {
            // HACK
            // There is something funny going on here. Sometimes there are a superfluous 4 DWORDs between constants descriptions
            // with SM5. I've yet to discern a pattern. Since we don't currently use this parsed data I shall disable parsing it until we can
            // parse it correctly.

            //#ifdef SD_DEV
            // Iterate through each constant
            const D3D11_Constant* pConstants = (D3D11_Constant*) pConstantData;

            for (UINT j = 0; j < pConstBuffers[i].dwVariables; j++)
            {
                // Read the constant name
                AddRDEFName(pszStringTable, pConstants[j].dwNameOffset);

                // So far every D3D11_Constant we'v ever seen has the same values for the extra DWORDs
                // Let's Assert on these to detect if\when this is not the case.
                assert(pConstants[j].dwReserved[0] == 0xffffffff && pConstants[j].dwReserved[1] == 0x00000000 &&
                       pConstants[j].dwReserved[2] == 0xffffffff && pConstants[j].dwReserved[3] == 0x00000000);

                // Read the D3D10_Constant structure
                m_Constants.push_back(*(const D3D10_Constant*)(&pConstants[j]));

                // Read the D3D10_Type structure
                D3D10_Type* pConstType = ((D3D10_Type*) &pszStringTable[pConstants[j].dwTypeOffset]);
                m_ConstantTypes[pConstants[j].dwTypeOffset] = (*pConstType);
                AddRDEFName(pszStringTable, pConstType->dwNameOffset);
            }

            //#endif // SD_DEV
        }
        else
        {
            // Iterate through each constant
            const D3D10_Constant* pConstants = (D3D10_Constant*) pConstantData;

            for (UINT j = 0; j < pConstBuffers[i].dwVariables; j++)
            {
                // Read the constant name
                AddRDEFName(pszStringTable, pConstants[j].dwNameOffset);

                // Read the D3D10_Constant structure
                m_Constants.push_back(pConstants[j]);

                // Read the D3D10_Type structure
                D3D10_Type* pConstType = ((D3D10_Type*) & pszStringTable[pConstants[j].dwTypeOffset]);
                m_ConstantTypes[pConstants[j].dwTypeOffset] = *pConstType;
            }
        }
    }

    return true;
}

/// Template class for handling the packing of data in the method required for DX10/DX11 shader objects.
template<class T, class _less = less<T>> class DX10AsmDataPacker
{
public:
    /// Constructor.
    /// \param[in] nBaseOffset The base offset from the start of the chunk to the start of the data area.
    DX10AsmDataPacker(size_t nBaseOffset) : m_nBaseOffset(nBaseOffset) {};

    /// Add the specified data item to the list of data to write.
    /// \param[in] t           The data item.
    void Add(const T& t)
    {
        if (m_OffsetMap.find(t) == m_OffsetMap.end())
        {
            m_OffsetMap[t] = 0;
            m_WrittenMap[t] = false;
        }
    }

    /// Get the offset within the chunk to where the specified data was written.
    /// \param[in] t     The data item.
    /// \return          The offset within the chunk to where the specified data was written if successful, otherwise 0.
    size_t GetOffset(const T& t)
    {
        if (m_WrittenMap[t])
        {
            return m_OffsetMap[t] - m_nBaseOffset;
        }
        else
        {
            return 0;
        }
    }

    /// Write a data item to the memory buffer.
    /// \param[in] t        The data item to write.
    /// \param[in] buffer   The memory buffer.
    /// \return             The size of the item written if successful, otherwise 0.
    size_t Write(const T& t, GDT_Memory::GDT_MemoryBuffer& buffer)
    {
        if (!m_WrittenMap[t])
        {
            m_OffsetMap[t] = buffer.GetOffset();
            size_t nSize = GetSize(t);
            buffer.Write(GetValue(t), nSize);
            m_WrittenMap[t] = true;
            return nSize;
        }

        return 0;
    }

private:
    size_t m_nBaseOffset;                           ///< The base offset from the start of the chunk to the start of the data area.
    std::map<const T, size_t, _less> m_OffsetMap;   ///< Map indicating the offset within the chunk for each data item.
    std::map<const T, bool, _less> m_WrittenMap;    ///< Map indicating whether a data item has yet been written.
};

template<class T> const void* GetValue(const T& t) { return &t; };      ///< Opportunity for specialization to support std::string
template<class T> size_t GetSize(const T&) { return sizeof(T); };      ///< Opportunity for specialization to support std::string

const void* GetValue(const std::string& t) { return t.c_str(); };
size_t GetSize(const std::string& t) { return t.length() + 1; };

/// Comparator for D3D10_Type struct.
/// Needed to allow using D3D10_Type as index in std::map.
struct D3D10_Type_less : public std::less<D3D10_Type>
{
    /// Perform comparison of D3D10_Type objects.
    /// \param[in] x  First D3D10_Type object.
    /// \param[in] y  Second D3D10_Type object.
    /// \return       True if x < y, otherwise false.
    bool operator()(const D3D10_Type& x, const D3D10_Type& y) const
    {
        return (memcmp(&x, &y, sizeof(D3D10_Type)) < 0) ? true : false;
    };
};


static const char PAD_BYTE = (BYTE) 0xab;


bool CD3D10ShaderObject::GenerateRDEFChunk()
{
    GDT_Memory::GDT_MemoryBuffer buffer;

    DX10AsmDataPacker<std::string>   namePacker(sizeof(D3D10_ChunkHeader));
    std::map<size_t, std::string>    mNameOffsetLocations;

    DX10AsmDataPacker<D3D10_Type, D3D10_Type_less> typePacker(sizeof(D3D10_ChunkHeader));
    std::map<size_t, D3D10_Type>     mTypeOffsetLocations;

    // Write chunk header
    DWORD dwChunkType = MAKEFOURCC('R', 'D', 'E', 'F');
    buffer.Write(dwChunkType);

    size_t nChunkSizePosition = buffer.GetOffset(); // Store the offset to the chunk size so that we can update it with the size later
    buffer.Write((DWORD) 0);

    // Write RDEF header
    size_t nRDEFHeaderOffset = buffer.GetOffset();

    if (m_pRDEF11_Header != NULL)
    {
        D3D11_RDEF_Header RDEF_Header;
        memcpy(&RDEF_Header, m_pRDEF11_Header, sizeof(RDEF_Header));

        RDEF_Header.dwConstantBuffers = GetConstantBufferCount();
        RDEF_Header.dwConstBufferOffset = 0;
        RDEF_Header.dwBoundResources = GetBoundResourceCount();
        RDEF_Header.dwBoundResourceOffset = sizeof(RDEF_Header);
        RDEF_Header.dwVersion = m_dwVersion;
        RDEF_Header.dwCompilerFlags = m_dwFlags;
        RDEF_Header.dwCreatorOffset = 0;

        buffer.Write(RDEF_Header);
    }
    else
    {
        D3D10_RDEF_Header RDEF_Header;
        RDEF_Header.dwConstantBuffers = GetConstantBufferCount();
        RDEF_Header.dwConstBufferOffset = 0;
        RDEF_Header.dwBoundResources = GetBoundResourceCount();
        RDEF_Header.dwBoundResourceOffset = sizeof(RDEF_Header);
        RDEF_Header.dwVersion = m_dwVersion;
        RDEF_Header.dwCompilerFlags = m_dwFlags;
        RDEF_Header.dwCreatorOffset = 0;

        buffer.Write(RDEF_Header);
    }

    // My brain isn't working fully because of my cold so I need to think this through.
    // So how do we want to do the offsets? When we store a data item with an offset we dereference the offset to get the data item.
    // We add this to a map of the data items to new offsets, setting the offsets to 0 for now. If already in the map we leave be.
    // And we add the position of the current offset to another map, pointing to the data value.
    // Add each point where data items are saved we iterate through the data->new_offset map & write any with offset 0, updating to the real offset.
    // We we've finished we iterate through the offset_location->data map and find the remapped offset to write from the other map.
    // Profit!

    // Iterate the resource bindings
    for (unsigned int i = 0; i < GetBoundResourceCount(); i++)
    {
        const std::string& strName = GetRDEFName(m_BoundResources[i].dwNameOffset);
        namePacker.Add(strName);

        size_t nOffsetLocation = buffer.GetOffset();
        mNameOffsetLocations[nOffsetLocation] = strName;

        buffer.Write(m_BoundResources[i]);
    }

    // Output the resource binding names
    for (unsigned int i = 0; i < GetBoundResourceCount(); i++)
    {
        const std::string& strName = GetRDEFName(m_BoundResources[i].dwNameOffset);
        namePacker.Write(strName, buffer);
    }

    if (m_dwConstantBuffers > 0) // Output the offset to the constant buffers if present
    {
        buffer.PadToDoubleWord(PAD_BYTE);

        buffer.WriteAt(nRDEFHeaderOffset + sizeof(DWORD), buffer.GetOffset() - sizeof(D3D10_ChunkHeader));
    }

    // Output the constant buffers
    std::vector<size_t> vnConstBufferDescOffset;

    for (unsigned int i = 0; i < m_dwConstantBuffers; i++)
    {
        std::string strName = GetRDEFName(m_ConstantBuffers[i].dwNameOffset);
        size_t nOffsetLocation = buffer.GetOffset();
        mNameOffsetLocations[nOffsetLocation] = strName;

        vnConstBufferDescOffset.push_back(buffer.GetOffset() + (2 * sizeof(DWORD)));

        buffer.Write(m_ConstantBuffers[i]);
    }

    // For each constant buffer output it's member constants & their types
    int nCurrConstBase = 0;

    for (unsigned int i = 0; i < m_dwConstantBuffers; i++)
    {
        buffer.PadToDoubleWord(PAD_BYTE);

        // Update the offset in the constant desc to point here.
        buffer.WriteAt(vnConstBufferDescOffset[i], buffer.GetOffset() - sizeof(D3D10_ChunkHeader));

        // Output the member constants
        for (unsigned int j = 0; j < m_ConstantBuffers[i].dwVariables; j++)
        {
            D3D10_Constant& constant = m_Constants.at(nCurrConstBase + j);

            const std::string& strName = GetRDEFName(constant.dwNameOffset);
            namePacker.Add(strName);

            size_t nOffsetLocation = buffer.GetOffset();
            mNameOffsetLocations[nOffsetLocation] = strName;

            D3D10_Type& constType = m_ConstantTypes[constant.dwTypeOffset];
            typePacker.Add(constType);

            nOffsetLocation = buffer.GetOffset() + (4 * sizeof(DWORD));
            mTypeOffsetLocations[nOffsetLocation] = constType;

            const std::string& strTypeName = GetRDEFName(constType.wOffset);
            namePacker.Add(strTypeName);

            buffer.Write(constant);

            // If we have the RD11 header then we need to write this - reason unknown.
            if (m_pRDEF11_Header != NULL)
            {
                buffer.Write(0xffffffff);
                buffer.Write(0x00000000);
                buffer.Write(0xffffffff);
                buffer.Write(0x00000000);
            }
        }

        // Output the member constant names & types
        for (unsigned int j = 0; j < m_ConstantBuffers[i].dwVariables; j++)
        {
            D3D10_Constant& constant = m_Constants.at(nCurrConstBase + j);

            const std::string& strName = GetRDEFName(constant.dwNameOffset);
            namePacker.Write(strName, buffer);

            const D3D10_Type& constType = m_ConstantTypes[constant.dwTypeOffset];
            const std::string& strTypeName = GetRDEFName(constType.dwNameOffset);
            namePacker.Write(strTypeName, buffer);
            buffer.PadToDoubleWord(PAD_BYTE);
            typePacker.Write(constType, buffer);
        }

        nCurrConstBase += m_ConstantBuffers[i].dwVariables;
    }

    buffer.WriteAt((nRDEFHeaderOffset + (6 * sizeof(DWORD))), buffer.GetOffset() - sizeof(D3D10_ChunkHeader));

    typedef std::pair<size_t, std::string> NameOffsetLocation;
    BOOST_FOREACH(NameOffsetLocation offsetLocation, mNameOffsetLocations)
    {
        buffer.WriteAt(offsetLocation.first, namePacker.GetOffset(offsetLocation.second));
    }

    typedef std::pair<size_t, D3D10_Type> TypeOffsetLocation;
    BOOST_FOREACH(TypeOffsetLocation offsetLocation, mTypeOffsetLocations)
    {
        buffer.WriteAt(offsetLocation.first, typePacker.GetOffset(offsetLocation.second));
    }

    buffer.Write(m_strRDEFCreator.c_str(), m_strRDEFCreator.length() + 1);

    buffer.PadToDoubleWord(PAD_BYTE);

    buffer.WriteAt(nChunkSizePosition, buffer.GetOffset() - sizeof(D3D10_ChunkHeader));

    return SetRDEFChunk((D3D10_ChunkHeader*) buffer.GetBuffer());
}


bool CD3D10ShaderObject::ParseSGNChunk(const D3D10_ChunkHeader* pChunk, DWORD& dwSignatures, DWORD& dwReserved,
                                       vector<string>& strSemanticNames, vector<D3D10_Signature>& signatures)
{
    assert(pChunk);

    if (pChunk == NULL)
    {
        return false;
    }

    // Initialize pointers to the chunk data & xSGN chunk header
    char* pChunkData = (char*)(pChunk + 1);
    D3D10_xSGN_Header& SGN_Header = *(D3D10_xSGN_Header*) pChunkData;
    const char* pszStringTable = pChunkData;
    D3D10_Signature* pSignatures = (D3D10_Signature*)(pChunkData + sizeof(D3D10_xSGN_Header));

    // Parse the xSGN chunk header
    dwSignatures = SGN_Header.dwSignatures;
    dwReserved = SGN_Header.dwReserved;

    // Iterate through each signature
    for (UINT i = 0; i < dwSignatures; i++)
    {
        // Read the signature semantic name
        if (pSignatures[i].dwSemanticNameOffset)
        {
            string strSemanticName = &pszStringTable[pSignatures[i].dwSemanticNameOffset];
            strSemanticNames.push_back(strSemanticName);
        }
        else
        {
            strSemanticNames.push_back("");
        }

        // Read the D3D10_Signature structure
        signatures.push_back(pSignatures[i]);
    }

    return true;
}


D3D10_ChunkHeader* CD3D10ShaderObject::GenerateSGNChunk(const DWORD dwChunkType, const DWORD dwSignatures,
                                                        const vector<string>& strSemanticNames,
                                                        const vector<D3D10_Signature>& signatures)
{
    GDT_Memory::GDT_MemoryBuffer buffer;

    DX10AsmDataPacker<std::string>   namePacker(sizeof(D3D10_ChunkHeader));
    std::map<size_t, std::string>    mNameOffsetLocations;

    assert(strSemanticNames.size() == dwSignatures);
    assert(signatures.size() == dwSignatures);

    if (strSemanticNames.size() != dwSignatures || signatures.size() != dwSignatures)
    {
        return NULL;
    }

    // Write chunk header
    buffer.Write(dwChunkType);

    size_t nChunkSizePosition = buffer.GetOffset(); // Store the offset to the chunk size so that we can update it with the size later
    buffer.Write((DWORD) 0);

    // Write xSGN header
    D3D10_xSGN_Header SGN_Header;
    SGN_Header.dwSignatures = dwSignatures;
    SGN_Header.dwReserved = 8;
    buffer.Write(SGN_Header);

    // Iterate over signatures
    for (unsigned int i = 0; i < dwSignatures; i++)
    {
        // add semantic name to list of names
        const std::string& strName = strSemanticNames[i];
        namePacker.Add(strName);

        // save the offset where signature will be written
        size_t nOffsetLocation = buffer.GetOffset();
        mNameOffsetLocations[nOffsetLocation] = strName;

        // write the signature
        buffer.Write(signatures[i]);
    }

    // Output the signature names
    for (unsigned int i = 0; i < dwSignatures; i++)
    {
        const std::string& strName = strSemanticNames[i];
        namePacker.Write(strName, buffer);
    }

    buffer.PadToDoubleWord(PAD_BYTE);

    // fill in the correct offset lcations for each signature
    typedef std::pair<size_t, std::string> NameOffsetLocation;
    BOOST_FOREACH(NameOffsetLocation offsetLocation, mNameOffsetLocations)
    {
        buffer.WriteAt(offsetLocation.first, namePacker.GetOffset(offsetLocation.second));
    }

    // fill in the size of the chunk
    buffer.WriteAt(nChunkSizePosition, buffer.GetOffset() - sizeof(D3D10_ChunkHeader));

    return CopyChunk((D3D10_ChunkHeader*) buffer.GetBuffer());
}


void CD3D10ShaderObject::ClearInputSignatures()
{
    m_dwInputSignatures = 0;
    m_dwInputReserved = 0;
    m_strISGNSemanticNames.clear();
    m_InputSignatures.clear();
}


bool CD3D10ShaderObject::ParseISGNChunk()
{
    ClearInputSignatures();

    return ParseSGNChunk(m_pISGNChunk, m_dwInputSignatures, m_dwInputReserved, m_strISGNSemanticNames, m_InputSignatures);
}


bool CD3D10ShaderObject::GenerateISGNChunk()
{
    SAFE_FREE(m_pISGNChunk);

    m_pISGNChunk = GenerateSGNChunk(MAKEFOURCC('I', 'S', 'G', 'N'), m_dwInputSignatures, m_strISGNSemanticNames, m_InputSignatures);

    assert(m_pISGNChunk != NULL);

    return (m_pISGNChunk != NULL);
}


void CD3D10ShaderObject::ClearOutputSignatures()
{
    m_dwOutputSignatures = 0;
    m_dwOutputReserved = 0;
    m_strOSGNSemanticNames.clear();
    m_OutputSignatures.clear();
    m_dwRenderTargets = 0;
}


bool CD3D10ShaderObject::AddInputSignature(const std::string strName, const D3D10_Signature signature)
{
    m_dwInputSignatures++;
    m_strISGNSemanticNames.push_back(strName);
    m_InputSignatures.push_back(signature);
    return true;
}


bool CD3D10ShaderObject::AddOutputSignature(const std::string strName, const D3D10_Signature signature)
{
    m_dwOutputSignatures++;
    m_strOSGNSemanticNames.push_back(strName);
    m_OutputSignatures.push_back(signature);

    if (signature.dwSystemValueType == D3D10_NAME_TARGET)
    {
        m_dwRenderTargets++;
    }

    return true;
}


bool CD3D10ShaderObject::ParseOSGNChunk()
{
    ClearOutputSignatures();

    if (ParseSGNChunk(m_pOSGNChunk, m_dwOutputSignatures, m_dwOutputReserved, m_strOSGNSemanticNames, m_OutputSignatures))
    {
        BOOST_FOREACH(D3D10_Signature outputSignature, GetOutputSignatures())
        {
            if (outputSignature.dwSystemValueType == D3D10_NAME_TARGET)
            {
                m_dwRenderTargets++;
            }
        }

        return true;
    }

    return false;
}


bool CD3D10ShaderObject::GenerateOSGNChunk()
{
    SAFE_FREE(m_pOSGNChunk);

    m_pOSGNChunk = GenerateSGNChunk(MAKEFOURCC('O', 'S', 'G', 'N'), m_dwOutputSignatures, m_strOSGNSemanticNames, m_OutputSignatures);

    assert(m_pOSGNChunk != NULL);

    return (m_pOSGNChunk != NULL);
}


void CD3D10ShaderObject::ClearShaderBytecode()
{
    m_dwSHDRVersion = 0;
    m_dwImmediateConstants = 0;
    m_dwIndexableTempRegisters = 0;
    m_IndexableTempRegisters.clear();
    m_dwThreadGroupSize[0] = m_dwThreadGroupSize[1] = m_dwThreadGroupSize[2];
    m_dwGlobalMemoryRegisters = 0;
    m_GlobalMemoryRegisters.clear();
}


bool CD3D10ShaderObject::ParseSHDRChunk()
{
    ClearShaderBytecode();

    if (m_pSHDRChunk && m_pSHDRChunk->dwChunkDataSize > 0)
    {
        m_dwSHDRVersion = *((DWORD*)(m_pSHDRChunk + 1));
    }

    DWORD* pdwToken = (DWORD*)(((BYTE*) m_pSHDRChunk) + sizeof(D3D10_ChunkHeader));

    DWORD dwLength = DECODE_D3D10_SB_TOKENIZED_PROGRAM_LENGTH(m_pSHDRChunk->dwChunkDataSize);

    DWORD* pdwFinish = (DWORD*)(((char*) pdwToken) + dwLength);

    /*DWORD dwVersion =*/ *pdwToken++;

    *pdwToken++;

    while (pdwToken < pdwFinish)
    {
        DWORD dwToken = *pdwToken++;

        D3D10_SB_OPCODE_TYPE opCode = DECODE_D3D10_SB_OPCODE_TYPE(dwToken);

        if (opCode == D3D10_SB_OPCODE_CUSTOMDATA)
        {
            int nCustomSize = *pdwToken++ - 2;

            D3D10_SB_CUSTOMDATA_CLASS customDataOp = DECODE_D3D10_SB_CUSTOMDATA_CLASS(dwToken);

            if (customDataOp == D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER)
            {
                m_dwImmediateConstants += (nCustomSize / 4);
            }

            pdwToken += nCustomSize;
        }
        else if (opCode == D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP)
        {
            if (DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) == 4)
            {
                D3D10_IndexableTempRegister reg;
                reg.dwIndex = *pdwToken++;
                reg.dwSize = *pdwToken++;
                reg.dwComponentCount = *pdwToken++;
                m_IndexableTempRegisters.push_back(reg);

                m_dwIndexableTempRegisters++;
            }
            else
            {
                pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
            }
        }
        else
        {
            pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
        }
    }

    return true;
}

bool CD3D10ShaderObject::ParseSHEXChunk()
{
    ClearShaderBytecode();

    if (m_pSHEXChunk && m_pSHEXChunk->dwChunkDataSize > 0)
    {
        m_dwSHDRVersion = *((DWORD*)(m_pSHEXChunk + 1));
    }

    DWORD* pdwToken = (DWORD*)(((BYTE*) m_pSHEXChunk) + sizeof(D3D10_ChunkHeader));

    DWORD dwLength = DECODE_D3D10_SB_TOKENIZED_PROGRAM_LENGTH(m_pSHEXChunk->dwChunkDataSize);

    DWORD* pdwFinish = (DWORD*)(((char*) pdwToken) + dwLength);

    /*DWORD dwVersion =*/ *pdwToken++;

    *pdwToken++;

    while (pdwToken < pdwFinish)
    {
        DWORD dwToken = *pdwToken++;

        D3D10_SB_OPCODE_TYPE opCode = DECODE_D3D10_SB_OPCODE_TYPE(dwToken);

        if (opCode == D3D10_SB_OPCODE_CUSTOMDATA)
        {
            int nCustomSize = *pdwToken++ - 2;

            D3D10_SB_CUSTOMDATA_CLASS customDataOp = DECODE_D3D10_SB_CUSTOMDATA_CLASS(dwToken);

            if (customDataOp == D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER)
            {
                m_dwImmediateConstants += (nCustomSize / 4);
            }

            pdwToken += nCustomSize;
        }
        else if (opCode == D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP)
        {
            if (DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) == 4)
            {
                D3D10_IndexableTempRegister reg;
                reg.dwIndex = *pdwToken++;
                reg.dwSize = *pdwToken++;
                reg.dwComponentCount = *pdwToken++;
                m_IndexableTempRegisters.push_back(reg);

                m_dwIndexableTempRegisters++;
            }
            else
            {
                pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
            }
        }
        else if (opCode == D3D11_SB_OPCODE_DCL_THREAD_GROUP)
        {
            //         GDT_Assert(DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) == 4);
            if (DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) == 4)
            {
                m_dwThreadGroupSize[0] = *pdwToken++;
                m_dwThreadGroupSize[1] = *pdwToken++;
                m_dwThreadGroupSize[2] = *pdwToken++;
            }
            else
            {
                pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
            }
        }
        else if (opCode == D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED)
        {
            if (DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) == 5)
            {
                D3D10_GlobalMemoryRegister reg;
                reg.dwRegister = *pdwToken++;
                reg.dwIndex = *pdwToken++;
                reg.dwStride = *pdwToken++;
                reg.dwCount = *pdwToken++;
                m_GlobalMemoryRegisters.push_back(reg);

                m_dwGlobalMemoryRegisters++;
            }
            else
            {
                pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
            }
        }
        else if (opCode == D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW)
        {
            if (DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) == 4)
            {
                D3D10_GlobalMemoryRegister reg;
                reg.dwRegister = *pdwToken++;
                reg.dwIndex = *pdwToken++;
                reg.dwCount = *pdwToken++;
                reg.dwStride = 0;
                m_GlobalMemoryRegisters.push_back(reg);

                m_dwGlobalMemoryRegisters++;
            }
            else
            {
                pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
            }
        }
        else
        {
            pdwToken += DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(dwToken) - 1;
        }
    }

    return true;
}

void CD3D10ShaderObject::ClearStatistics()
{
    m_pStats = NULL;
}

bool CD3D10ShaderObject::ParseSTATChunk()
{
    ClearStatistics();

    m_pStats = (D3D10_SHADER_STATS*)(m_pSTATChunk + 1);
    return (m_pStats != NULL);
}

void CD3D10ShaderObject::ClearDebugInfo()
{
    m_strCreator.clear();
    m_strEntrypointName.clear();
    m_strShaderTarget.clear();
    m_uiCompileFlags = 0;
    m_nEntrypointLine = 0;

    m_uiFiles = 0;
    m_strFiles.clear();
    m_strSource.clear();
    m_NumberOfLinesInFiles.clear();
    m_bGotSourceCode = false;

    m_uiInstructions = 0;
    m_Instructions.clear();

    m_uiVariables = 0;
    m_Variables.clear();

    m_uiInputVariables = 0;
    m_InputVariables.clear();

    m_uiTokens = 0;
    m_strTokens.clear();
    m_Tokens.clear();

    m_uiScopes = 0;
    m_strScopes.clear();
    m_Scopes.clear();

    m_uiScopeVariables = 0;
    m_ScopeVariables.clear();
}

bool CD3D10ShaderObject::ParseSDBGChunk()
{
    ClearDebugInfo();

    if (m_pSDBGChunk == NULL)
    {
        return false;
    }

    if (m_pSDBGChunk->dwChunkDataSize < sizeof(D3D10_SHADER_DEBUG_INFO))
    {
        return false;
    }

    // Initialize pointers to the chunk data & D3D10_SHADER_DEBUG_INFO structure
    char* pDebugInfo = (char*)(m_pSDBGChunk + 1);
    D3D10_SHADER_DEBUG_INFO& debugInfo = *(D3D10_SHADER_DEBUG_INFO*) pDebugInfo;
    char* pDebugDataOffset = pDebugInfo + debugInfo.Size;
    char* pszStringTable = pDebugDataOffset +  debugInfo.StringOffset;
    UINT* puiUintTable = (UINT*)(pDebugDataOffset + debugInfo.UintOffset);

    // Initialize pointers to the structures within the chunk data
    D3D10_SHADER_DEBUG_FILE_INFO* pFileInfo = (D3D10_SHADER_DEBUG_FILE_INFO*)(pDebugDataOffset + debugInfo.FileInfo);
    D3D10_SHADER_DEBUG_INST_INFO* pInstructionInfo = (D3D10_SHADER_DEBUG_INST_INFO*)(pDebugDataOffset + debugInfo.InstructionInfo);
    D3D10_SHADER_DEBUG_VAR_INFO* pVariableInfo = (D3D10_SHADER_DEBUG_VAR_INFO*)(pDebugDataOffset + debugInfo.VariableInfo);
    D3D10_SHADER_DEBUG_INPUT_INFO* pInputVariableInfo = (D3D10_SHADER_DEBUG_INPUT_INFO*)(pDebugDataOffset + debugInfo.InputVariableInfo);
    D3D10_SHADER_DEBUG_TOKEN_INFO* pTokenInfo = (D3D10_SHADER_DEBUG_TOKEN_INFO*)(pDebugDataOffset + debugInfo.TokenInfo);
    D3D10_SHADER_DEBUG_SCOPE_INFO* pScopeInfo = (D3D10_SHADER_DEBUG_SCOPE_INFO*)(pDebugDataOffset + debugInfo.ScopeInfo);
    D3D10_SHADER_DEBUG_SCOPEVAR_INFO* pScopeVariableInfo = (D3D10_SHADER_DEBUG_SCOPEVAR_INFO*)(pDebugDataOffset + debugInfo.ScopeVariableInfo);

    // Parse the debugInfo structure
    m_strCreator = pszStringTable + debugInfo.Creator;
    m_strEntrypointName = pszStringTable + debugInfo.EntrypointName;
    m_strShaderTarget = pszStringTable + debugInfo.ShaderTarget;
    m_uiCompileFlags = debugInfo.CompileFlags;

    // Iterate through each file
    m_uiFiles = debugInfo.Files;

    for (UINT i = 0; i < debugInfo.Files; i++)
    {
        // Read the file name. The string is not null-terminated so we need to use the file name length.
        if (pFileInfo[i].FileNameLen)
        {
            string strFileName(pszStringTable + pFileInfo[i].FileName, pszStringTable + pFileInfo[i].FileName + pFileInfo[i].FileNameLen);
            m_strFiles.push_back(strFileName);
        }
        else
        {
            m_strFiles.push_back("");
        }

        // Read the file source. The string is not null-terminated so we need to use the file length.
        vector<string> lines;

        if (pFileInfo[i].FileLen)
        {
            string strSource(pszStringTable + pFileInfo[i].FileData, pszStringTable + pFileInfo[i].FileData + pFileInfo[i].FileLen);

            // split into separate lines
            StringUtils::SplitStringIntoLines(strSource, lines);
            m_bGotSourceCode = true;
            m_strSource += strSource + "\n";
        }
        else
        {
            lines.push_back("");
        }

        m_NumberOfLinesInFiles.push_back((UINT) lines.size());
    }

    // Iterate through each instruction
    m_uiInstructions = debugInfo.Instructions;

    for (UINT i = 0; i < debugInfo.Instructions; i++)
    {
        // Read the D3D10_SHADER_DEBUG_INST_INFO structure
        m_Instructions.push_back(pInstructionInfo[i]);
    }

    // Iterate through each instruction and record the scope index
    m_ScopeIndices.resize(debugInfo.Instructions);

    for (UINT i = 0; i < debugInfo.Instructions; i++)
    {
        m_ScopeIndices[ i ].resize(pInstructionInfo[ i ].Scopes);

        // Read the scope index
        UINT* pScopeNum = (UINT*)(((char*) puiUintTable) + pInstructionInfo[ i ].ScopeInfo);

        for (UINT j = 0; j < pInstructionInfo[ i ].Scopes; j++)
        {
            m_ScopeIndices[ i ] [ j ] = *pScopeNum;
            pScopeNum++;
        }
    }

    // Iterate through each variable
    m_uiVariables = debugInfo.Variables;

    for (UINT i = 0; i < debugInfo.Variables; i++)
    {
        // Read the D3D10_SHADER_DEBUG_VAR_INFO structure
        m_Variables.push_back(pVariableInfo[i]);
    }

    // Iterate through each input variable
    m_uiInputVariables = debugInfo.InputVariables;

    for (UINT i = 0; i < debugInfo.InputVariables; i++)
    {
        // Read the D3D10_SHADER_DEBUG_INPUT_INFO structure
        m_InputVariables.push_back(pInputVariableInfo[i]);
    }

    // Iterate through each token
    m_uiTokens = debugInfo.Tokens;

    for (UINT i = 0; i < debugInfo.Tokens; i++)
    {
        // Read the token name. The string is not null-terminated so we need to use the token name length.
        if (pTokenInfo[i].TokenLength)
        {
            string strToken(pszStringTable + pTokenInfo[i].TokenId, pszStringTable + pTokenInfo[i].TokenId + pTokenInfo[i].TokenLength);
            m_strTokens.push_back(strToken);

            if (strToken.compare(m_strEntrypointName) == 0)
            {
                m_nEntrypointLine = pTokenInfo[i].Line;
                UINT nLineOffset = 0;

                for (UINT j = 0; j < pTokenInfo[i].File; j++)
                {
                    nLineOffset += m_NumberOfLinesInFiles[ j ] + 1;
                }

                m_nEntrypointLine += nLineOffset;

            }
        }
        else
        {
            m_strTokens.push_back("");
        }

        // Read the D3D10_SHADER_DEBUG_TOKEN_INFO structure
        m_Tokens.push_back(pTokenInfo[i]);
    }

    if (m_nEntrypointLine == 0 && !m_strEntrypointName.empty())
    {
        // the entry function name is not in the token list (for example GS entry functions)
        // let's scan through the source code to find the line number of the entry function
        m_nEntrypointLine = (UINT) StringUtils::FindFunctionEntryInString(m_strEntrypointName, m_strSource);
    }

    // Iterate through each scope
    m_uiScopes = debugInfo.Scopes;

    for (UINT i = 0; i < debugInfo.Scopes; i++)
    {
        // Read the scope name. The string is not null-terminated so we need to use the scope name length.
        if (pScopeInfo[i].uNameLen)
        {
            string strScope(pszStringTable + pScopeInfo[i].Name, pszStringTable + pScopeInfo[i].Name + pScopeInfo[i].uNameLen);
            m_strScopes.push_back(strScope);
        }
        else
        {
            m_strScopes.push_back("");
        }

        // Read the D3D10_SHADER_DEBUG_SCOPE_INFO structure
        m_Scopes.push_back(pScopeInfo[i]);
    }

    // Iterate through each scope variables
    m_uiScopeVariables = debugInfo.ScopeVariables;

    for (UINT i = 0; i < debugInfo.ScopeVariables; i++)
    {
        // Read the D3D10_SHADER_DEBUG_SCOPEVAR_INFO structure
        m_ScopeVariables.push_back(pScopeVariableInfo[i]);
    }

    return true;
}

bool CD3D10ShaderObject::ParseAon9Chunk()
{
    if (m_pAon9Chunk == NULL)
    {
        return false;
    }

    return true;
}

bool CD3D10ShaderObject::ParseSFI0Chunk()
{
    if (m_pSFI0Chunk == NULL)
    {
        return false;
    }

    return true;
}

bool CD3D10ShaderObject::ParseIFCEChunk()
{
    if (m_pIFCEChunk == NULL)
    {
        return false;
    }

    return true;
}

bool CD3D10ShaderObject::ParsePCSGChunk()
{
    if (m_pPCSGChunk == NULL)
    {
        return false;
    }

    return true;
}

bool CD3D10ShaderObject::ParseOSG5Chunk()
{
    if (m_pOSG5Chunk == NULL)
    {
        return false;
    }

    return true;
}

bool CD3D10ShaderObject::GetShaderSource(std::string& strShader) const
{
    strShader.clear();

    if (!m_bGotSourceCode)
    {
        return false;
    }

    strShader = m_strSource;

    return strShader.empty();
}

const D3D10_RESOURCE_RETURN_TYPE CD3D10ShaderObject::GetTextureReturnType(DWORD dwIndex) const
{
    BOOST_FOREACH(D3D10_ResourceBinding resource, m_BoundResources)
    {
        if (resource.Type == D3D10_SIT_TEXTURE && resource.dwBindPoint == dwIndex)
        {
            return resource.ReturnType;
        }
    }

    return D3D10_RETURN_TYPE_UNORM;
}

const D3D10_SRV_DIMENSION CD3D10ShaderObject::GetTextureDimension(DWORD dwIndex) const
{
    BOOST_FOREACH(D3D10_ResourceBinding resource, m_BoundResources)
    {
        if (resource.Type == D3D10_SIT_TEXTURE && resource.dwBindPoint == dwIndex)
        {
            return resource.Dimension;
        }
    }

    return D3D10_SRV_DIMENSION_UNKNOWN;
}

bool CD3D10ShaderObject::FindUnusedComputeSlot(DWORD& dwUnusedComputeSlot) const
{
    bool bInUse [ D3D11_PS_CS_UAV_REGISTER_COUNT ];
    ZeroMemory(bInUse, sizeof(bInUse));

    DWORD dwMaxRenderTarget = 0;
    BOOST_FOREACH(D3D10_Signature outputSignature, GetOutputSignatures())
    {
        if (outputSignature.dwRegister < D3D11_PS_CS_UAV_REGISTER_COUNT)
        {
            bInUse[ outputSignature.dwRegister ] = true;
            dwMaxRenderTarget = max(dwMaxRenderTarget, outputSignature.dwRegister);
        }
    }

    BOOST_FOREACH(D3D10_ResourceBinding resource, m_BoundResources)
    {
        if (D3D10ShaderUtils::IsComputeBuffer(resource.Type)  && resource.dwBindPoint < D3D11_PS_CS_UAV_REGISTER_COUNT)
        {
            bInUse[ resource.dwBindPoint ] = true;
        }
    }

    // We start our search for an open slot at the max render target as we can't interleave RTs & UAVs
    for (dwUnusedComputeSlot = dwMaxRenderTarget; dwUnusedComputeSlot < D3D11_PS_CS_UAV_REGISTER_COUNT; dwUnusedComputeSlot++)
    {
        if (bInUse[ dwUnusedComputeSlot ] == false)
        {
            return true;
        }
    }

    return false;
}

void CD3D10ShaderObject::AddRDEFName(const char* pszStringTable, DWORD dwOffset)
{
    if (pszStringTable != NULL && pszStringTable[dwOffset] != '\0')
    {
        m_strRDEFNames[dwOffset] = &pszStringTable[dwOffset];
    }
}

DWORD CD3D10ShaderObject::AddRDEFName(const char* pszName)
{
    if (pszName == NULL || *pszName == '\0')
    {
        return 0;
    }

    // Check whether the name exist already.
    map<DWORD, std::string>::const_iterator iName = m_strRDEFNames.begin();

    while (iName != m_strRDEFNames.end())
    {
        const std::string& strName = iName->second;

        if (strName.compare(pszName) == 0)
        {
            return iName->first;
        }

        iName++;
    }

    // Find a fake offset to use
    for (DWORD dwOffset = UINT_MAX; dwOffset > 0; dwOffset--)
    {
        if (m_strRDEFNames.find(dwOffset) == m_strRDEFNames.end())
        {
            m_strRDEFNames[dwOffset] = pszName;
            return dwOffset;
        }
    }

    return 0;
}

const std::string& CD3D10ShaderObject::GetRDEFName(DWORD dwOffset)
{
    return m_strRDEFNames[ dwOffset ];
}

bool CD3D10ShaderObject::AddRawUAV(const DWORD dwIndex, const char* pszName)
{
    D3D10_ResourceBinding binding;
    binding.dwNameOffset = AddRDEFName(pszName);
    binding.Type = D3D11_SIT_UAV_RWBYTEADDRESS;
    binding.ReturnType = (D3D10_RESOURCE_RETURN_TYPE) 0;
    binding.Dimension = D3D10_SRV_DIMENSION_UNKNOWN;
    binding.dwNumSamples = 0;
    binding.dwBindPoint = dwIndex;
    binding.dwBindCount = 1;
    binding.dwFlags = 0;

    m_BoundResources.push_back(binding);

    return true;
}
