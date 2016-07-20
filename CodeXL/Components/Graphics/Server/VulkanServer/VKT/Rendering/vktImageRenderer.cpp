//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktImageRenderer.cpp
/// \brief  Implementation file for VktImageRenderer.
///         This class helps to render Vulkan images into RGBA8 CPU buffers.
//==============================================================================

#include "vktImageRenderer.h"

//-----------------------------------------------------------------------------
/// Statically create a VktImageRenderer object.
/// \param config The renderer configuration.
/// \return A VktImageRenderer pointer if successful.
//-----------------------------------------------------------------------------
VktImageRenderer* VktImageRenderer::Create(const VktImageRendererConfig& config)
{
    VktImageRenderer* pOut = new VktImageRenderer();

    if (pOut != nullptr)
    {
        if (pOut->Init(config) != VK_SUCCESS)
        {
            delete pOut;
            pOut = nullptr;
        }
    }

    return pOut;
}

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
VktImageRenderer::VktImageRenderer() :
    m_pInstanceDT(nullptr),
    m_pDeviceDT(nullptr),
    m_cmdPool(VK_NULL_HANDLE),
    m_cmdBuf(VK_NULL_HANDLE),
    m_renderPass(VK_NULL_HANDLE),
    m_descPool(VK_NULL_HANDLE),
    m_descSetLayout(VK_NULL_HANDLE),
    m_descSet(VK_NULL_HANDLE),
    m_pipelineLayout(VK_NULL_HANDLE),
    m_pipelineCache(VK_NULL_HANDLE),
    m_pipeline(VK_NULL_HANDLE),
    m_sampler(VK_NULL_HANDLE)
{
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
VktImageRenderer::~VktImageRenderer()
{
    m_pDeviceDT->DestroySampler(m_config.device, m_sampler, nullptr);
    m_sampler = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyPipeline(m_config.device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyPipelineCache(m_config.device, m_pipelineCache, nullptr);
    m_pipelineCache = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyPipelineLayout(m_config.device, m_pipelineLayout, nullptr);
    m_pipelineLayout = VK_NULL_HANDLE;

    m_pDeviceDT->FreeDescriptorSets(m_config.device, m_descPool, 1, &m_descSet);
    m_descSet = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyDescriptorPool(m_config.device, m_descPool, nullptr);
    m_descPool = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyDescriptorSetLayout(m_config.device, m_descSetLayout, nullptr);
    m_descSetLayout = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyRenderPass(m_config.device, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;

    m_pDeviceDT->FreeCommandBuffers(m_config.device, m_cmdPool, 1, &m_cmdBuf);
    m_cmdBuf = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyCommandPool(m_config.device, m_cmdPool, nullptr);
    m_cmdPool = VK_NULL_HANDLE;

}

//-----------------------------------------------------------------------------
/// Perform renderer initialization.
/// \param config The renderer configuration.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::Init(const VktImageRendererConfig& config)
{
    VkResult result = VK_INCOMPLETE;

    if ((config.physicalDevice != VK_NULL_HANDLE) &&
        (config.device != VK_NULL_HANDLE) &&
        (config.queue != VK_NULL_HANDLE))
    {
        memcpy(&m_config, &config, sizeof(m_config));

        m_pInstanceDT = instance_dispatch_table(config.physicalDevice);
        m_pDeviceDT = device_dispatch_table(config.device);

        UINT queueCount = 0;
        m_pInstanceDT->GetPhysicalDeviceQueueFamilyProperties(config.physicalDevice, &queueCount, nullptr);

        VkQueueFamilyProperties* pQueueProps = (VkQueueFamilyProperties*)malloc(queueCount * sizeof(VkQueueFamilyProperties));

        m_pInstanceDT->GetPhysicalDeviceQueueFamilyProperties(config.physicalDevice, &queueCount, pQueueProps);

        m_pInstanceDT->GetPhysicalDeviceMemoryProperties(config.physicalDevice, &m_memProps);

        UINT graphicsQueueNodeIndex = UINT32_MAX;

        for (UINT i = 0; i < queueCount; i++)
        {
            if ((pQueueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                if (graphicsQueueNodeIndex == UINT32_MAX)
                {
                    graphicsQueueNodeIndex = i;
                }
            }
        }

        // Create command pool
        VkCommandPoolCreateInfo cmdPoolCreateInfo = VkCommandPoolCreateInfo();
        cmdPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.pNext            = nullptr;
        cmdPoolCreateInfo.queueFamilyIndex = graphicsQueueNodeIndex;
        cmdPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        result = m_pDeviceDT->CreateCommandPool(m_config.device, &cmdPoolCreateInfo, nullptr, &m_cmdPool);

        // Create command buffer
        if (result == VK_SUCCESS)
        {
            VkCommandBufferAllocateInfo cmdBufAllocInfo = VkCommandBufferAllocateInfo();
            cmdBufAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocInfo.pNext              = nullptr;
            cmdBufAllocInfo.commandPool        = m_cmdPool;
            cmdBufAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocInfo.commandBufferCount = 1;
            result = m_pDeviceDT->AllocateCommandBuffers(m_config.device, &cmdBufAllocInfo, &m_cmdBuf);
        }

        // Create render pass
        if (result == VK_SUCCESS)
        {
            VkAttachmentDescription attachmentDescription[1] = { VkAttachmentDescription() };
            attachmentDescription[0].format         = VK_FORMAT_R8G8B8A8_UNORM;
            attachmentDescription[0].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription[0].initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentDescription[0].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorReference = VkAttachmentReference();
            colorReference.attachment = 0;
            colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = VkSubpassDescription();
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.flags                   = 0;
            subpass.inputAttachmentCount    = 0;
            subpass.pInputAttachments       = nullptr;
            subpass.colorAttachmentCount    = 1;
            subpass.pColorAttachments       = &colorReference;
            subpass.pResolveAttachments     = nullptr;
            subpass.pDepthStencilAttachment = nullptr;
            subpass.preserveAttachmentCount = 0;
            subpass.pPreserveAttachments    = nullptr;

            VkRenderPassCreateInfo rendePassCreateInfo = VkRenderPassCreateInfo();
            rendePassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rendePassCreateInfo.pNext           = nullptr;
            rendePassCreateInfo.attachmentCount = 1;
            rendePassCreateInfo.pAttachments    = attachmentDescription;
            rendePassCreateInfo.subpassCount    = 1;
            rendePassCreateInfo.pSubpasses      = &subpass;
            rendePassCreateInfo.dependencyCount = 0;
            rendePassCreateInfo.pDependencies   = nullptr;
            result = m_pDeviceDT->CreateRenderPass(m_config.device, &rendePassCreateInfo, nullptr, &m_renderPass);
        }

        // Create descriptor pool
        if (result == VK_SUCCESS)
        {
            VkDescriptorPoolSize descPoolSize[3] = { VkDescriptorPoolSize() };
            descPoolSize[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descPoolSize[0].descriptorCount = 1;
            descPoolSize[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descPoolSize[1].descriptorCount = 1;
            descPoolSize[2].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descPoolSize[2].descriptorCount = 1;

            VkDescriptorPoolCreateInfo descPoolCreateInfo = VkDescriptorPoolCreateInfo();
            descPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descPoolCreateInfo.pNext         = nullptr;
            descPoolCreateInfo.maxSets       = 1;
            descPoolCreateInfo.poolSizeCount = 3;
            descPoolCreateInfo.pPoolSizes    = descPoolSize;
            result = m_pDeviceDT->CreateDescriptorPool(m_config.device, &descPoolCreateInfo, nullptr, &m_descPool);
        }

        // Create descriptor set layout
        if (result == VK_SUCCESS)
        {
            VkDescriptorSetLayoutBinding descSetLayoutBindings[3] = { VkDescriptorSetLayoutBinding() };
            descSetLayoutBindings[0].binding            = 0;
            descSetLayoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descSetLayoutBindings[0].descriptorCount    = 1;
            descSetLayoutBindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            descSetLayoutBindings[0].pImmutableSamplers = nullptr;
            descSetLayoutBindings[1].binding            = 1;
            descSetLayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descSetLayoutBindings[1].descriptorCount    = 1;
            descSetLayoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            descSetLayoutBindings[1].pImmutableSamplers = nullptr;
            descSetLayoutBindings[2].binding            = 2;
            descSetLayoutBindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descSetLayoutBindings[2].descriptorCount    = 1;
            descSetLayoutBindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            descSetLayoutBindings[2].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo descLayoutCreateInfo = VkDescriptorSetLayoutCreateInfo();
            descLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descLayoutCreateInfo.pNext        = nullptr;
            descLayoutCreateInfo.bindingCount = 3;
            descLayoutCreateInfo.pBindings    = descSetLayoutBindings;
            result = m_pDeviceDT->CreateDescriptorSetLayout(m_config.device, &descLayoutCreateInfo, nullptr, &m_descSetLayout);
        }

        // Create descriptor set
        if (result == VK_SUCCESS)
        {
            VkDescriptorSetAllocateInfo descSetAllocInfo = VkDescriptorSetAllocateInfo();
            descSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descSetAllocInfo.pNext              = nullptr;
            descSetAllocInfo.descriptorPool     = m_descPool;
            descSetAllocInfo.descriptorSetCount = 1;
            descSetAllocInfo.pSetLayouts        = &m_descSetLayout;
            result = m_pDeviceDT->AllocateDescriptorSets(m_config.device, &descSetAllocInfo, &m_descSet);
        }

        // Create pipeline layout
        if (result == VK_SUCCESS)
        {
            VkPipelineLayoutCreateInfo pipeLayoutCreateInfo = VkPipelineLayoutCreateInfo();
            pipeLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeLayoutCreateInfo.pNext          = nullptr;
            pipeLayoutCreateInfo.setLayoutCount = 1;
            pipeLayoutCreateInfo.pSetLayouts    = &m_descSetLayout;
            result = m_pDeviceDT->CreatePipelineLayout(m_config.device, &pipeLayoutCreateInfo, nullptr, &m_pipelineLayout);
        }

        // Create pipeline cache
        if (result == VK_SUCCESS)
        {
            VkPipelineCacheCreateInfo pipelineCache = VkPipelineCacheCreateInfo();
            pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            result = m_pDeviceDT->CreatePipelineCache(m_config.device, &pipelineCache, nullptr, &m_pipelineCache);
        }

        // Create pipeline
        if (result == VK_SUCCESS)
        {
            VkPipelineVertexInputStateCreateInfo viState = VkPipelineVertexInputStateCreateInfo();
            viState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            VkPipelineInputAssemblyStateCreateInfo iaState = VkPipelineInputAssemblyStateCreateInfo();
            iaState.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            iaState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineRasterizationStateCreateInfo rsState = VkPipelineRasterizationStateCreateInfo();
            rsState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rsState.polygonMode             = VK_POLYGON_MODE_FILL;
            rsState.cullMode                = VK_CULL_MODE_BACK_BIT;
            rsState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rsState.depthClampEnable        = VK_FALSE;
            rsState.rasterizerDiscardEnable = VK_FALSE;
            rsState.depthBiasEnable         = VK_FALSE;

            VkPipelineColorBlendAttachmentState cbAttState[1] = { VkPipelineColorBlendAttachmentState() };
            cbAttState[0].colorWriteMask = 0xf;
            cbAttState[0].blendEnable    = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo cbState = VkPipelineColorBlendStateCreateInfo();
            cbState.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            cbState.attachmentCount = 1;
            cbState.pAttachments    = cbAttState;

            VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE] = { VkDynamicState() };

            VkPipelineDynamicStateCreateInfo dynamicState = VkPipelineDynamicStateCreateInfo();
            dynamicState.sType          = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pDynamicStates = dynamicStateEnables;

            VkPipelineViewportStateCreateInfo vpState = VkPipelineViewportStateCreateInfo();
            vpState.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            vpState.viewportCount                                 = 1;
            dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
            vpState.scissorCount                                  = 1;
            dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

            VkPipelineDepthStencilStateCreateInfo dsState = VkPipelineDepthStencilStateCreateInfo();
            dsState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            dsState.depthTestEnable       = VK_FALSE;
            dsState.depthWriteEnable      = VK_FALSE;
            dsState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
            dsState.depthBoundsTestEnable = VK_FALSE;
            dsState.back.failOp           = VK_STENCIL_OP_KEEP;
            dsState.back.passOp           = VK_STENCIL_OP_KEEP;
            dsState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
            dsState.stencilTestEnable     = VK_FALSE;
            dsState.front                 = dsState.back;

            VkPipelineMultisampleStateCreateInfo msState = VkPipelineMultisampleStateCreateInfo();
            msState.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            msState.pSampleMask          = nullptr;
            msState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            const std::string vertShaderSrc =
#include "FsQuadToBuffer.vert"
                ;

            const std::string fragShaderSrc =
#include "FsQuadToBuffer.frag"
                ;

            VkPipelineShaderStageCreateInfo shaderStages[2] = { VkPipelineShaderStageCreateInfo() };
            result = InitShaders(m_config.device, shaderStages, vertShaderSrc.c_str(), fragShaderSrc.c_str());

            if (result == VK_SUCCESS)
            {
                VkGraphicsPipelineCreateInfo pipeCreateInfo = VkGraphicsPipelineCreateInfo();
                pipeCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipeCreateInfo.layout              = m_pipelineLayout;
                pipeCreateInfo.stageCount          = 2;
                pipeCreateInfo.pVertexInputState   = &viState;
                pipeCreateInfo.pInputAssemblyState = &iaState;
                pipeCreateInfo.pRasterizationState = &rsState;
                pipeCreateInfo.pColorBlendState    = &cbState;
                pipeCreateInfo.pMultisampleState   = &msState;
                pipeCreateInfo.pViewportState      = &vpState;
                pipeCreateInfo.pDepthStencilState  = &dsState;
                pipeCreateInfo.pStages             = shaderStages;
                pipeCreateInfo.renderPass          = m_renderPass;
                pipeCreateInfo.pDynamicState       = &dynamicState;
                result = m_pDeviceDT->CreateGraphicsPipelines(m_config.device, m_pipelineCache, 1, &pipeCreateInfo, nullptr, &m_pipeline);

                m_pDeviceDT->DestroyShaderModule(m_config.device, shaderStages[0].module, nullptr);
                m_pDeviceDT->DestroyShaderModule(m_config.device, shaderStages[1].module, nullptr);
            }
        }

        // Create sampler
        if (result == VK_SUCCESS)
        {
            VkSamplerCreateInfo samplerCreateInfo = VkSamplerCreateInfo();
            samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.pNext                   = nullptr;
            samplerCreateInfo.magFilter               = VK_FILTER_LINEAR;
            samplerCreateInfo.minFilter               = VK_FILTER_LINEAR;
            samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.mipLodBias              = 0.0f;
            samplerCreateInfo.anisotropyEnable        = VK_FALSE;
            samplerCreateInfo.maxAnisotropy           = 1;
            samplerCreateInfo.compareOp               = VK_COMPARE_OP_NEVER;
            samplerCreateInfo.minLod                  = 0.0f;
            samplerCreateInfo.maxLod                  = 0.0f;
            samplerCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
            result = m_pDeviceDT->CreateSampler(m_config.device, &samplerCreateInfo, nullptr, &m_sampler);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Initialize vertex and fragment shaders.
/// \param device The parent device.
/// \param pShaderStages Pointer to shader stages struct.
/// \param pVertShaderText Text for vertex shader.
/// \param pFragShaderText Text for pixel shader.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::InitShaders(
    VkDevice                         device,
    VkPipelineShaderStageCreateInfo* pShaderStages,
    const char*                      pVertShaderText,
    const char*                      pFragShaderText)
{
    VkResult result = VK_INCOMPLETE;

    glslang::InitializeProcess();

    VkShaderModule vsShaderModule = VK_NULL_HANDLE;
    VkShaderModule fsShaderModule = VK_NULL_HANDLE;

    std::vector<UINT> vtxSpv;
    std::vector<UINT> fragSpv;

    result = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, pVertShaderText, vtxSpv);

    if (result == VK_SUCCESS)
    {
        result = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, pFragShaderText, fragSpv);
    }

    if (result == VK_SUCCESS)
    {
        VkShaderModuleCreateInfo vsModuleCreateInfo = VkShaderModuleCreateInfo();
        vsModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vsModuleCreateInfo.pNext    = nullptr;
        vsModuleCreateInfo.flags    = 0;
        vsModuleCreateInfo.codeSize = vtxSpv.size() * sizeof(UINT);
        vsModuleCreateInfo.pCode    = vtxSpv.data();
        result = m_pDeviceDT->CreateShaderModule(device, &vsModuleCreateInfo, nullptr, &vsShaderModule);
    }

    if (result == VK_SUCCESS)
    {
        VkShaderModuleCreateInfo fsModuleCreateInfo = VkShaderModuleCreateInfo();
        fsModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fsModuleCreateInfo.pNext    = nullptr;
        fsModuleCreateInfo.flags    = 0;
        fsModuleCreateInfo.codeSize = fragSpv.size() * sizeof(UINT);
        fsModuleCreateInfo.pCode    = fragSpv.data();
        result = m_pDeviceDT->CreateShaderModule(device, &fsModuleCreateInfo, nullptr, &fsShaderModule);
    }

    if (result == VK_SUCCESS)
    {
        pShaderStages[0].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pShaderStages[0].pNext               = nullptr;
        pShaderStages[0].pSpecializationInfo = nullptr;
        pShaderStages[0].flags               = 0;
        pShaderStages[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
        pShaderStages[0].pName               = "main";
        pShaderStages[0].module              = vsShaderModule;
        pShaderStages[1].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pShaderStages[1].pNext               = nullptr;
        pShaderStages[1].pSpecializationInfo = nullptr;
        pShaderStages[1].flags               = 0;
        pShaderStages[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
        pShaderStages[1].pName               = "main";
        pShaderStages[1].module              = fsShaderModule;
    }

    glslang::FinalizeProcess();

    return result;
}

//-----------------------------------------------------------------------------
/// Build command buffer which will read an image and flatten it into a byte array.
/// \param srcImage The source image to capture.
/// \param prevState The source image's previous state.
/// \param dstWidth The destination RT width.
/// \param dstHeight The destination RT height.
/// \return A handle to the renderer's internal command buffer.
//-----------------------------------------------------------------------------
VkCommandBuffer VktImageRenderer::PrepCmdBuf(
    VkImage        srcImage,
    VkImageLayout  prevState,
    UINT           dstWidth,
    UINT           dstHeight,
    CaptureAssets& assets)
{
    VkCommandBufferInheritanceInfo cmdBufInheritInfo = VkCommandBufferInheritanceInfo();
    cmdBufInheritInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cmdBufInheritInfo.pNext                = nullptr;
    cmdBufInheritInfo.renderPass           = VK_NULL_HANDLE;
    cmdBufInheritInfo.subpass              = 0;
    cmdBufInheritInfo.framebuffer          = VK_NULL_HANDLE;
    cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
    cmdBufInheritInfo.queryFlags           = 0;
    cmdBufInheritInfo.pipelineStatistics   = 0;

    VkCommandBufferBeginInfo cmdBufBeginInfo = VkCommandBufferBeginInfo();
    cmdBufBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufBeginInfo.pNext            = nullptr;
    cmdBufBeginInfo.flags            = 0;
    cmdBufBeginInfo.pInheritanceInfo = &cmdBufInheritInfo;

    VkClearValue clearValues[1] = { VkClearValue() };
    clearValues[0].color.float32[0] = 0.0f;
    clearValues[0].color.float32[1] = 0.0f;
    clearValues[0].color.float32[2] = 0.0f;
    clearValues[0].color.float32[3] = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo();
    renderPassBeginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext                    = nullptr;
    renderPassBeginInfo.renderPass               = m_renderPass;
    renderPassBeginInfo.framebuffer              = assets.frameBuffer;
    renderPassBeginInfo.renderArea.offset.x      = 0;
    renderPassBeginInfo.renderArea.offset.y      = 0;
    renderPassBeginInfo.renderArea.extent.width  = dstWidth;
    renderPassBeginInfo.renderArea.extent.height = dstHeight;
    renderPassBeginInfo.clearValueCount          = 1;
    renderPassBeginInfo.pClearValues             = clearValues;

    VkViewport viewport = VkViewport();
    viewport.width    = (float)dstWidth;
    viewport.height   = (float)dstHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = VkRect2D();
    scissor.extent.width  = dstWidth;
    scissor.extent.height = dstHeight;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;

    m_pDeviceDT->ResetCommandBuffer(m_cmdBuf, 0);

    // Start recording
    m_pDeviceDT->BeginCommandBuffer(m_cmdBuf, &cmdBufBeginInfo);

    // Change image state
    ChangeImageLayout(assets.internalRT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    ChangeImageLayout(srcImage, VK_IMAGE_ASPECT_COLOR_BIT, prevState, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_pDeviceDT->CmdBeginRenderPass(m_cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_pDeviceDT->CmdBindPipeline(m_cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    m_pDeviceDT->CmdBindDescriptorSets(m_cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descSet, 0, nullptr);
    m_pDeviceDT->CmdSetViewport(m_cmdBuf, 0, 1, &viewport);
    m_pDeviceDT->CmdSetScissor(m_cmdBuf, 0, 1, &scissor);
    m_pDeviceDT->CmdDraw(m_cmdBuf, 3, 1, 0, 0);
    m_pDeviceDT->CmdEndRenderPass(m_cmdBuf);

    // Revert image state
    ChangeImageLayout(srcImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, prevState);

    // End recording
    m_pDeviceDT->EndCommandBuffer(m_cmdBuf);

    return m_cmdBuf;
}

//-----------------------------------------------------------------------------
/// FetchResults
/// \param width The destination RT width.
/// \param height The destination RT height.
/// \param assets Capture assets used during capture.
/// \param pImgOut The output image with CPU-visible pixels.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::FetchResults(UINT width, UINT height, CaptureAssets& assets, CpuImage* pImgOut)
{
    VkResult result = VK_INCOMPLETE;

    // Read back results
    const UINT totalBytes = width * height * BytesPerPixel;

    void* pStorageBufferData = nullptr;
    result = m_pDeviceDT->MapMemory(m_config.device, assets.storageBufMem, 0, totalBytes, 0, (void**)&pStorageBufferData);

    if (result == VK_SUCCESS)
    {
        pImgOut->pitch  = width * BytesPerPixel;
        pImgOut->width  = width;
        pImgOut->height = height;
        pImgOut->pData  = new char[totalBytes];

        memcpy(pImgOut->pData, pStorageBufferData, totalBytes);

        m_pDeviceDT->UnmapMemory(m_config.device, assets.storageBufMem);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Convert a Vulkan resource to a CPU - visible linear buffer of pixels.The data is filled  in a user - provided CpuImage struct.
/// IMPORTANT: Memory inside pImgOut is allocated on behalf of the caller, so it is their responsibility to free it.
/// \param settings Input settings for this capture.
/// \param pImgOut Pointer to output image.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::CaptureImage(ImgCaptureSettings& settings, CpuImage* pImgOut)
{
    VkResult result = VK_INCOMPLETE;

    if ((settings.srcImage != VK_NULL_HANDLE) && (settings.srcWidth > 0) && (settings.srcHeight > 0) && (pImgOut != nullptr))
    {
        // Create temp assets used in this capture
        CaptureAssets assets = CaptureAssets();
        result = CreateCaptureAssets(settings.srcImage, settings.dstWidth, settings.dstHeight, settings.flipX, settings.flipY, assets);

        if (result == VK_SUCCESS)
        {
            VkCommandBuffer cmdBuf = PrepCmdBuf(settings.srcImage, settings.prevState, settings.dstWidth, settings.dstHeight, assets);

            if (cmdBuf != VK_NULL_HANDLE)
            {
                VkSubmitInfo submitInfo = VkSubmitInfo();
                submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.pNext                = nullptr;
                submitInfo.waitSemaphoreCount   = 0;
                submitInfo.pWaitSemaphores      = nullptr;
                submitInfo.commandBufferCount   = 1;
                submitInfo.pCommandBuffers      = &cmdBuf;
                submitInfo.signalSemaphoreCount = 0;
                submitInfo.pSignalSemaphores    = nullptr;

                // Submit
                result = m_pDeviceDT->QueueSubmit(m_config.queue, 1, &submitInfo, VK_NULL_HANDLE);

                // Wait for results
                result = m_pDeviceDT->QueueWaitIdle(m_config.queue);

                // Copy results into output buffer
                result = FetchResults(settings.dstWidth, settings.dstHeight, assets, pImgOut);
            }
        }

        // Free temp assets used in this capture
        FreeCaptureAssets(assets);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Create resources that are unique to each capture.
/// \param srcImage The source image.
/// \param dstWidth The destination RT width.
/// \param dstHeight The destination RT height.
/// \param flipX Whether we want to flip the x-axis.
/// \param flipY Whether we want to flip the y-axis.
/// \param assets Output assets used during capture.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::CreateCaptureAssets(
    VkImage        srcImage,
    UINT           dstWidth,
    UINT           dstHeight,
    bool           flipX,
    bool           flipY,
    CaptureAssets& assets)
{
    VkResult result = VK_INCOMPLETE;

    // Create render target
    VkImageCreateInfo rtCreateInfo = VkImageCreateInfo();
    rtCreateInfo.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    rtCreateInfo.pNext       = nullptr;
    rtCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
    rtCreateInfo.format      = VK_FORMAT_R8G8B8A8_UNORM;
    rtCreateInfo.extent      = { dstWidth, dstHeight, 1 };
    rtCreateInfo.mipLevels   = 1;
    rtCreateInfo.arrayLayers = 1;
    rtCreateInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
    rtCreateInfo.tiling      = VK_IMAGE_TILING_LINEAR;
    rtCreateInfo.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    rtCreateInfo.flags       = 0;
    result = m_pDeviceDT->CreateImage(m_config.device, &rtCreateInfo, nullptr, &assets.internalRT);

    if (result == VK_SUCCESS)
    {
        result = AllocBindImageMem(&assets.internalRT, &assets.internalRTMem, nullptr);
    }

    // Create render target view
    if (result == VK_SUCCESS)
    {
        VkImageViewCreateInfo imageViewCreateInfo = VkImageViewCreateInfo();
        imageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext            = nullptr;
        imageViewCreateInfo.format           = VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfo.components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A, };
        imageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.flags            = 0;
        imageViewCreateInfo.image            = assets.internalRT;
        result = m_pDeviceDT->CreateImageView(m_config.device, &imageViewCreateInfo, nullptr, &assets.internalRTView);
    }

    // Create frame buffer
    if (result == VK_SUCCESS)
    {
        VkFramebufferCreateInfo frameBufferCreateInfo = VkFramebufferCreateInfo();
        frameBufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.pNext           = nullptr;
        frameBufferCreateInfo.renderPass      = m_renderPass;
        frameBufferCreateInfo.attachmentCount = 1;
        frameBufferCreateInfo.pAttachments    = &assets.internalRTView;
        frameBufferCreateInfo.width           = dstWidth;
        frameBufferCreateInfo.height          = dstHeight;
        frameBufferCreateInfo.layers          = 1;
        result = m_pDeviceDT->CreateFramebuffer(m_config.device, &frameBufferCreateInfo, nullptr, &assets.frameBuffer);
    }

    // Create source image view
    VkDescriptorImageInfo textureDescs[1] = { VkDescriptorImageInfo() };

    if (result == VK_SUCCESS)
    {
        VkImageViewCreateInfo srcImageViewCreateInfo = VkImageViewCreateInfo();
        srcImageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        srcImageViewCreateInfo.pNext            = nullptr;
        srcImageViewCreateInfo.image            = VK_NULL_HANDLE;
        srcImageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
        srcImageViewCreateInfo.format           = VK_FORMAT_B8G8R8A8_UNORM;
        srcImageViewCreateInfo.components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A, };
        srcImageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        srcImageViewCreateInfo.flags            = 0;
        srcImageViewCreateInfo.image            = srcImage;
        result = m_pDeviceDT->CreateImageView(m_config.device, &srcImageViewCreateInfo, nullptr, &assets.srcImageView);

        textureDescs[0].sampler     = m_sampler;
        textureDescs[0].imageView   = assets.srcImageView;
        textureDescs[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // Setup uniform buffer
    VkDescriptorBufferInfo uniformBufferInfo[1] = { VkDescriptorBufferInfo() };

    if (result == VK_SUCCESS)
    {
        UniformBuffer uniformData = UniformBuffer();
        uniformData.rtWidth = dstWidth;
        uniformData.flipX = flipX ? 1 : 0;
        uniformData.flipY = flipY ? 1 : 0;

        VkBufferCreateInfo uniformBufCreateInfo = VkBufferCreateInfo();
        uniformBufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        uniformBufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        uniformBufCreateInfo.size  = sizeof(uniformData);
        result = m_pDeviceDT->CreateBuffer(m_config.device, &uniformBufCreateInfo, nullptr, &assets.uniformBuf);

        if (result == VK_SUCCESS)
        {
            VkDeviceSize memSize = 0;
            result = AllocBindBufferMem(uniformBufferInfo[0], &assets.uniformBuf, &assets.uniformBufMem, &memSize);

            // Upload data
            if (result == VK_SUCCESS)
            {
                void* pData = nullptr;
                result = m_pDeviceDT->MapMemory(m_config.device, assets.uniformBufMem, 0, memSize, 0, (void**)&pData);

                if (result == VK_SUCCESS)
                {
                    memcpy(pData, &uniformData, sizeof(uniformData));
                    m_pDeviceDT->UnmapMemory(m_config.device, assets.uniformBufMem);
                }
            }
        }
    }

    // Setup storage buffer
    VkDescriptorBufferInfo storageBufferInfo[1] = { VkDescriptorBufferInfo() };

    if (result == VK_SUCCESS)
    {
        const UINT storageBufferSize = dstWidth * dstHeight * BytesPerPixel;

        VkBufferCreateInfo storageBufCreateInfo = VkBufferCreateInfo();
        storageBufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        storageBufCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        storageBufCreateInfo.size  = storageBufferSize;
        result = m_pDeviceDT->CreateBuffer(m_config.device, &storageBufCreateInfo, nullptr, &assets.storageBuf);

        if (result == VK_SUCCESS)
        {
            result = AllocBindBufferMem(storageBufferInfo[0], &assets.storageBuf, &assets.storageBufMem, nullptr);
        }
    }

    // Setup desc set
    VkWriteDescriptorSet writeDescSets[3] = { VkWriteDescriptorSet() };
    writeDescSets[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescSets[0].dstSet          = m_descSet;
    writeDescSets[0].dstBinding      = 0;
    writeDescSets[0].descriptorCount = 1;
    writeDescSets[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescSets[0].pBufferInfo     = uniformBufferInfo;
    writeDescSets[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescSets[1].dstSet          = m_descSet;
    writeDescSets[1].dstBinding      = 1;
    writeDescSets[1].descriptorCount = 1;
    writeDescSets[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescSets[1].pImageInfo      = textureDescs;
    writeDescSets[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescSets[2].dstSet          = m_descSet;
    writeDescSets[2].dstBinding      = 2;
    writeDescSets[2].descriptorCount = 1;
    writeDescSets[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescSets[2].pBufferInfo     = storageBufferInfo;

    m_pDeviceDT->UpdateDescriptorSets(m_config.device, 3, writeDescSets, 0, nullptr);

    return result;
}

//-----------------------------------------------------------------------------
/// Destroy per-capture resources.
/// \param assets The assets to release.
//-----------------------------------------------------------------------------
void VktImageRenderer::FreeCaptureAssets(CaptureAssets& assets)
{
    m_pDeviceDT->FreeMemory(m_config.device, assets.storageBufMem, nullptr);
    assets.storageBufMem = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyBuffer(m_config.device, assets.storageBuf, nullptr);
    assets.storageBuf = VK_NULL_HANDLE;

    m_pDeviceDT->FreeMemory(m_config.device, assets.uniformBufMem, nullptr);
    assets.uniformBufMem = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyBuffer(m_config.device, assets.uniformBuf, nullptr);
    assets.uniformBuf = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyImageView(m_config.device, assets.srcImageView, nullptr);
    assets.srcImageView = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyFramebuffer(m_config.device, assets.frameBuffer, nullptr);
    assets.frameBuffer = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyImageView(m_config.device, assets.internalRTView, nullptr);
    assets.internalRTView = VK_NULL_HANDLE;

    m_pDeviceDT->FreeMemory(m_config.device, assets.internalRTMem, nullptr);
    assets.internalRTMem = VK_NULL_HANDLE;

    m_pDeviceDT->DestroyImage(m_config.device, assets.internalRT, nullptr);
    assets.internalRT = VK_NULL_HANDLE;
}

//-----------------------------------------------------------------------------
/// Allocate and bind CPU-visible memory for a VkImage.
/// \param pImage The image to bind memory to.
/// \param pMem The memory for this image.
/// \param pMemSize The memory size.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::AllocBindImageMem(
    VkImage*        pImage,
    VkDeviceMemory* pMem,
    VkDeviceSize*   pMemSize)
{
    VkResult result = VK_INCOMPLETE;

    if ((pImage != nullptr) && (pMem != nullptr))
    {
        VkMemoryRequirements imageMemReqs = VkMemoryRequirements();
        m_pDeviceDT->GetImageMemoryRequirements(m_config.device, *pImage, &imageMemReqs);

        VkMemoryAllocateInfo memAllocInfo = VkMemoryAllocateInfo();
        memAllocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.pNext          = nullptr;
        memAllocInfo.allocationSize = imageMemReqs.size;

        result = MemTypeFromProps(imageMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);

        if (result == VK_SUCCESS)
        {
            result = m_pDeviceDT->AllocateMemory(m_config.device, &memAllocInfo, nullptr, pMem);
        }

        if (result == VK_SUCCESS)
        {
            result = m_pDeviceDT->BindImageMemory(m_config.device, *pImage, *pMem, 0);
        }

        if (result == VK_SUCCESS)
        {
            if (pMemSize != nullptr)
            {
                *pMemSize = imageMemReqs.size;
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Allocate and bind CPU-visible memory for a VkBuffer.
/// \param bufferInfo Information about this buffer.
/// \param pBuf The buffer.
/// \param pMem The memory for this buffer.
/// \param pMemSize The memory size.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::AllocBindBufferMem(
    VkDescriptorBufferInfo& bufferInfo,
    VkBuffer*               pBuf,
    VkDeviceMemory*         pMem,
    VkDeviceSize*           pMemSize)
{
    VkResult result = VK_INCOMPLETE;

    if ((pBuf != nullptr) && (pMem != nullptr))
    {
        VkMemoryRequirements bufMemReqs = VkMemoryRequirements();
        m_pDeviceDT->GetBufferMemoryRequirements(m_config.device, *pBuf, &bufMemReqs);

        VkMemoryAllocateInfo memAllocInfo = VkMemoryAllocateInfo();
        memAllocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.pNext          = nullptr;
        memAllocInfo.allocationSize = bufMemReqs.size;

        result = MemTypeFromProps(bufMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);

        if (result == VK_SUCCESS)
        {
            result = m_pDeviceDT->AllocateMemory(m_config.device, &memAllocInfo, nullptr, pMem);
        }

        if (result == VK_SUCCESS)
        {
            result = m_pDeviceDT->BindBufferMemory(m_config.device, *pBuf, *pMem, 0);
        }

        if (result == VK_SUCCESS)
        {
            bufferInfo.buffer = *pBuf;
            bufferInfo.offset = 0;
            bufferInfo.range  = memAllocInfo.allocationSize;

            if (pMemSize != nullptr)
            {
                *pMemSize = bufMemReqs.size;
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Utility function to change an image layout/state.
/// \param image The image to transition.
/// \param aspectMask The image's aspect mask.
/// \param prevLayout The image's previous layout.
/// \param newLayout The image's new layout.
//-----------------------------------------------------------------------------
void VktImageRenderer::ChangeImageLayout(
    VkImage            image,
    VkImageAspectFlags aspectMask,
    VkImageLayout      prevLayout,
    VkImageLayout      newLayout)
{
    VkImageMemoryBarrier barrier = VkImageMemoryBarrier();
    barrier.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext            = nullptr;
    barrier.srcAccessMask    = 0;
    barrier.dstAccessMask    = 0;
    barrier.oldLayout        = prevLayout;
    barrier.newLayout        = newLayout;
    barrier.image            = image;
    barrier.subresourceRange = { aspectMask, 0, 1, 0, 1 };

    if (prevLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (prevLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }

    m_pDeviceDT->CmdPipelineBarrier(
        m_cmdBuf,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);
}

//-----------------------------------------------------------------------------
/// Find a memory type from available heaps.
/// \param typeBits The requested mem type.
/// \param reqsMask Mem requirements.
/// \param pTypeIdx The output heap index.
/// \returns A Vulkan result code.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::MemTypeFromProps(UINT typeBits, VkFlags reqsMask, UINT* pTypeIdx)
{
    // Search memory types to find first index with those properties
    for (UINT i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((m_memProps.memoryTypes[i].propertyFlags & reqsMask) == reqsMask)
            {
                *pTypeIdx = i;
                return VK_SUCCESS;
            }
        }

        typeBits >>= 1;
    }

    // No memory types matched, return failure
    return VK_INCOMPLETE;
}

//-----------------------------------------------------------------------------
/// Resizes the input width and height so that their aspect ratio matches the aspect ratio of
/// the input resource. The resulting width and height is never bigger than the input values.
/// \param srcWidth The source width.
/// \param srcHeight The source height.
/// \param dstWidth The destination width.
/// \param dstHeight The destination height.
//-----------------------------------------------------------------------------
void VktImageRenderer::CorrectSizeForApsectRatio(
    UINT  srcWidth,
    UINT  srcHeight,
    UINT& dstWidth,
    UINT& dstHeight)
{
    // Get the destination aspect ratio
    float destAspectRatio = (float)dstWidth / (float)dstHeight;

    // Get the source aspect ratio
    float srcAspectRatio = (float)srcWidth / (float)srcHeight;

    // In most cases we will see images that have an aspect ratio (width/height) greater than 1.0 in the
    // application's frame buffer and the destination's display size
    if (srcAspectRatio > destAspectRatio)
    {
        // In this case the width of the destination is the limiting factor.
        // We need to reduce the height of the destination image (and use its existing width)
        dstHeight = (UINT)((float)dstHeight / (srcAspectRatio / destAspectRatio));
    }
    else
    {
        // In this case the height of the destination is the limiting factor.
        // We need to reduce the width of the destination image (and use its existing height)
        dstWidth = (UINT)((float)dstWidth * (srcAspectRatio / destAspectRatio));
    }
}

//-----------------------------------------------------------------------------
/// Utility function to initialize glslang's TBuiltInResource
/// the input resource. The resulting width and height is never bigger than the input values.
/// \param resources The TBuiltInResources.
//-----------------------------------------------------------------------------
void VktImageRenderer::InitResources(TBuiltInResource& resources)
{
    resources.maxLights = 32;
    resources.maxClipPlanes = 6;
    resources.maxTextureUnits = 32;
    resources.maxTextureCoords = 32;
    resources.maxVertexAttribs = 64;
    resources.maxVertexUniformComponents = 4096;
    resources.maxVaryingFloats = 64;
    resources.maxVertexTextureImageUnits = 32;
    resources.maxCombinedTextureImageUnits = 80;
    resources.maxTextureImageUnits = 32;
    resources.maxFragmentUniformComponents = 4096;
    resources.maxDrawBuffers = 32;
    resources.maxVertexUniformVectors = 128;
    resources.maxVaryingVectors = 8;
    resources.maxFragmentUniformVectors = 16;
    resources.maxVertexOutputVectors = 16;
    resources.maxFragmentInputVectors = 15;
    resources.minProgramTexelOffset = -8;
    resources.maxProgramTexelOffset = 7;
    resources.maxClipDistances = 8;
    resources.maxComputeWorkGroupCountX = 65535;
    resources.maxComputeWorkGroupCountY = 65535;
    resources.maxComputeWorkGroupCountZ = 65535;
    resources.maxComputeWorkGroupSizeX = 1024;
    resources.maxComputeWorkGroupSizeY = 1024;
    resources.maxComputeWorkGroupSizeZ = 64;
    resources.maxComputeUniformComponents = 1024;
    resources.maxComputeTextureImageUnits = 16;
    resources.maxComputeImageUniforms = 8;
    resources.maxComputeAtomicCounters = 8;
    resources.maxComputeAtomicCounterBuffers = 1;
    resources.maxVaryingComponents = 60;
    resources.maxVertexOutputComponents = 64;
    resources.maxGeometryInputComponents = 64;
    resources.maxGeometryOutputComponents = 128;
    resources.maxFragmentInputComponents = 128;
    resources.maxImageUnits = 8;
    resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    resources.maxCombinedShaderOutputResources = 8;
    resources.maxImageSamples = 0;
    resources.maxVertexImageUniforms = 0;
    resources.maxTessControlImageUniforms = 0;
    resources.maxTessEvaluationImageUniforms = 0;
    resources.maxGeometryImageUniforms = 0;
    resources.maxFragmentImageUniforms = 8;
    resources.maxCombinedImageUniforms = 8;
    resources.maxGeometryTextureImageUnits = 16;
    resources.maxGeometryOutputVertices = 256;
    resources.maxGeometryTotalOutputComponents = 1024;
    resources.maxGeometryUniformComponents = 1024;
    resources.maxGeometryVaryingComponents = 64;
    resources.maxTessControlInputComponents = 128;
    resources.maxTessControlOutputComponents = 128;
    resources.maxTessControlTextureImageUnits = 16;
    resources.maxTessControlUniformComponents = 1024;
    resources.maxTessControlTotalOutputComponents = 4096;
    resources.maxTessEvaluationInputComponents = 128;
    resources.maxTessEvaluationOutputComponents = 128;
    resources.maxTessEvaluationTextureImageUnits = 16;
    resources.maxTessEvaluationUniformComponents = 1024;
    resources.maxTessPatchComponents = 120;
    resources.maxPatchVertices = 32;
    resources.maxTessGenLevel = 64;
    resources.maxViewports = 16;
    resources.maxVertexAtomicCounters = 0;
    resources.maxTessControlAtomicCounters = 0;
    resources.maxTessEvaluationAtomicCounters = 0;
    resources.maxGeometryAtomicCounters = 0;
    resources.maxFragmentAtomicCounters = 8;
    resources.maxCombinedAtomicCounters = 8;
    resources.maxAtomicCounterBindings = 1;
    resources.maxVertexAtomicCounterBuffers = 0;
    resources.maxTessControlAtomicCounterBuffers = 0;
    resources.maxTessEvaluationAtomicCounterBuffers = 0;
    resources.maxGeometryAtomicCounterBuffers = 0;
    resources.maxFragmentAtomicCounterBuffers = 1;
    resources.maxCombinedAtomicCounterBuffers = 1;
    resources.maxAtomicCounterBufferSize = 16384;
    resources.maxTransformFeedbackBuffers = 4;
    resources.maxTransformFeedbackInterleavedComponents = 64;
    resources.maxCullDistances = 8;
    resources.maxCombinedClipAndCullDistances = 8;
    resources.maxSamples = 4;
    resources.limits.nonInductiveForLoops = 1;
    resources.limits.whileLoops = 1;
    resources.limits.doWhileLoops = 1;
    resources.limits.generalUniformIndexing = 1;
    resources.limits.generalAttributeMatrixVectorIndexing = 1;
    resources.limits.generalVaryingIndexing = 1;
    resources.limits.generalSamplerIndexing = 1;
    resources.limits.generalVariableIndexing = 1;
    resources.limits.generalConstantMatrixVectorIndexing = 1;
}

//-----------------------------------------------------------------------------
/// Determine EShLanguage for each pipeline state.
/// \param shaderType The shader type.
/// \return resources The TBuiltInResources.
//-----------------------------------------------------------------------------
EShLanguage VktImageRenderer::FindLanguage(const VkShaderStageFlagBits shaderType)
{
    switch (shaderType)
    {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;

        default:
            return EShLangVertex;
    }
}

//-----------------------------------------------------------------------------
/// Convert GLSL to SPIRV.
/// \param shaderType The shader type.
/// \param pShaderStr The shader text.
/// \param spirv The output SPIR-V size.
/// \return VK_SUCCESS if successful.
//-----------------------------------------------------------------------------
VkResult VktImageRenderer::GLSLtoSPV(
    const VkShaderStageFlagBits shaderType,
    const char*                 pShaderStr,
    std::vector<UINT>&          spirv)
{
    glslang::TProgram& program = *new glslang::TProgram;
    const char* pShaderStrings[1];

    TBuiltInResource resources = TBuiltInResource();
    InitResources(resources);

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    EShLanguage stage = FindLanguage(shaderType);
    glslang::TShader* pShader = new glslang::TShader(stage);

    pShaderStrings[0] = pShaderStr;
    pShader->setStrings(pShaderStrings, 1);

    if (pShader->parse(&resources, 100, false, messages) == false)
    {
        puts(pShader->getInfoLog());
        puts(pShader->getInfoDebugLog());

        return VK_INCOMPLETE; // something didn't work
    }

    program.addShader(pShader);

    if (program.link(messages) == false)
    {
        puts(pShader->getInfoLog());
        puts(pShader->getInfoDebugLog());

        return VK_INCOMPLETE;
    }

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

    return VK_SUCCESS;
}
