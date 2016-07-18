//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktFrameDebuggerLayer.h
/// \brief  Header file for our Vulkan frame debugger layer.
//==============================================================================

#ifndef __VKT_FRAME_DEBUGGER_LAYER_H__
#define __VKT_FRAME_DEBUGGER_LAYER_H__

#include "../Rendering/vktImageRenderer.h"
#include "../../../Common/ModernAPIFrameDebuggerLayer.h"

//-----------------------------------------------------------------------------
/// Holds information about swap chain, used by frame buffer renderer.
//-----------------------------------------------------------------------------
struct SwapChainInfo
{
    VkSwapchainKHR appSwapChain;         ///< The swap chain
    VkImage*       pSwapChainImages;     ///< The images backing the swap chain
    UINT           swapChainImageCount;  ///< The number os swap chain images
    VkExtent2D     swapChainExtents;     ///< The swap chain dimensions
};

//-----------------------------------------------------------------------------
/// The Vulkan-specific Frame Profiler layer implementation.
//-----------------------------------------------------------------------------
class VktFrameDebuggerLayer : public ModernAPIFrameDebuggerLayer, public TSingleton < VktFrameDebuggerLayer >
{
    /// This class is a singleton
    friend TSingleton < VktFrameDebuggerLayer >;

public:
    VktFrameDebuggerLayer();

    virtual ~VktFrameDebuggerLayer();
    virtual ModernAPILayerManager* GetParentLayerManager();

    /// Called when creating this layer
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr) { GT_UNREFERENCED_PARAMETER(type); GT_UNREFERENCED_PARAMETER(pPtr); return true; }

    /// Called when destroying this layer
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr) { GT_UNREFERENCED_PARAMETER(type); GT_UNREFERENCED_PARAMETER(pPtr); return true; }

    /// Get derived layer settings
    virtual std::string GetDerivedSettings() { return ""; }

    virtual void EndFrame();
    virtual bool CaptureFrameBuffer(unsigned int inWidth, unsigned int inHeight, unsigned char** ppFrameBufferPngData, unsigned int* pNumBytes, bool adjustAspectRatio);

    void OnPresent(const QueueInfo& queueInfo);
    void OnSwapchainCreated(VkDevice device, VkSwapchainKHR swapChain, VkExtent2D extents);

private:

    bool HandleFrameBufferRequest(PictureCommandResponse& inImageCommand);

    /// Used to render the render target
    VktImageRenderer* m_pFrameBufferRenderer;

    /// Command response which sends frame buffer image to client
    PictureCommandResponse m_getFrameBufferImage;

    /// Track which queue was last to present and use its frame buffer
    QueueInfo m_lastPresentQueueInfo;

    /// Information for a swap chain
    SwapChainInfo m_swapChainInfo;
};

#endif // __VKT_FRAME_DEBUGGER_LAYER_H__