//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File contains the implementation of the DX11StatusKeeper class, that
//         is used for keeping device seting. Constructor saves them, destructor set them back.
//         Stole from GPS2 and modified
//==============================================================================

#include "DCStatusKeeper.h"
#include "D3D11.h"
#include "DCAuto.h"
#include <assert.h>

DX11Status::DX11Status()
    :
    m_pDevice(NULL),
    m_pOldDepthStencilView(NULL),
    m_pOldInputLayout(NULL),
    m_pOldRasterState(NULL),
    m_oldTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED) ,
    m_pOldDepthStencilState(NULL),
    m_nOldStencilRef(0),
    m_pOldPixelShader(NULL),
    m_pOldGeometryShader(NULL),
    m_pOldVertexShader(NULL),
    m_pOldIndexBuffer(NULL),
    m_OldIndexBufferFormat(DXGI_FORMAT_UNKNOWN),
    m_OldIndexBufferOffset(0),
    m_pOldPredicate(NULL),
    m_bOldPredicateValue(false),
    m_bCaptured(false),
    m_pBlendState(NULL),
    m_nOldVpNum(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE),
    m_nOldNumScissorRects(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
{
    for (int i = 0; i < D3D11_SO_BUFFER_SLOT_COUNT; i++)
    {
        m_pSOBuffers[i] = NULL;
        m_SOOffsets[i] = 0;
    }

    for (unsigned int i = 0 ; i < DX11_MAX_RENDERTARGETS; i++)
    {
        m_pOldRenderTargetViews[ i ] = NULL;
    }

    for (int i = 0; i < DX11_MAX_TEXTURES; i++)
    {
        m_pVSResourceViews[i] = NULL;
        m_pGSResourceViews[i] = NULL;
        m_pPSResourceViews[i] = NULL;
        m_pHSResourceViews[i] = NULL;
        m_pDSResourceViews[i] = NULL;
        m_pCSResourceViews[i] = NULL;
    }

    for (int i = 0 ; i < D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE ; i++)
    {
        m_oldViewPorts[i].Height = 200;
        m_oldViewPorts[i].Width = 200;
        m_oldViewPorts[i].TopLeftX = 0;
        m_oldViewPorts[i].TopLeftY = 0;
        m_oldViewPorts[i].MinDepth = 0.0f;
        m_oldViewPorts[i].MaxDepth = 0.0f;
    }

    for (int i = 0 ; i < D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE ; i++)
    {
        m_pOldScissorRects[i].bottom = 0;
        m_pOldScissorRects[i].left = 0;
        m_pOldScissorRects[i].right = 0;
        m_pOldScissorRects[i].top = 0;
    }

    for (unsigned int i = 0 ; i < DX11_MAX_VERTEXBUFFERCOUNT; i++)
    {
        m_pOldVertexBuffers[i] = NULL;
        m_nOldVertexBufferStrides[i] = 0 ;
        m_nOldVertexBufferOffsets[i] = 0 ;
    }

    for (unsigned int i = 0 ; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++)
    {
        m_pCSUnorderedAccessViews[i] = NULL;
        m_pOMUnorderedAccessViews[i] = NULL;
    }

    for (unsigned int i = 0 ; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++)
    {
        m_PSSamplers[i] = NULL ;
        m_GSSamplers[i] = NULL ;
        m_VSSamplers[i] = NULL ;
        m_HSSamplers[i] = NULL ;
        m_DSSamplers[i] = NULL ;
        m_CSSamplers[i] = NULL ;
    }
}

DX11Status::~DX11Status()
{
}

void DX11Status::Capture(ID3D11Device* pDevice)
{
    assert(pDevice != NULL);

    if (pDevice == NULL)
    {
        //Log( logERROR, "DX11Status: trying to capture NULL device.\n" );
        return ;
    }

    m_pDevice = pDevice;

    ImmediateContext IC(m_pDevice);

    for (int i = 0; i < DX11_MAX_RENDERTARGETS; i++)
    {
        m_pOldRenderTargetViews[ i ]  = NULL;
    }

    m_pOldDepthStencilView = NULL;
    m_pOldInputLayout = NULL;
    m_pOldDepthStencilState = NULL;
    m_nOldStencilRef = 0;
    m_pOldPixelShader = NULL;
    m_pOldGeometryShader = NULL;
    m_pOldVertexShader = NULL;

    for (int i = 0; i < DX11_MAX_VERTEXBUFFERCOUNT; i++)
    {
        m_pOldVertexBuffers [i] = NULL;
    }

    m_nOldVpNum = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    IC->RSGetViewports(&m_nOldVpNum, m_oldViewPorts);

    m_nOldNumScissorRects = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    IC->RSGetScissorRects(&m_nOldNumScissorRects, m_pOldScissorRects);

    // Commented out as the documentation is vague and the call causes a crash in OIT demo.
    IC->OMGetRenderTargetsAndUnorderedAccessViews(DX11_MAX_RENDERTARGETS,
                                                  m_pOldRenderTargetViews,
                                                  &m_pOldDepthStencilView,
                                                  0,
                                                  D3D11_PS_CS_UAV_REGISTER_COUNT,
                                                  m_pOMUnorderedAccessViews);

    //IC->OMGetRenderTargets( DX11_MAX_RENDERTARGETS,
    //                        m_pOldRenderTargetViews,
    //                        &m_pOldDepthStencilView );

    IC->VSGetShaderResources(0, DX11_MAX_TEXTURES, m_pVSResourceViews);
    IC->GSGetShaderResources(0, DX11_MAX_TEXTURES, m_pGSResourceViews);
    IC->PSGetShaderResources(0, DX11_MAX_TEXTURES, m_pPSResourceViews);
    IC->HSGetShaderResources(0, DX11_MAX_TEXTURES, m_pHSResourceViews);
    IC->DSGetShaderResources(0, DX11_MAX_TEXTURES, m_pDSResourceViews);
    IC->CSGetShaderResources(0, DX11_MAX_TEXTURES, m_pCSResourceViews);
    IC->CSGetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, m_pCSUnorderedAccessViews);

    IC->IAGetInputLayout(&m_pOldInputLayout);
    IC->IAGetPrimitiveTopology(&m_oldTopology);
    IC->IAGetVertexBuffers(0, DX11_MAX_VERTEXBUFFERCOUNT, m_pOldVertexBuffers, m_nOldVertexBufferStrides, m_nOldVertexBufferOffsets);
    IC->IAGetIndexBuffer(&m_pOldIndexBuffer, &m_OldIndexBufferFormat, &m_OldIndexBufferOffset);

    IC->RSGetState(&m_pOldRasterState);
    IC->OMGetDepthStencilState(&m_pOldDepthStencilState, &m_nOldStencilRef);
    IC->OMGetBlendState(&m_pBlendState, m_BlendFactor, &m_pSampleMask) ;

    IC->GSGetShader(&m_pOldGeometryShader, NULL, 0);
    IC->PSGetShader(&m_pOldPixelShader, NULL, 0);
    IC->VSGetShader(&m_pOldVertexShader, NULL, 0);
    IC->HSGetShader(&m_pOldHullShader, NULL, 0);
    IC->DSGetShader(&m_pOldDomainShader, NULL, 0);
    IC->CSGetShader(&m_pOldComputeShader, NULL, 0);

    IC->SOGetTargets(D3D11_SO_BUFFER_SLOT_COUNT, m_pSOBuffers);
    IC->GetPredication(&m_pOldPredicate, &m_bOldPredicateValue);

    // Buffer the samplers
    IC->PSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_PSSamplers);
    IC->GSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_GSSamplers);
    IC->VSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_VSSamplers);
    IC->HSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_HSSamplers);
    IC->DSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_DSSamplers);
    IC->CSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_CSSamplers);

    m_bCaptured = true;
}

