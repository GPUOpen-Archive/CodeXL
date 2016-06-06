//=====================================================================
// Copyright 2007-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file D3D10Utils.cpp
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10Utils.cpp#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#include <Windows.h>
#include "D3D10Utils.h"
#include <d3dx10.h>
#include <d3d9.h>
#include <ddraw.h>

bool D3D10Utils::SaveTextureAsDDS(ID3D10Texture2D* pTexture, const std::string& strFilePath)
{
    if (strFilePath.empty())
    {
        return false;
    }

    D3D10_TEXTURE2D_DESC desc;
    pTexture->GetDesc(&desc);

    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    ddsd.dwWidth = desc.Width;
    ddsd.dwHeight = desc.Height;
    ddsd.dwMipMapCount = desc.MipLevels;
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

    //    if(pMipSet->m_TextureType == TT_CubeMap)
    //    {
    //       ddsd2.ddsCaps.dwCaps2 |= DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES;
    //    }
    //    else if(pMipSet->m_TextureType == TT_VolumeTexture)
    //    {
    //       ddsd2.dwFlags |= DDSD_DEPTH;
    //       ddsd2.dwDepth = pMipSet->m_nDepth;
    //       ddsd2.ddsCaps.dwCaps2 |= DDSCAPS2_VOLUME;
    //    }

    if (ddsd.dwMipMapCount > 1)
    {
        ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP;
    }

    switch (desc.Format)
    {
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC | DDPF_ALPHAPIXELS;
            ddsd.ddpfPixelFormat.dwFourCC = D3DFMT_A32B32G32R32F;
            break;

        case DXGI_FORMAT_R8G8B8A8_UINT:
            ddsd.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
            ddsd.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
            ddsd.ddpfPixelFormat.dwBBitMask = 0x000000ff;
            ddsd.ddpfPixelFormat.dwRGBBitCount = 24;
            ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000;
            break;

        case DXGI_FORMAT_R8_UINT:
            ddsd.ddpfPixelFormat.dwFlags = DDPF_ALPHA;
            ddsd.ddpfPixelFormat.dwAlphaBitDepth = 8;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff;
            break;

        default:
            return false;
    }

    UINT uiSubresource = 0  ; // D3D10CalcSubresource
    D3D10_MAPPED_TEXTURE2D rect;

    if (FAILED(pTexture->Map(uiSubresource, D3D10_MAP_READ, 0, &rect)))
    {
        return false;
    }

    ddsd.lPitch = rect.RowPitch;
    DWORD dwDataSize = ddsd.lPitch * ddsd.dwHeight;

    FILE* pFile;

    if (fopen_s(&pFile, strFilePath.c_str(), "wb") != 0)
    {
        pTexture->Unmap(uiSubresource);
        return false;
    }

    DWORD dwMagicNumber = MAKEFOURCC('D', 'D', 'S', ' ');

    fwrite(&dwMagicNumber, sizeof(dwMagicNumber), 1, pFile);

    fwrite(&ddsd, sizeof(ddsd), 1, pFile);

    fwrite(rect.pData, dwDataSize, 1, pFile);

    fclose(pFile);

    pTexture->Unmap(uiSubresource);

    return true;
}
