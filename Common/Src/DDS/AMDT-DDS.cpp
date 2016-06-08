//==============================================================================
// Copyright (c) 2009-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  TODO
//==============================================================================

#include <algorithm>

#if defined (_WIN32)
    #include <Windows.h>
    #include <tchar.h>
#else
    #include <stdio.h>
    #include <string.h>
    typedef unsigned int UINT32;
    typedef char TCHAR;
#endif

#include "GDT_MemUtils.h"
#include "DDS_Internal.h"
#include "AMDT-DDS.h"

using namespace DDS;

DDS_Error DDS::SaveDDSToFile(UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nSubResourceCount, UINT32 nMipMapCount, DDS_Type eType, DDS_Format eFormat, void* pData, size_t nSize, const TCHAR* pszFileName)
{
    void* pDDSData = NULL;
    size_t nDDSSize = 0;

    DDS_Error err = SaveDDSToMemory(nWidth, nHeight, nDepth, nSubResourceCount, nMipMapCount, eType, eFormat, pData, nSize, &pDDSData, &nDDSSize);

    if (err == Err_Ok)
    {
        FILE* pFile = NULL;

#if defined (_WIN32)
        _tfopen_s(&pFile, pszFileName, _T("wb"));
#else
        pFile = fopen(pszFileName, "wb");
#endif

        if (pFile == NULL)
        {
            err = Err_Unknown;
        }
        else
        {
            if (fwrite(pDDSData, nDDSSize, 1,  pFile) != 1)
            {
                err = Err_Unknown;
            }

            fclose(pFile);
        }
    }

    GDT_SAFE_FREE(pDDSData);

    return err;
}

DDS_Error CheckValid(UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nSubResourceCount, UINT32 nMipMapCount, DDS_Type eType, DDS_Format eFormat, void* pData, size_t nSize)
{
    if (nWidth == 0)
    {
        return Err_Unknown;
    }

    if ((eType != Texture1D) && nHeight == 0)
    {
        return Err_Unknown;
    }

    if ((eType == Texture3D) && nDepth == 0)
    {
        return Err_Unknown;
    }

    if ((eType != TextureCubeMap) && nSubResourceCount == 0)
    {
        return Err_Unknown;
    }

    if ((eType == TextureCubeMap) && (nSubResourceCount != 1 && nSubResourceCount != 6))
    {
        return Err_Unknown;
    }

    if (eType <= UnknownType || eType > TextureCubeMap)
    {
        return Err_Unknown;
    }

    DWORD nBytesPerPixel = 0;

    switch (eFormat)
    {
        case RGBA_32F:
            nBytesPerPixel = 16;
            break;

        case RGBA_8:
            nBytesPerPixel = 4;
            break;

        case R_8:
            nBytesPerPixel = 1;
            break;

        case R_32F:
        case D_32F:
            nBytesPerPixel = 4;
            break;

        case RG_8:
        case RG_S8:
            nBytesPerPixel = 2;
            break;

        case D24_S8:
            nBytesPerPixel = 4;
            break;

        case L8_A8:
            nBytesPerPixel = 2;
            break;

        default:
            return Err_Unknown;
    }

    size_t nRequiredSize = 0;

    switch (eType)
    {

        case Texture1D:

            for (UINT32 i = 0 ; i < nMipMapCount; i++)
            {
                nRequiredSize += nWidth * nSubResourceCount * nBytesPerPixel;
                nWidth = std::max<UINT32>(nWidth / 2, 1);
            }

            break;

        case Texture2D:

            for (UINT32 i = 0 ; i < nMipMapCount; i++)
            {
                nRequiredSize += nWidth * nHeight * nSubResourceCount * nBytesPerPixel;
                nHeight = std::max<UINT32>(nHeight / 2, 1);
                nWidth = std::max<UINT32>(nWidth / 2, 1);
            }

            break;

        case Texture3D:

            for (UINT32 i = 0 ; i < nMipMapCount; i++)
            {
                nRequiredSize += nWidth * nHeight * nDepth * nSubResourceCount * nBytesPerPixel;
                nHeight = std::max<UINT32>(nHeight / 2, 1);
                nWidth = std::max<UINT32>(nWidth / 2, 1);
                nDepth = std::max<UINT32>(nDepth / 2, 1);   //As the mip XY resolution halves so does the depth.
            }

            break;

        case TextureCubeMap:
            for (UINT32 i = 0 ; i < nMipMapCount; i++)
            {
                nRequiredSize += nWidth * nHeight * 6 * nBytesPerPixel;
                nHeight = std::max<UINT32>(nHeight / 2, 1);
                nWidth = std::max<UINT32>(nWidth / 2, 1);
            }

            break;

        case UnknownType:
            return Err_Unknown;
    }

    if (nRequiredSize != nSize)
    {
        return Err_Unknown;
    }

    if (pData == NULL)
    {
        return Err_Unknown;
    }

    return Err_Ok;
}

