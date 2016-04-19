//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute Utilities
//==============================================================================

#include "DCUtils.h"
#include "DCFuncDefs.h"
#include "DCID3D11Device_wrapper.h"
#include "DCID3D11DeviceContext_wrapper.h"
#include "..\Common\Defs.h"
#include "..\Common\Logger.h"

ID3D11Resource* DCUtils::CloneResource(ID3D11DeviceContext* pDevCntx, ID3D11Resource* pObj)
{
    ID3D11Resource* pNew = NULL;
    pDevCntx = GetRealDeviceContext11(pDevCntx);

    ID3D11Device* pDev = NULL;
    pObj->GetDevice(&pDev);
    // get the unwrapped and unpatched Device
    pDev = GetRealDevice11(pDev);

    D3D11_RESOURCE_DIMENSION Dim;
    pObj->GetType(&Dim);

    HRESULT hRes = S_OK;

    switch (Dim)
    {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
        {
            ID3D11Buffer* pBuf = (ID3D11Buffer*)pObj;

            D3D11_BUFFER_DESC Desc;
            pBuf->GetDesc(&Desc);

            ID3D11Buffer* pBufOut;

            hRes = pDev->CreateBuffer(&Desc, NULL, &pBufOut);

            pNew = pBufOut;
            break;
        }

        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            ID3D11Texture1D* pTex = (ID3D11Texture1D*)pObj;

            D3D11_TEXTURE1D_DESC Desc;
            pTex->GetDesc(&Desc);

            ID3D11Texture1D* pTex1D;
            hRes = pDev->CreateTexture1D(&Desc, NULL, &pTex1D);

            pNew = pTex1D;
            break;
        }

        case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
        {
            ID3D11Texture2D* pTex = (ID3D11Texture2D*)pObj;

            D3D11_TEXTURE2D_DESC Desc;
            pTex->GetDesc(&Desc);

            ID3D11Texture2D* pTex2D;
            hRes = pDev->CreateTexture2D(&Desc, NULL, &pTex2D);
            pNew = pTex2D;
            break;
        }

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            ID3D11Texture3D* pTex = (ID3D11Texture3D*)pObj;

            D3D11_TEXTURE3D_DESC Desc;
            pTex->GetDesc(&Desc);

            ID3D11Texture3D* pTex3D;
            hRes = pDev->CreateTexture3D(&Desc, NULL, &pTex3D);
            pNew = pTex3D;
            break;
        }
    }

    if (SUCCEEDED(hRes))
    {
        pDevCntx->CopyResource(pNew, pObj);
        pDev->Release();
        return pNew;
    }
    else
    {
        Log(logWARNING, "CloneResource() - Failed to clone resources\n");
        pDev->Release();
        return NULL;
    }


}

void DCUtils::CopyBuffer(ID3D11Resource* pObj, void* pSrc, void** pDst, UINT& size)
{
    size = GetResourceSize(pObj);
    *pDst = malloc(size * sizeof(BYTE));
    memcpy(*pDst, pSrc, size * (sizeof(BYTE)));
}


