//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktImageRenderer.h
/// \brief  Header file for VktImageRenderer.
///         This class helps to render Vulkan images into RGBA8 CPU buffers.
//==============================================================================

#ifndef __VKT_IMAGE_RENDERER_H__
#define __VKT_IMAGE_RENDERER_H__

#include "../Util/vktUtil.h"
#include "../../../Common/SaveImage.h"

#pragma warning (push)
#pragma warning (disable : 4100)
#pragma warning (disable : 4458)
#include <GlslangToSpv.h>
#pragma warning (pop)

/// Bytes per pixel in render target
static const UINT BytesPerPixel = 4;

//-----------------------------------------------------------------------------
/// Define possible measurement types.
//-----------------------------------------------------------------------------
struct UniformBuffer
{
    UINT rtWidth;  ///< Tell the shader the width of the RT
    UINT flipX;    ///< Flip along X axis
    UINT flipY;    ///< Flip along Y axis
};

//-----------------------------------------------------------------------------
/// Define possible measurement types.
//-----------------------------------------------------------------------------
struct VktImageRendererConfig
{
    VkPhysicalDevice physicalDevice; ///< The renderer's physical device
    VkDevice         device;         ///< The renderer's device
    VkQueue          queue;          ///< The queue used by the renderer
};

//-----------------------------------------------------------------------------
/// Define possible measurement types.
//-----------------------------------------------------------------------------
struct CaptureAssets
{
    VkImage        internalRT;     ///< Image backing our RT
    VkDeviceMemory internalRTMem;  ///< Memory backing our RT
    VkImageView    internalRTView; ///< View of our RT
    VkFramebuffer  frameBuffer;    ///< Our RT
    VkImageView    srcImageView;   ///< View pointing to the app's RT
    VkBuffer       uniformBuf;     ///< Uniform buffer used to pass data to our shader
    VkDeviceMemory uniformBufMem;  ///< Memory backing our uniform buffer
    VkBuffer       storageBuf;     ///< Storage buffer
    VkDeviceMemory storageBufMem;  ///< Memory backing our storage buffer
};

//-----------------------------------------------------------------------------
/// This class is used to capture a screen shot of an app's frame buffer.
//-----------------------------------------------------------------------------
class VktImageRenderer
{
public:
    static VktImageRenderer* Create(const VktImageRendererConfig& config);

    static void CorrectSizeForApsectRatio(
        UINT  srcWidth,
        UINT  srcHeight,
        UINT& dstWidth,
        UINT& dstHeight);

    ~VktImageRenderer();

    VkResult CaptureImage(
        VkImage       res,
        VkImageLayout prevState,
        UINT          oldWidth,
        UINT          oldHeight,
        UINT          newWidth,
        UINT          newHeight,
        CpuImage*     pImgOut,
        bool          flipX,
        bool          flipY);
private:
    VktImageRenderer();

    VkResult Init(const VktImageRendererConfig& config);

    VkResult InitShaders(
        VkDevice                         device,
        VkPipelineShaderStageCreateInfo* pShaderStages,
        const char*                      pVertShaderText,
        const char*                      pFragShaderText);

    void ChangeImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout prevLayout, VkImageLayout newLayout);

    VkResult MemTypeFromProps(UINT typeBits, VkFlags reqsMask, UINT* pTypeIdx);

    VkResult CreateCaptureAssets(VkImage srcImage, UINT newWidth, UINT newHeight, bool flipX, bool flipY, CaptureAssets& assets);

    void FreeCaptureAssets(CaptureAssets& assets);

    VkResult AllocBindImageMem(VkImage* pImage, VkDeviceMemory* pMem, VkDeviceSize* pMemSize);
    VkResult AllocBindBufferMem(VkDescriptorBufferInfo& bufferInfo, VkBuffer* pBuf, VkDeviceMemory* pMem, VkDeviceSize* pMemSize);

    void InitResources(TBuiltInResource& resources);
    EShLanguage FindLanguage(const VkShaderStageFlagBits shaderType);
    VkResult GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char* pShaderStr, std::vector<UINT>& spirv);

    /// Configuration for this renderer
    VktImageRendererConfig m_config;

    /// Physical device properties
    VkPhysicalDeviceMemoryProperties m_memProps;

    /// Instance dispatch table
    VkLayerInstanceDispatchTable* m_pInstanceDT;

    /// Device dispatch table
    VkLayerDispatchTable* m_pDeviceDT;

    /// This renderer's command pool
    VkCommandPool m_cmdPool;

    /// This renderer's command buffer
    VkCommandBuffer m_cmdBuf;

    /// This renderer's render pass
    VkRenderPass m_renderPass;

    /// This renderer's descriptor set pool
    VkDescriptorPool m_descPool;

    /// This renderer's descriptor set layout
    VkDescriptorSetLayout m_descSetLayout;

    /// This renderer's descriptor set
    VkDescriptorSet m_descSet;

    /// The PSO layout used by the renderer
    VkPipelineLayout m_pipelineLayout;

    /// The PSO cache used by the renderer
    VkPipelineCache m_pipelineCache;

    /// The PSO used by the renderer
    VkPipeline m_pipeline;

    /// Texture sampler for the src image
    VkSampler m_sampler;
};

#endif // __VKT_IMAGE_RENDERER_H__