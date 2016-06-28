//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktFrameDebuggerLayer.cpp
/// \brief  The Vulkan-specific Frame Debugger layer implementation.
//==============================================================================

#include "vktFrameDebuggerLayer.h"
#include "../vktLayerManager.h"
#include "../Tracing/vktTraceAnalyzerLayer.h"
#include "../Objects/vktObjectDatabaseProcessor.h"
#include "../../../Common/ErrorImage.h"
#include "../../../Common/SaveImage.h"
#include "../../../Common/FrameInfo.h"
#include "../../../Common/TraceMetadata.h"

//-----------------------------------------------------------------------------
/// Default constructor for VktFrameDebuggerLayer.
//-----------------------------------------------------------------------------
VktFrameDebuggerLayer::VktFrameDebuggerLayer() :
    ModernAPIFrameDebuggerLayer(),
    m_pFrameBufferRenderer(nullptr)
{
    AddCommand(CONTENT_PNG, "GetFrameBufferImage", "GetFrameBufferImage", "GetFrameBufferImage.png", NO_DISPLAY, NO_INCLUDE, m_getFrameBufferImage);
    memset(&m_swapChainInfo, 0, sizeof(m_swapChainInfo));
    memset(&m_lastPresentQueueInfo, 0, sizeof(m_lastPresentQueueInfo));
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
VktFrameDebuggerLayer::~VktFrameDebuggerLayer()
{
    SAFE_DELETE(m_pFrameBufferRenderer);
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* VktFrameDebuggerLayer::GetParentLayerManager()
{
    return VktLayerManager::GetLayerManager();
}

//-----------------------------------------------------------------------------
/// Handle frame buffer screenshot requests at end of frame.
//-----------------------------------------------------------------------------
void VktFrameDebuggerLayer::EndFrame()
{
    if (m_getFrameBufferImage.IsActive())
    {
        HandleFrameBufferRequest(m_getFrameBufferImage);
    }

    ModernAPIFrameDebuggerLayer::EndFrame();
}

//-----------------------------------------------------------------------------
/// Save off info about the last queue which was used to present.
//-----------------------------------------------------------------------------
void VktFrameDebuggerLayer::OnPresent(const QueueInfo& queueInfo)
{
    memcpy(&m_lastPresentQueueInfo, &queueInfo, sizeof(queueInfo));
}

//-----------------------------------------------------------------------------
/// Gets called after a frame has been presented.
/// \param device The device used to create this swap chain.
/// \param swapChain The newly created swap chain.
/// \param extents The swap chain dimensions.
//-----------------------------------------------------------------------------
void VktFrameDebuggerLayer::OnSwapchainCreated(VkDevice device, VkSwapchainKHR swapChain, VkExtent2D extents)
{
    VkResult result = VK_INCOMPLETE;

    m_swapChainInfo.appSwapChain = swapChain;
    m_swapChainInfo.swapChainExtents = extents;

    result = device_dispatch_table(device)->GetSwapchainImagesKHR(device, swapChain, &m_swapChainInfo.swapChainImageCount, nullptr);

    if (result == VK_SUCCESS)
    {
        m_swapChainInfo.pSwapChainImages = (VkImage*)malloc(m_swapChainInfo.swapChainImageCount * sizeof(VkImage));

        result = device_dispatch_table(device)->GetSwapchainImagesKHR(device, swapChain, &m_swapChainInfo.swapChainImageCount, m_swapChainInfo.pSwapChainImages);
    }
}

//-----------------------------------------------------------------------------
/// Capture the current frame buffer image, and return an byte array of PNG-encoded image data. NOTE: Passing in both a width and height of 0 will causes the frame buffer's size to be used when generating the output image.
/// Note that the output "ioFrameBufferPngData" array must be deleted when finished, or else it will leak.
/// \param inWidth The requested width of the captured frame buffer image.
/// \param inHeight The requested height of the captured frame buffer image.
/// \param ppFrameBufferPngData A pointer to the byte array of PNG-encoded image data.
/// \param pNumBytes The total number of bytes in the array of encoded image data.
/// \param adjustAspectRatio Turns on or off the resizing of the input height and width for cases where the aspect ratios do not match.
/// \returns True if the frame buffer image was captured successfully. False if it failed.
//-----------------------------------------------------------------------------
bool VktFrameDebuggerLayer::CaptureFrameBuffer(unsigned int inWidth, unsigned int inHeight, unsigned char** ppFrameBufferPngData, unsigned int* pNumBytes, bool adjustAspectRatio)
{
    if (m_pFrameBufferRenderer == NULL)
    {
        VktImageRendererConfig rendererConfig = VktImageRendererConfig();
        rendererConfig.physicalDevice = m_lastPresentQueueInfo.physicalDevice;
        rendererConfig.device         = m_lastPresentQueueInfo.device;
        rendererConfig.queue          = m_lastPresentQueueInfo.queue;
        m_pFrameBufferRenderer = VktImageRenderer::Create(rendererConfig);
    }

    // Correct the size of the output image to match the aspect ratio of the input resource
    if (adjustAspectRatio == true)
    {
        m_pFrameBufferRenderer->CorrectSizeForApsectRatio(m_swapChainInfo.swapChainExtents.width, m_swapChainInfo.swapChainExtents.height, inWidth, inHeight);
    }

    // If the requested size is 0 x 0 pixels then use the actual source resource size.
    if (inWidth == 0 && inHeight == 0)
    {
        inWidth = m_swapChainInfo.swapChainExtents.width;
        inHeight = m_swapChainInfo.swapChainExtents.height;
    }

    CpuImage capturedImage = CpuImage();
    VkResult captureResult = m_pFrameBufferRenderer->CaptureImage(
                                 m_swapChainInfo.pSwapChainImages[0],
                                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                 m_swapChainInfo.swapChainExtents.width,
                                 m_swapChainInfo.swapChainExtents.height,
                                 inWidth,
                                 inHeight,
                                 &capturedImage,
                                 false,
                                 false);

    bool pngSuccess = false;

    if (captureResult == S_OK)
    {
        // Convert the captured image's pixel data into a PNG byte array.
        pngSuccess = RGBAtoPNG(static_cast<unsigned char*>(capturedImage.pData), inWidth, inHeight, pNumBytes, ppFrameBufferPngData);

        // free memory allocated by VktFrameDebuggerLayer::CaptureImage
        free(capturedImage.pData);
    }
    else
    {
        Log(logERROR, "Failed to capture frame buffer image.\n");
    }

    return pngSuccess;
}

//-----------------------------------------------------------------------------
/// Handle an incoming image request by sending the image data as a response.
/// \param inImageCommand The command used to request image data.
/// \returns True when the frame buffer was successfully captured and sent back as a response.
//-----------------------------------------------------------------------------
bool VktFrameDebuggerLayer::HandleFrameBufferRequest(PictureCommandResponse& inImageCommand)
{
    // @TODO - pull it up
    bool bCaptureSuccessful = false;

    ModernAPILayerManager* layerManager = VktLayerManager::GetLayerManager();

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
            ErrorImage::Instance()->Send(&m_getFrameBufferImage);

            Log(logERROR, "Failed to capture frame buffer. Replying with error image.\n");
        }
    }

    return bCaptureSuccessful;
}