void DX11Status::RestoreAndRelease()
{
    Restore();

    SAFE_RELEASE(m_pOldPredicate);

    for (int i = 0; i < D3D11_SO_BUFFER_SLOT_COUNT; i++)
    {
        SAFE_RELEASE(m_pSOBuffers[i]);
        m_SOOffsets[i] = 0;
    }

    SAFE_RELEASE(m_pOldInputLayout);

    for (int i = 0; i < DX11_MAX_RENDERTARGETS; i++)
    {
        SAFE_RELEASE(m_pOldRenderTargetViews[i]);
    }

    for (int i = 0; i < DX11_MAX_TEXTURES; i++)
    {
        SAFE_RELEASE(m_pVSResourceViews[i]);
        SAFE_RELEASE(m_pGSResourceViews[i]);
        SAFE_RELEASE(m_pPSResourceViews[i]);
        SAFE_RELEASE(m_pHSResourceViews[i]);
        SAFE_RELEASE(m_pDSResourceViews[i]);
        SAFE_RELEASE(m_pCSResourceViews[i]);
    }

    SAFE_RELEASE(m_pOldDepthStencilView);
    SAFE_RELEASE(m_pOldRasterState);
    SAFE_RELEASE(m_pOldDepthStencilState);
    SAFE_RELEASE(m_pBlendState);
    SAFE_RELEASE(m_pOldGeometryShader);
    SAFE_RELEASE(m_pOldPixelShader);
    SAFE_RELEASE(m_pOldVertexShader);
    SAFE_RELEASE(m_pOldHullShader);
    SAFE_RELEASE(m_pOldDomainShader);
    SAFE_RELEASE(m_pOldComputeShader);

    for (int i = 0; i < DX11_MAX_VERTEXBUFFERCOUNT; i++)
    {
        SAFE_RELEASE(m_pOldVertexBuffers [i]);
    }

    SAFE_RELEASE(m_pOldIndexBuffer);

    for (unsigned int i = 0 ; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++)
    {
        SAFE_RELEASE(m_pCSUnorderedAccessViews[i]);
        SAFE_RELEASE(m_pOMUnorderedAccessViews[i]);
    }

    for (unsigned int i = 0 ; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++)
    {
        SAFE_RELEASE(m_PSSamplers[i]);
        SAFE_RELEASE(m_GSSamplers[i]);
        SAFE_RELEASE(m_VSSamplers[i]);
        SAFE_RELEASE(m_HSSamplers[i]);
        SAFE_RELEASE(m_DSSamplers[i]);
        SAFE_RELEASE(m_CSSamplers[i]);
    }

    m_bCaptured = false;
}