DDS_Error DDS::SaveDDSToMemory(UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nSubResourceCount, UINT32 nMipMapCount, DDS_Type eType, DDS_Format eFormat, void* pData, size_t nSize, void** ppDDSData, size_t* pnDDSSize)
{
    if (ppDDSData == NULL)
    {
        return Err_Unknown;
    }

    if (pnDDSSize == NULL)
    {
        return Err_Unknown;
    }

    *ppDDSData = NULL;
    *pnDDSSize = 0;

    // Check data validity here
    DDS_Error err = CheckValid(nWidth, nHeight, nDepth, nSubResourceCount, nMipMapCount, eType, eFormat, pData, nSize);

    if (err != Err_Ok)
    {
        return err;
    }

    // Arrays for cubemaps are not supported so enforce that the nSubResourceCount is 1 rather than 6 (ie. num faces)
    if (eType == TextureCubeMap)
    {
        nSubResourceCount = 1;
    }

    // Important dont forget to include the DX10 Header texture types in the folowing check. these are texture types that can use the DX10 Header data.
    bool bNeedDX10Header = false;

    if (nSubResourceCount > 1 ||
        eFormat == D_32F ||
        eFormat == R_32F ||
        eFormat == RG_8 ||
        eFormat == RG_S8 ||
        eFormat == D24_S8 ||
        eFormat == L8_A8)
    {
        bNeedDX10Header = true;
    }

    size_t nTotalSize = *pnDDSSize = sizeof(DWORD) + sizeof(DDS_HEADER) + (bNeedDX10Header ? sizeof(DDS_HEADER_DXT10) : 0) + nSize;

    *ppDDSData = malloc(nTotalSize);

    if (*ppDDSData == NULL)
    {
        return Err_Unknown;
    }

    char* pDDSData = (char*) * ppDDSData;

    DWORD* pdwMagic = (DWORD*) pDDSData;
    pDDSData += sizeof(DWORD);
    *pdwMagic = DDS_MAGIC;

    DDS_HEADER* pHeader = (DDS_HEADER*) pDDSData;
    pDDSData += sizeof(DDS_HEADER);

#if defined (_WIN32)
    GDT_ZERO_MEMORY_PTR(pHeader);
#else
    memset(pHeader, 0, sizeof(DDS_HEADER));
#endif
    // according the the Microsoft documentation, the header dwSize field needs to be the size of the DDS_HEADER
    // struct (124 bytes)
    // source: http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982%28v=vs.85%29.aspx
    pHeader->dwSize = (DWORD) sizeof(DDS_HEADER);     //nTotalSize;
    pHeader->dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | (eType == Texture3D ? DDS_HEADER_FLAGS_VOLUME : 0);
    pHeader->dwHeight = nHeight;
    pHeader->dwWidth = nWidth;
    //   pHeader->dwPitchOrLinearSize;

    if (eType == Texture3D)
    {
        pHeader->dwDepth = nDepth;
    }
    else
    {
        pHeader->dwDepth = 0;
    }

    pHeader->dwMipMapCount = nMipMapCount;
    pHeader->dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;

    if (eType == TextureCubeMap)
    {
        pHeader->dwSurfaceFlags |= DDS_SURFACE_FLAGS_CUBEMAP;
        pHeader->dwCubemapFlags = DDS_CUBEMAP_ALLFACES;
    }

    if (bNeedDX10Header)
    {
        memcpy(&pHeader->ddspf, &DDSPF_DX10, sizeof(DDS_PIXELFORMAT));

        DDS_HEADER_DXT10* pDX10Header = (DDS_HEADER_DXT10*) pDDSData;
        pDDSData += sizeof(DDS_HEADER_DXT10);

#if defined (_WIN32)
        GDT_ZERO_MEMORY_PTR(pDX10Header);
#else
        memset(pDX10Header, 0, sizeof(DDS_HEADER_DXT10));
#endif

        switch (eFormat)
        {
            case RGBA_32F:
                pDX10Header->dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
                break;

            case RGBA_8:
                pDX10Header->dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
                break;

            case R_8:
                pDX10Header->dxgiFormat = DXGI_FORMAT_R8_UNORM;
                break;

            case RG_8:
                pDX10Header->dxgiFormat = DXGI_FORMAT_R8G8_UINT;
                break;

            case RG_S8:
                pDX10Header->dxgiFormat = DXGI_FORMAT_R8G8_SINT;
                break;

            case D_32F:
                pDX10Header->dxgiFormat = DXGI_FORMAT_D32_FLOAT;
                break;

            case R_32F:
                pDX10Header->dxgiFormat = DXGI_FORMAT_R32_FLOAT;
                break;

            case D24_S8:
                pDX10Header->dxgiFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
                break;

            case L8_A8:
                // NOTE: there is no DXGI format for L8_A8 so we are hijacking the DXGI_FORMAT_R8G8_UINT32 format
                // The reverse is done on the client.
                pDX10Header->dxgiFormat = DXGI_FORMAT_R8G8_UINT;
                break;

            default:
                pDX10Header->dxgiFormat = DXGI_FORMAT_UNKNOWN;
                break;
        }

        switch (eType)
        {
            case Texture1D:
                pDX10Header->resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE1D;
                break;

            case Texture2D:
                pDX10Header->resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
                break;

            case Texture3D:
                pDX10Header->resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE3D;
                break;

            case TextureCubeMap:
                pDX10Header->resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
                pDX10Header->miscFlag |= D3D10_RESOURCE_MISC_TEXTURECUBE;
                break;

            case UnknownType:
                break;
        }

        pDX10Header->arraySize = nSubResourceCount;
    }
    else
    {
        switch (eFormat)
        {
            case RGBA_32F:
                memcpy(&pHeader->ddspf, &DDSPF_A32B32G32R32F, sizeof(DDS_PIXELFORMAT));
                break;

            case RGBA_8:
                memcpy(&pHeader->ddspf, &DDSPF_A8R8G8B8, sizeof(DDS_PIXELFORMAT));
                break;

            case R_8:
                memcpy(&pHeader->ddspf, &DDSPF_R8, sizeof(DDS_PIXELFORMAT));
                break;

            case RG_8:
            case RG_S8:
            case D_32F:
            case R_32F:
            case D24_S8:
                memcpy(&pHeader->ddspf, &DDSPF_DX10, sizeof(DDS_PIXELFORMAT));
                break;

            case L8_A8:
                memcpy(&pHeader->ddspf, &DDSPF_L8A8, sizeof(DDS_PIXELFORMAT));
                break;

            case UnknownFormat:
                break;

        }
    }

    memcpy(pDDSData, pData, nSize);

    return Err_Ok;
}
