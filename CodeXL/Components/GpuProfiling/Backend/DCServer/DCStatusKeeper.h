//=====================================================================
//
// Author: Konstantin L Tolskiy
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DX11StatusKeeper.h
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/Backend/DCServer/DCStatusKeeper.h#2 $
//
// Last checkin:  $DateTime: 2013/11/07 16:00:44 $
// Last edited by: $Author: chesik $
//=====================================================================
//   ( C ) AMD, Inc.  All rights reserved.
//=====================================================================
//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File contains the header of the DX11StatusKeeper class
//         Adapted from GPS2
//==============================================================================

#ifndef _DC_STATUS_KEEPER_H_
#define _DC_STATUS_KEEPER_H_

#include <windows.h>
#include <d3d11.h>
#include "..\Common\Defs.h"

/// \addtogroup DCCommandRecorder
// @{

#define DX11_MAX_TEXTURES           D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT
#define DX11_MAX_RENDERTARGETS      D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
#define DX11_MAX_VERTEXBUFFERCOUNT  D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT

/// This class is used for keeping device settings.
class DX11Status
{
public:

    /// The device whose status we are keeping track of
    ID3D11Device*            m_pDevice;

    /// The Capture() function will fill this array with the RT's currently bound to the pipeline
    ID3D11RenderTargetView*  m_pOldRenderTargetViews[ DX11_MAX_RENDERTARGETS ];

    /// The Capture() function will fill this array with the SRV's currently bound to the VS
    ID3D11ShaderResourceView*  m_pVSResourceViews[DX11_MAX_TEXTURES];

    /// The Capture() function will fill this array with the SRV bound to the GS
    ID3D11ShaderResourceView*  m_pGSResourceViews[DX11_MAX_TEXTURES];

    /// The Capture() function will fill this array with the PS SRV's bound to the PS
    ID3D11ShaderResourceView*  m_pPSResourceViews[DX11_MAX_TEXTURES];

    /// The Capture() function will fill this array with the HS SRV's bound to the HS
    ID3D11ShaderResourceView*  m_pHSResourceViews[DX11_MAX_TEXTURES];

    /// The Capture() function will fill this array with the DS SRV's bound to the DS
    ID3D11ShaderResourceView*  m_pDSResourceViews[DX11_MAX_TEXTURES];

    /// The Capture() function will fill this array with the CS SRV's bound to the CS
    ID3D11ShaderResourceView*  m_pCSResourceViews[DX11_MAX_TEXTURES];

