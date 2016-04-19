//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12ImageRenderer.h
/// \brief  Header file for DX12ImageRenderer.
///         This class helps to render D3D12 resources into RGBA8 CPU buffers.
//=============================================================================

#ifndef __DX12_IMAGE_RENDERER_H__
#define __DX12_IMAGE_RENDERER_H__

#include <d3d12.h>
#include "../../Common/SaveImage.h"

/// Used for debugging
#define OVERWRITE_SRC_RES 0

/// Helper macro for safe release
#define SAFE_DX_RELEASE(s) if (s) { s->Release(); s = nullptr; }

/// Root param enums
enum RootParameters : UINT32
{
    RootParameterSRV,
    RootParameterUAV,
    RootParameterCBV,
    RootParametersCount
};

/// Color used for clearing
static const float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

/// Bytes per pixel definition
static const UINT BytesPerPixel  = 4;

/// Constant buffer size
static const UINT ConstBufSize   = 0x10000;

//-----------------------------------------------------------------------------
/// A structure of data that data we'll upload to a constant buffer.
//-----------------------------------------------------------------------------
struct ConstantBuffer
{
    UINT rtWidth; ///< The width of the render target.
    UINT flipX;   ///< Responsible for flipping the image horizontally.
    UINT flipY;   ///< Responsible for flipping the image vertically.
};

//-----------------------------------------------------------------------------
/// A structure containing ImageRenderer initialization info.
//-----------------------------------------------------------------------------
struct DX12ImageRendererConfig
{
    ID3D12Device*       pDevice;   ///< The device that the Image Renderer will use.
    ID3D12CommandQueue* pCmdQueue; ///< The CommandQueue that the Image Renderer will use.
};

//-----------------------------------------------------------------------------
/// A structure with instances that must be created to capture a Render Target.
//-----------------------------------------------------------------------------
struct CaptureAssets
{
    ID3D12Resource*       pInternalRT;          ///< A handle to a copy of the Render Target.
    ID3D12DescriptorHeap* pInternalRtvHeap;     ///< An internal RTV heap.
    ID3D12Resource*       pPSWriteBuf;          ///< A buffer where the image data will be written.
    ID3D12Resource*       pPSWriteBufReadBack;  ///< A buffer that we can map to read back image data.
};

//-----------------------------------------------------------------------------
/// An Image Renderer used to capture the Render Target of the instrumented application.
//-----------------------------------------------------------------------------
class DX12ImageRenderer
{
public:
    //-----------------------------------------------------------------------------
    /// Statically create a DX12ImageRenderer.
    /// \param config A structure containing all of the necessary initialization info.
    /// \returns A new DX12ImageRenderer instance.
    //-----------------------------------------------------------------------------
    static DX12ImageRenderer* Create(const DX12ImageRendererConfig& config);

    //-----------------------------------------------------------------------------
    /// Resizes the input width and height so that their aspect ratio matches the aspect ratio of
    /// the input resource. The resulting width and height is never bigger than the input values.
    /// \param pRes The resource of the Render Target.
    /// \param destWidth The width of the Render Target.
    /// \param destHeight The height of the Render Target.
    //-----------------------------------------------------------------------------
    static void CorrectSizeForAspectRatio(
        ID3D12Resource* pRes,
        UINT&           destWidth,
        UINT&           destHeight);

    //-----------------------------------------------------------------------------
    /// Destructor.
    //-----------------------------------------------------------------------------
    ~DX12ImageRenderer();

    //-----------------------------------------------------------------------------
    /// Convert a DX12 resource to a CPU-visible linear buffer of pixels.
    /// The data is filled in a user - provided CpuImage struct.
    /// IMPORTANT : Memory inside pImgOut is allocated on behalf of the caller, so it is their responsibility to free it.
    /// \param pRes The Render Target resource to capture image data for.
    /// \param prevState The previous state that has been set on the resource.
    /// \param newWidth The width of the output image data.
    /// \param newHeight The height of the output image data.
    /// \param format The image format for the output image data.
    /// \param pImgOut A pointer to the structure containing all capture image data.
    /// \param bFlipX Option used to flip the image horizontally.
    /// \param bFlipY Option used to flip the image vertically.
    /// \returns The result code of the capture operation.
    //-----------------------------------------------------------------------------
    HRESULT CaptureImage(
        ID3D12Resource*       pRes,
        D3D12_RESOURCE_STATES prevState,
        UINT                  newWidth,
        UINT                  newHeight,
        DXGI_FORMAT           format,
        CpuImage*             pImgOut,
        bool                  bFlipX,
        bool                  bFlipY);

private:
    //-----------------------------------------------------------------------------
    /// Constructor.
    //-----------------------------------------------------------------------------
    DX12ImageRenderer();

    //-----------------------------------------------------------------------------
    /// Initialize all members needed by this rendering class.
    /// \param config A structure containing all of the necessary initialization info.
    /// \returns The result code for the initialization step.
    //-----------------------------------------------------------------------------
    HRESULT Init(const DX12ImageRendererConfig& config);

    //-----------------------------------------------------------------------------
    /// Create resources that are unique to each capture.
    /// \param pRes The Render Target resource that will be captured.
    /// \param newWidth The new width of the output image data.
    /// \param newHeight The new height of the output image data.
    /// \param format The format of the output image data.
    /// \param assets A structure containing all assets necessary to capture the Render Target.
    /// \returns The result code for the capture.
    //-----------------------------------------------------------------------------
    HRESULT CreateCaptureAssets(
        ID3D12Resource* pRes,
        UINT            newWidth,
        UINT            newHeight,
        DXGI_FORMAT     format,
        CaptureAssets&  assets);

    //-----------------------------------------------------------------------------
    /// Destroy per-capture resources.
    /// \param assets A structure containing all assets necessary to capture the Render Target.
    //-----------------------------------------------------------------------------
    void FreeCaptureAssets(CaptureAssets& assets);

    //-----------------------------------------------------------------------------
    /// Wait for a command list to finish.
    //-----------------------------------------------------------------------------
    void WaitCmdListFinish();

    /// The structure containing all configuration info for this Image Renderer.
    DX12ImageRendererConfig m_config;

    /// The command allocator used to make a copy the Render Target.
    ID3D12CommandAllocator* m_pCmdAllocator;

    /// The command list that will do the work of copying the Render Target data.
    ID3D12GraphicsCommandList* m_pCmdList;

    /// The root signature for the capture renderer pipeline.
    ID3D12RootSignature* m_pRootSignatureGraphics;

    /// The pipeline state object for the capture renderer.
    ID3D12PipelineState* m_pPipelineStateGraphics;

    /// A descriptor heap for our image data buffer.
    ID3D12DescriptorHeap* m_pSrvUavCbHeap;

    /// The size of a UAV descriptor.
    UINT m_srvUavCbDescriptorSize;

    /// Fence used to indicate completed Render Target capture.
    ID3D12Fence* m_pFence;

    /// Handle to the event that indicates completed Render Target capture.
    HANDLE m_fenceEvent;

    /// The value to watch for in the next fence wait.
    UINT64 m_fenceValue;

    /// A constant buffer where capture inputs are stored.
    ID3D12Resource* m_pConstantBuffer;

    /// The constant buffer that's mapped to transfer capture options.
    UINT8* m_pCbvDataBegin;
};

#endif