void DX11Status::Restore()
{
    assert(m_pDevice != NULL);

    assert(m_bCaptured == true);

    if (m_bCaptured == false)
    {
        //Log( logERROR, "DX11Status: trying to restore state before capture has occurred.\n" );
        return ;
    }

    assert(m_pDevice != NULL);

    if (m_pDevice == NULL)
    {
        //Log( logERROR, "DX11Status: trying to restore NULL device.\n" );
        return ;
    }

    ImmediateContext IC(m_pDevice);

    IC->SetPredication(m_pOldPredicate, m_bOldPredicateValue);
    IC->SOSetTargets(D3D11_SO_BUFFER_SLOT_COUNT, m_pSOBuffers, m_SOOffsets);
    IC->RSSetViewports(m_nOldVpNum, m_oldViewPorts);
    IC->RSSetScissorRects(m_nOldNumScissorRects, m_pOldScissorRects);

    // Keep commented out for now - required in OMSetRenderTargetsAndUnorderedAccessViews
    // Use existing offset. We need to use a real array instead of just a pointer.
    UINT nUAVInitialCounts [D3D11_PS_CS_UAV_REGISTER_COUNT];

    for (UINT i = 0 ; i < D3D11_PS_CS_UAV_REGISTER_COUNT ; i++)
    {
        nUAVInitialCounts[i] = 0xffff;
    }


    // Copy UAVs back into an array that we can use to reset the original app UAVs
    // Set up an array to copy the UAV pointers into
    ID3D11UnorderedAccessView* pUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];

    for (UINT i = 0 ; i < D3D11_PS_CS_UAV_REGISTER_COUNT ; i++)
    {
        pUAVs[i] = NULL;
    }

    UINT nUAVCount = 0;

    // Find the UAvs and copy them into the new array starting at index 0.
    for (UINT i = 0 ; i < D3D11_PS_CS_UAV_REGISTER_COUNT ; i++)
    {
        if (m_pOMUnorderedAccessViews[i] != NULL)
        {
            pUAVs[nUAVCount++] =  m_pOMUnorderedAccessViews[i];
        }
    }

    // Calculate the UAV offset by counting the number of RTVs.
    UINT nRTVOffset = 0 ;

    for (UINT i = 0 ; i < DX11_MAX_RENDERTARGETS ; i++)
    {
        if (m_pOldRenderTargetViews[i] != NULL)
        {
            nRTVOffset++;
        }
    }

    // Now we can reset the RTV/DSV/UAV data
    IC->OMSetRenderTargetsAndUnorderedAccessViews(DX11_MAX_RENDERTARGETS,
                                                  m_pOldRenderTargetViews,
                                                  m_pOldDepthStencilView,
                                                  nRTVOffset,
                                                  nUAVCount,
                                                  pUAVs,
                                                  &nUAVInitialCounts[0]);

    // Keep in for now but commented out.
    //IC->OMSetRenderTargets( DX11_MAX_RENDERTARGETS,
    //                        m_pOldRenderTargetViews,
    //                        m_pOldDepthStencilView );

    IC->VSSetShaderResources(0, DX11_MAX_TEXTURES, m_pVSResourceViews);
    IC->GSSetShaderResources(0, DX11_MAX_TEXTURES, m_pGSResourceViews);
    IC->PSSetShaderResources(0, DX11_MAX_TEXTURES, m_pPSResourceViews);
    IC->HSSetShaderResources(0, DX11_MAX_TEXTURES, m_pHSResourceViews);
    IC->DSSetShaderResources(0, DX11_MAX_TEXTURES, m_pDSResourceViews);
    IC->CSSetShaderResources(0, DX11_MAX_TEXTURES, m_pCSResourceViews);
    IC->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, m_pCSUnorderedAccessViews, NULL);

    IC->IASetInputLayout(m_pOldInputLayout);
    IC->IASetPrimitiveTopology(m_oldTopology);

    IC->RSSetState(m_pOldRasterState);
    IC->OMSetDepthStencilState(m_pOldDepthStencilState, m_nOldStencilRef);
    IC->OMSetBlendState(m_pBlendState, m_BlendFactor, m_pSampleMask) ;

    IC->GSSetShader(m_pOldGeometryShader, NULL, 0);
    IC->PSSetShader(m_pOldPixelShader, NULL, 0);
    IC->VSSetShader(m_pOldVertexShader, NULL, 0);

    if (m_pDevice->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_0)
    {
        IC->HSSetShader(m_pOldHullShader, NULL, 0);
        IC->DSSetShader(m_pOldDomainShader, NULL, 0);
        IC->CSSetShader(m_pOldComputeShader, NULL, 0);
    }

    IC->IASetVertexBuffers(0, DX11_MAX_VERTEXBUFFERCOUNT, m_pOldVertexBuffers, m_nOldVertexBufferStrides, m_nOldVertexBufferOffsets);
    IC->IASetIndexBuffer(m_pOldIndexBuffer, m_OldIndexBufferFormat, m_OldIndexBufferOffset);

    // Reset the samplers.
    IC->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_PSSamplers);
    IC->GSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_GSSamplers);
    IC->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_VSSamplers);
    IC->HSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_HSSamplers);
    IC->DSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_DSSamplers);
    IC->CSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_CSSamplers);
}