    /// The Capture() function will fill this array with the CS UAV's bound to the CS
    ID3D11UnorderedAccessView* m_pCSUnorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT];

    /// The Capture() function will fill this array with the OM UAV's bound to the COM
    ID3D11UnorderedAccessView* m_pOMUnorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT];

    /// Buffer the apps sampling states.
    ID3D11SamplerState* m_PSSamplers [D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* m_GSSamplers [D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* m_VSSamplers [D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* m_HSSamplers [D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* m_DSSamplers [D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* m_CSSamplers [D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

    /// The Capture() function will fill this array with the SO's buffers bound to the pipeline
    ID3D11Buffer* m_pSOBuffers[D3D11_SO_BUFFER_SLOT_COUNT];

    /// The Capture() function will fill this array with the offsets to be used with the vertex buffer bound to the pipeline
    UINT m_SOOffsets[D3D11_SO_BUFFER_SLOT_COUNT];

    /// The Capture() function will store here the DSV bound to the pipeline
    ID3D11DepthStencilView*  m_pOldDepthStencilView;

    /// The Capture() function will store here the Inputlayout bound to the pipeline
    ID3D11InputLayout*       m_pOldInputLayout;

    /// The Capture() function will store here the pointer to the currently set resterizer state
    ID3D11RasterizerState*   m_pOldRasterState;

    /// Store the current blend state pointer
    ID3D11BlendState* m_pBlendState ;
    UINT  m_pSampleMask ;
    FLOAT m_BlendFactor[4];

    /// The Capture() function will store here the pointer to the currently set topology
    D3D11_PRIMITIVE_TOPOLOGY m_oldTopology;

    /// The Capture() function will store here the currently set viewport
    D3D11_VIEWPORT    m_oldViewPorts[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    /// The number of viewpoints
    UINT m_nOldVpNum ;

    /// Stores the current scissor rects
    D3D11_RECT     m_pOldScissorRects [D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    /// The number of stored scissor rects
    UINT m_nOldNumScissorRects ;

    /// The Capture() function will store here a pointer of the currently set depth stencil state
    ID3D11DepthStencilState* m_pOldDepthStencilState;

    /// The Capture() function will store here the currently set depth stencil ref value
    UINT                     m_nOldStencilRef;

    /// The Capture() function will store here the pointer of the currently PS bound to the pipeline
    ID3D11PixelShader*       m_pOldPixelShader;

    /// The Capture() function will store here the pointer of the currently GS bound to the pipeline
    ID3D11GeometryShader*    m_pOldGeometryShader;

    /// The Capture() function will store here the pointer of the currently VS bound to the pipeline
    ID3D11VertexShader*      m_pOldVertexShader;

    /// The Capture() function will store here the pointer of the currently HS bound to the pipeline
    ID3D11HullShader*        m_pOldHullShader;

    /// The Capture() function will store here the pointer of the currently DS bound to the pipeline
    ID3D11DomainShader*      m_pOldDomainShader;

    /// The Capture() function will store here the pointer of the currently CS bound to the pipeline
    ID3D11ComputeShader*     m_pOldComputeShader;

    /// The Capture() function will fill this array with the vertex buffers bound to the pipeline
    ID3D11Buffer*            m_pOldVertexBuffers[ DX11_MAX_VERTEXBUFFERCOUNT ];

    /// The Capture() function will store here the pointer of the strides of the VB's bound to the pipeline
    UINT                     m_nOldVertexBufferStrides[ DX11_MAX_VERTEXBUFFERCOUNT ];

    /// The Capture() function will store here the pointer of the offsets of the VB's bound to the pipeline
    UINT                     m_nOldVertexBufferOffsets[ DX11_MAX_VERTEXBUFFERCOUNT ];

    /// The Capture() function will store here the pointer of the IB's bound to the pipeline
    ID3D11Buffer*            m_pOldIndexBuffer;

    /// The Capture() function will store here the format of the IB bound to the pipeline
    DXGI_FORMAT              m_OldIndexBufferFormat;

    /// The Capture() function will store here the offset of the IB bound to the pipeline
    UINT                     m_OldIndexBufferOffset;

    /// The Capture() function will store here the pointer to the currently predicate bound to the pipeline
    ID3D11Predicate*         m_pOldPredicate;

    /// The Capture() function will store here the value of the currently predicate bound to the pipeline
    BOOL                     m_bOldPredicateValue;

    /// Records if a status has been captured
    bool                     m_bCaptured ;

public:

    /// Constructor
    DX11Status();

    /// Destructor
    virtual ~DX11Status();

    /// Captures the state of the GPU.
    /// \param pDevice The device to capture from.
    void Capture(ID3D11Device* pDevice);

    /// Restores the state of the GPU, and release the interfaces.
    void RestoreAndRelease();

    /// Benign restore - restores the captured state without releasing anything.
    void Restore();

    /// Get the original render target views befor PS2 stepped all over them.
    /// \return a pointer to the array of old render target views.
    ID3D11RenderTargetView**  GetRenderTargets()
    {
        return m_pOldRenderTargetViews;
    }

    /// Get the original depth stencil view befor PS2 stepped all over them.
    /// \return Pointer to the original depth detncil view.
    ID3D11DepthStencilView* GetDepthStencilView()
    {
        return m_pOldDepthStencilView;
    }

    /// Get the pixel shader resource views.
    /// \return A pointer to the array of shader resource views.
    ID3D11ShaderResourceView** GetPSResourceViews()
    {
        return m_pPSResourceViews;
    }

    /// Get the vertex shader resource views.
    /// \return A pointer to the array of shader resource views.
    ID3D11ShaderResourceView** GetVSResourceViews()
    {
        return m_pVSResourceViews;
    }

    /// Gets the geometry shader resource views.
    /// \return A pointer to the array of shader resource views.
    ID3D11ShaderResourceView** GetGSResourceViews()
    {
        return m_pGSResourceViews;
    }

    /// Get the hull shader resource views.
    /// \return A pointer to the array of shader resource views.
    ID3D11ShaderResourceView** GetHSResourceViews()
    {
        return m_pHSResourceViews;
    }

    /// Get the domain shader resource views.
    /// \return A pointer to the array of shader resource views.
    ID3D11ShaderResourceView** GetDSResourceViews()
    {
        return m_pDSResourceViews;
    }

    /// Get the compute shader resource views.
    /// \return A pointer to the array of shader resource views.
    ID3D11ShaderResourceView** GetCSResourceViews()
    {
        return m_pCSResourceViews;
    }

    /// Gets the compute unordered access views.
    /// \return A pointer to the array of unordered access views.
    ID3D11UnorderedAccessView** GetCSUnorderedAccessViews()
    {
        return m_pCSUnorderedAccessViews;
    }

    /// Get the compute unordered access views.
    /// \return A pointer to the array of unordered access views.
    ID3D11UnorderedAccessView** GetOMUnorderedAccessViews()
    {
        return m_pOMUnorderedAccessViews;
    }

    /// Get the original index buffer
    /// \param ppOldIndexBuffer Output pointer to array of index buffer pointers.
    /// \param eOldFormat Output reference to the format.
    /// \param nOldOffset Output reference to the original offset.
    void GetIndexBuffer(ID3D11Buffer** ppOldIndexBuffer, DXGI_FORMAT* eOldFormat, UINT* nOldOffset)
    {
        *ppOldIndexBuffer = m_pOldIndexBuffer;
        *eOldFormat = m_OldIndexBufferFormat;
        *nOldOffset = m_OldIndexBufferOffset;
    }

    /// Get a specific vertex buffer.
    /// \param nIndex The index of the vertex buffer to get.
    /// \param ppOutVertexBuffer The output vertex buffer address.
    /// \param pOutStride The output stride.
    /// \param pOutOffest The Output offset.
    void GetVertexBuffer(unsigned int nIndex, ID3D11Buffer** ppOutVertexBuffer, UINT* pOutStride, UINT* pOutOffest)
    {
        *ppOutVertexBuffer = m_pOldVertexBuffers[nIndex];
        *pOutStride = m_nOldVertexBufferStrides[nIndex];
        *pOutOffest = m_nOldVertexBufferOffsets[nIndex];
    }

    /// Get the input layout.
    /// \param ppInputLayout This is an output variable that is an input layout.
    void GetInputLayout(ID3D11InputLayout** ppInputLayout)
    {
        *ppInputLayout = m_pOldInputLayout;
    }

    /// Get the view port.
    /// \param nNumVPs Out - the number of bound viewports
    /// \param pViewPorts Out - the array of bound viewports
    void GetViewPorts(UINT* nNumVPs, D3D11_VIEWPORT** pViewPorts)
    {
        *nNumVPs = m_nOldVpNum;
        *pViewPorts = m_oldViewPorts;
    }

    /// Get the scissor rects.
    /// \param pnScissorRectCount Out - the number of scissor rects
    /// \param ppRects Out - the array of scissor rects
    void GetScissorRects(UINT* pnScissorRectCount, D3D11_RECT** ppRects)
    {
        *pnScissorRectCount = m_nOldNumScissorRects;
        *ppRects = m_pOldScissorRects;
    }

    /// Gets the topology.
    /// \return the topology
    D3D11_PRIMITIVE_TOPOLOGY GetTopology()
    {
        return m_oldTopology;
    }

    /// Gets the rasterizer state
    /// \return The Rasterizer state
    ID3D11RasterizerState* GetRasterizerState()
    {
        return m_pOldRasterState;
    }

    /// Gets the depth stencil state
    /// \param ppDepthStencilState Out - the depth stencil state
    /// \param pStencilRef Out - the stencil index
    void GetDepthStencilState(ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef)
    {
        *ppDepthStencilState = m_pOldDepthStencilState;
        *pStencilRef = m_nOldStencilRef;
    }

    /// Gets the blend state
    /// \param pBlendState Out - the blend state
    /// \param BlendFactor Out - the blend factors
    /// \param nSampleMask Out - the sample mask
    void GetBlendState(ID3D11BlendState** pBlendState, FLOAT BlendFactor[], UINT* nSampleMask)
    {
        *pBlendState = m_pBlendState;
        *nSampleMask = m_pSampleMask;
        BlendFactor[0] = m_BlendFactor[0];
        BlendFactor[1] = m_BlendFactor[1];
        BlendFactor[2] = m_BlendFactor[2];
        BlendFactor[3] = m_BlendFactor[3];
    }

private:
    /// Disable copy constructor
    /// \param obj  the input object
    DX11Status(const DX11Status& obj);

    /// Disable assignment operator
    /// \param obj  the input object
    /// \return a reference to the object
    DX11Status& operator= (const DX11Status& obj);
};

/// This class is used for keeping device setings. Constructor saves them, destructor sets them back.
class DX11StatusKeeper : private DX11Status
{
public:
    /// Constructor
    DX11StatusKeeper(ID3D11Device* pDevice)
    {
        Capture(pDevice);
    }

    /// Destructor
    ~DX11StatusKeeper()
    {
        RestoreAndRelease();
    }

private:
    /// Disable copy constructor
    /// \param obj  the input object
    DX11StatusKeeper(const DX11StatusKeeper& obj);

    /// Disable assignment operator
    /// \param obj  the input object
    /// \return a reference to the object
    DX11StatusKeeper& operator= (const DX11StatusKeeper& obj);
};

// @}

#endif // _DC_STATUS_KEEPER_H_
