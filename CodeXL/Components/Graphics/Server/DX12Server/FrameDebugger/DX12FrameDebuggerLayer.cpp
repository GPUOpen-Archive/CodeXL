//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12FrameDebuggerLayer.cpp
/// \brief  The DX12FrameDebuggerLayer is responsible for operations related to
///         inspecting individual frames of an instrumented D3D12 application.
//=============================================================================

#include "DX12FrameDebuggerLayer.h"
#include "../Tracing/DX12TraceAnalyzerLayer.h"
#include "../Rendering/DX12ImageRenderer.h"
#include "../Common/SaveImage.h"
#include "../Common/ErrorImage.h"
#include "../Common/FrameInfo.h"
#include "../Common/TraceMetadata.h"
#include "../DX12LayerManager.h"

//-----------------------------------------------------------------------------
/// Default constructor for DX12FrameDebuggerLayer.
//-----------------------------------------------------------------------------
DX12FrameDebuggerLayer::DX12FrameDebuggerLayer()
    : mLastPresentedSwapchain(nullptr)
    , mFrameBufferRenderer(nullptr)
{
    AddCommand(CONTENT_PNG, "GetFrameBufferImage", "GetFrameBufferImage", "GetFrameBufferImage.png", NO_DISPLAY, NO_INCLUDE, mGetFrameBufferImage);
}

//-----------------------------------------------------------------------------
/// Default destructor for DX12FrameDebuggerLayer.
//-----------------------------------------------------------------------------
DX12FrameDebuggerLayer::~DX12FrameDebuggerLayer()
{
    // Destroy the image renderer.
    SAFE_DELETE(mFrameBufferRenderer);
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* DX12FrameDebuggerLayer::GetParentLayerManager()
{
    return DX12LayerManager::Instance();
}

//-----------------------------------------------------------------------------
/// Called at the end of a frame
//-----------------------------------------------------------------------------
void DX12FrameDebuggerLayer::EndFrame()
{
    if (mGetFrameBufferImage.IsActive())
    {
        HandleFrameBufferRequest(mGetFrameBufferImage);
    }

    ModernAPIFrameDebuggerLayer::EndFrame();
}

//-----------------------------------------------------------------------------
/// Gets called after a frame has been presented.
/// \param inSwapchain The swap chain instance used to present to the screen.
//-----------------------------------------------------------------------------
void DX12FrameDebuggerLayer::OnPresent(void* inSwapchain)
{
    mLastPresentedSwapchain = static_cast<IDXGISwapChain*>(inSwapchain);
}

//-----------------------------------------------------------------------------
/// Handler invoked when a new swap chain is created.
/// \param inSwapchain The swap chain instance that was just created.
/// \param inDevice The CommandQueue used to create the swap chain.
//-----------------------------------------------------------------------------
void DX12FrameDebuggerLayer::OnSwapchainCreated(void* inSwapchain, void* inDevice)
{
    IDXGISwapChain* newSwapchain = static_cast<IDXGISwapChain*>(inSwapchain);

    if (mSwapchainToCommandQueue.find(newSwapchain) == mSwapchainToCommandQueue.end())
    {
        // Associate the swap chain with a command queue.
        mSwapchainToCommandQueue[newSwapchain] = static_cast<ID3D12CommandQueue*>(inDevice);
    }
}

//-----------------------------------------------------------------------------
/// Capture the current frame buffer image, and return an byte array of PNG-encoded image data. NOTE: Passing in both a width and height of 0 will causes the frame buffer's size to be used when generating the output image.
/// Note that the output "ioFrameBufferPngData" array must be deleted when finished, or else it will leak.
/// \param inWidth The requested width of the captured frame buffer image.
/// \param inHeight The requested height of the captured frame buffer image.
/// \param ioFrameBufferPngData A pointer to the byte array of PNG-encoded image data.
/// \param outNumBytes The total number of bytes in the array of encoded image data.
/// \param adjustAspectRatio Turns on or off the resizing of the input height and width for cases where the aspect ratios do not match.
/// \returns True if the frame buffer image was captured successfully. False if it failed.
//-----------------------------------------------------------------------------
bool DX12FrameDebuggerLayer::CaptureFrameBuffer(unsigned int inWidth, unsigned int inHeight, unsigned char** ioFrameBufferPngData, unsigned int* outNumBytes, bool adjustAspectRatio)
{
    bool bCaptureSuccessful = false;

    if (mLastPresentedSwapchain != nullptr)
    {
        ID3D12Resource* frameBufferResource = nullptr;
        HRESULT gotFrameBuffer = mLastPresentedSwapchain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&frameBufferResource);

        if (gotFrameBuffer == S_OK)
        {
            // Attempt to find the CommandQueue used to create the active swap chain.
            SwapchainToQueueMap::iterator swapchainQueueIter = mSwapchainToCommandQueue.find(mLastPresentedSwapchain);

            if (swapchainQueueIter != mSwapchainToCommandQueue.end())
            {
                ID3D12CommandQueue* commandQueue = swapchainQueueIter->second;

                ID3D12Device* parentDevice = nullptr;
                HRESULT gotDevice = commandQueue->GetDevice(__uuidof(ID3D12Device), (void**)&parentDevice);

                if (gotDevice == S_OK)
                {
                    if (mFrameBufferRenderer == nullptr)
                    {
                        DX12ImageRendererConfig rendererConfig;
                        rendererConfig.pCmdQueue = commandQueue;
                        rendererConfig.pDevice = parentDevice;
                        mFrameBufferRenderer = DX12ImageRenderer::Create(rendererConfig);
                    }

                    if (mFrameBufferRenderer != nullptr)
                    {
                        D3D12_RESOURCE_DESC fbDesc = frameBufferResource->GetDesc();

                        // get the frame buffer format
                        DXGI_FORMAT format = fbDesc.Format;

                        // If the requested size is 0 x 0 pixels then use the actual source resource size.
                        if (inWidth == 0 && inHeight == 0)
                        {
                            inWidth = (unsigned int)fbDesc.Width;
                            inHeight = fbDesc.Height;
                        }

                        // Correct the size of the output image to match the aspect ratio of the input resource
                        if (adjustAspectRatio == true)
                        {
                            mFrameBufferRenderer->CorrectSizeForAspectRatio(frameBufferResource, inWidth, inHeight);
                        }

                        CpuImage capturedImage;
                        HRESULT captureResult = mFrameBufferRenderer->CaptureImage(frameBufferResource, D3D12_RESOURCE_STATE_PRESENT, inWidth, inHeight, format, &capturedImage, false, true);

                        if (captureResult == S_OK)
                        {
                            // Convert the captured image's pixel data into a PNG byte array.
                            bCaptureSuccessful = RGBAtoPNG(static_cast<unsigned char*>(capturedImage.pData), inWidth, inHeight, outNumBytes, ioFrameBufferPngData);

                            // free memory allocated by DX12ImageRenderer::CaptureImage
                            free(capturedImage.pData);
                        }
                        else
                        {
                            Log(logERROR, "Failed to capture frame buffer image.\n");
                        }
                    }
                    else
                    {
                        Log(logERROR, "Failed to create a DX12ImageRenderer for frame buffer capture.\n");
                    }

                    SAFE_DX_RELEASE(parentDevice);
                }
                else
                {
                    Log(logERROR, "Failed to retrieve parent device from swap chain buffer.\n");
                }

                SAFE_DX_RELEASE(frameBufferResource);
            }
            else
            {
                Log(logERROR, "Failed to retrieve CommandQueue used to create presentation Swapchain.\n");
            }
        }
        else
        {
            Log(logERROR, "Failed to retrieve frame buffer from swap chain.\n");
        }
    }
    else
    {
        Log(logERROR, "Failed to capture frame buffer: No active Swapchain set in Frame Debugger.\n");
    }

    return bCaptureSuccessful;
}