/// Helper function
UINT DCUtils::GetFormatSizeInByte(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS       :
        case DXGI_FORMAT_R32G32B32A32_FLOAT          :
        case DXGI_FORMAT_R32G32B32A32_UINT           :
        case DXGI_FORMAT_R32G32B32A32_SINT           :
            return 16;

        case DXGI_FORMAT_R32G32B32_TYPELESS          :
        case DXGI_FORMAT_R32G32B32_FLOAT             :
        case DXGI_FORMAT_R32G32B32_UINT              :
        case DXGI_FORMAT_R32G32B32_SINT              :
            return 12;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS       :
        case DXGI_FORMAT_R16G16B16A16_FLOAT          :
        case DXGI_FORMAT_R16G16B16A16_UNORM          :
        case DXGI_FORMAT_R16G16B16A16_UINT           :
        case DXGI_FORMAT_R16G16B16A16_SNORM          :
        case DXGI_FORMAT_R16G16B16A16_SINT           :
        case DXGI_FORMAT_R32G32_TYPELESS             :
        case DXGI_FORMAT_R32G32_FLOAT                :
        case DXGI_FORMAT_R32G32_UINT                 :
        case DXGI_FORMAT_R32G32_SINT                 :
        case DXGI_FORMAT_R32G8X24_TYPELESS           :
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT        :
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    :
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     :
            return 8;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS        :
        case DXGI_FORMAT_R10G10B10A2_UNORM           :
        case DXGI_FORMAT_R10G10B10A2_UINT            :
        case DXGI_FORMAT_R11G11B10_FLOAT             :
        case DXGI_FORMAT_R8G8B8A8_TYPELESS           :
        case DXGI_FORMAT_R8G8B8A8_UNORM              :
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         :
        case DXGI_FORMAT_R8G8B8A8_UINT               :
        case DXGI_FORMAT_R8G8B8A8_SNORM              :
        case DXGI_FORMAT_R8G8B8A8_SINT               :
        case DXGI_FORMAT_R16G16_TYPELESS             :
        case DXGI_FORMAT_R16G16_FLOAT                :
        case DXGI_FORMAT_R16G16_UNORM                :
        case DXGI_FORMAT_R16G16_UINT                 :
        case DXGI_FORMAT_R16G16_SNORM                :
        case DXGI_FORMAT_R16G16_SINT                 :
        case DXGI_FORMAT_R32_TYPELESS                :
        case DXGI_FORMAT_D32_FLOAT                   :
        case DXGI_FORMAT_R32_FLOAT                   :
        case DXGI_FORMAT_R32_UINT                    :
        case DXGI_FORMAT_R32_SINT                    :
        case DXGI_FORMAT_R24G8_TYPELESS              :
        case DXGI_FORMAT_D24_UNORM_S8_UINT           :
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS       :
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT        :
            return 4;

        case DXGI_FORMAT_R8G8_TYPELESS               :
        case DXGI_FORMAT_R8G8_UNORM                  :
        case DXGI_FORMAT_R8G8_UINT                   :
        case DXGI_FORMAT_R8G8_SNORM                  :
        case DXGI_FORMAT_R8G8_SINT                   :
        case DXGI_FORMAT_R16_TYPELESS                :
        case DXGI_FORMAT_R16_FLOAT                   :
        case DXGI_FORMAT_D16_UNORM                   :
        case DXGI_FORMAT_R16_UNORM                   :
        case DXGI_FORMAT_R16_UINT                    :
        case DXGI_FORMAT_R16_SNORM                   :
        case DXGI_FORMAT_R16_SINT                    :
            return 2;

        case DXGI_FORMAT_R8_TYPELESS                 :
        case DXGI_FORMAT_R8_UNORM                    :
        case DXGI_FORMAT_R8_UINT                     :
        case DXGI_FORMAT_R8_SNORM                    :
        case DXGI_FORMAT_R8_SINT                     :
        case DXGI_FORMAT_A8_UNORM                    :
        case DXGI_FORMAT_R1_UNORM                    :
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP          :
        case DXGI_FORMAT_R8G8_B8G8_UNORM             :
        case DXGI_FORMAT_G8R8_G8B8_UNORM             :
        case DXGI_FORMAT_BC1_TYPELESS                :
        case DXGI_FORMAT_BC1_UNORM                   :
        case DXGI_FORMAT_BC1_UNORM_SRGB              :
        case DXGI_FORMAT_BC2_TYPELESS                :
        case DXGI_FORMAT_BC2_UNORM                   :
        case DXGI_FORMAT_BC2_UNORM_SRGB              :
        case DXGI_FORMAT_BC3_TYPELESS                :
        case DXGI_FORMAT_BC3_UNORM                   :
        case DXGI_FORMAT_BC3_UNORM_SRGB              :
        case DXGI_FORMAT_BC4_TYPELESS                :
        case DXGI_FORMAT_BC4_UNORM                   :
        case DXGI_FORMAT_BC4_SNORM                   :
        case DXGI_FORMAT_BC5_TYPELESS                :
        case DXGI_FORMAT_BC5_UNORM                   :
        case DXGI_FORMAT_BC5_SNORM                   :
        case DXGI_FORMAT_B5G6R5_UNORM                :
        case DXGI_FORMAT_B5G5R5A1_UNORM              :
        case DXGI_FORMAT_B8G8R8A8_UNORM              :
        case DXGI_FORMAT_B8G8R8X8_UNORM              :
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  :
        case DXGI_FORMAT_B8G8R8A8_TYPELESS           :
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         :
        case DXGI_FORMAT_B8G8R8X8_TYPELESS           :
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         :
        case DXGI_FORMAT_BC6H_TYPELESS               :
        case DXGI_FORMAT_BC6H_UF16                   :
        case DXGI_FORMAT_BC6H_SF16                   :
        case DXGI_FORMAT_BC7_TYPELESS                :
        case DXGI_FORMAT_BC7_UNORM                   :
        case DXGI_FORMAT_BC7_UNORM_SRGB              :
        case DXGI_FORMAT_FORCE_UINT                  :
            return 1;

        /*case DXGI_FORMAT_UNKNOWN:    */
        default:
            assert(!"UNKNOWN TYPE");
            return 0;
    }
}

UINT DCUtils::GetResourceSize(ID3D11Resource* pResource)
{
    assert(pResource);
    D3D11_RESOURCE_DIMENSION type;
    pResource->GetType(&type);
    ID3D11Buffer* pBuf;
    ID3D11Texture1D* pTex1D;
    ID3D11Texture2D* pTex2D;
    ID3D11Texture3D* pTex3D;

    switch (type)
    {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
            pBuf = (ID3D11Buffer*)pResource;
            D3D11_BUFFER_DESC bufDesc;
            pBuf->GetDesc(&bufDesc);
            return bufDesc.ByteWidth;
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            pTex1D = (ID3D11Texture1D*)pResource;
            D3D11_TEXTURE1D_DESC tex1DDesc;
            pTex1D->GetDesc(&tex1DDesc);
            return tex1DDesc.Width * GetFormatSizeInByte(tex1DDesc.Format);
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            pTex2D = (ID3D11Texture2D*)pResource;
            D3D11_TEXTURE2D_DESC tex2DDesc;
            pTex2D->GetDesc(&tex2DDesc);
            return tex2DDesc.Width * tex2DDesc.Height * GetFormatSizeInByte(tex2DDesc.Format);
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            pTex3D = (ID3D11Texture3D*)pResource;
            D3D11_TEXTURE3D_DESC tex3DDesc;
            pTex3D->GetDesc(&tex3DDesc);
            return tex3DDesc.Width * tex3DDesc.Height * tex3DDesc.Depth * GetFormatSizeInByte(tex3DDesc.Format);
            break;

        default:
            assert(!"GetResourceSize() Unknown resource type");
            break;
    }

    return 0;
}