//-----------------------------------------------------------------------------
/// Handle an incoming image request by sending the image data as a response.
/// \param inImageCommand The command used to request image data.
/// \returns True when the frame buffer was successfully captured and sent back as a response.
//-----------------------------------------------------------------------------
bool DX12FrameDebuggerLayer::HandleFrameBufferRequest(PictureCommandResponse& inImageCommand)
{
    bool bCaptureSuccessful = false;

    ModernAPILayerManager* layerManager = GetDX12LayerManager();

    if (layerManager->InCapturePlayer())
    {
        // If we're operating within the CapturePlayer, we'll want to retrieve the cached image instead of capturing a new one.
        const std::string& metadataFilepath = layerManager->GetPathToTargetMetadataFile();

        TraceMetadata traceMetadata = {};
        traceMetadata.mFrameInfo = new FrameInfo();
        ReadMetadataFile(metadataFilepath, &traceMetadata);

        const std::string& frameBufferImagePath = traceMetadata.mPathToFrameBufferImage;

        // Read all bytes in the cached image file and return through the response.
        FILE* frameBufferImageFile = fopen(frameBufferImagePath.c_str(), "rb");

        if (frameBufferImageFile != nullptr)
        {
            // Figure out how many bytes are in the image file.
            fseek(frameBufferImageFile, 0L, SEEK_END);
            unsigned int numBytes = ftell(frameBufferImageFile);

            // Rewind to the beginning.
            fseek(frameBufferImageFile, 0L, SEEK_SET);

            // Read the image out of the file and into a temporary buffer.
            unsigned char* imageData = new unsigned char[numBytes];
            fread(imageData, sizeof(unsigned char), numBytes, frameBufferImageFile);

            // We're done with the image file- close the handle.
            fclose(frameBufferImageFile);

            // Send the image data back through the command.
            inImageCommand.Send(reinterpret_cast<const char*>(imageData), numBytes);

            bCaptureSuccessful = true;

            // Destroy the image data that was just sent.
            SAFE_DELETE_ARRAY(imageData);
        }
        else
        {
            Log(logERROR, "Failed to open cached frame buffer image file at '%s'.\n", frameBufferImagePath.c_str());
        }

        // Destroy the FrameInfo structure.
        SAFE_DELETE(traceMetadata.mFrameInfo);
    }
    else
    {
        unsigned int imageWidth = inImageCommand.GetWidth();
        unsigned int imageHeight = inImageCommand.GetHeight();
        unsigned char* frameBufferImageData = nullptr;
        unsigned int numImageBytes = 0;

        bool bCapturedBuffer = CaptureFrameBuffer(imageWidth, imageHeight, &frameBufferImageData, &numImageBytes, true);

        if (bCapturedBuffer)
        {
            // Send the image back as a chunk of response data.
            inImageCommand.Send(reinterpret_cast<const char*>(frameBufferImageData), numImageBytes);

            bCaptureSuccessful = true;

            // Free the image data after we're done using it, and destroy the image renderer.
            // use free rather than delete [] since the memory was allocated with malloc()
            if (frameBufferImageData != nullptr)
            {
                free(frameBufferImageData);
            }
        }
        else
        {
            // Send the error image if capturing failed.
            ErrorImage::Instance()->Send(&mGetFrameBufferImage);

            Log(logERROR, "Failed to capture frame buffer. Replying with error image.\n");
        }
    }

    return bCaptureSuccessful;
}
