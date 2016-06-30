//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktTimestampedCmdBuf.cpp
/// \brief  A wrapper for internal command buffer which holds 1 timestamp.
//==============================================================================

#include "vktTimestampedCmdBuf.h"

//-----------------------------------------------------------------------------
/// Static method that instantiates a VktTimestampedCmdBuf.
/// \param config A configuration structure.
/// \returns A new VktTimestampedCmdBuf instance.
//-----------------------------------------------------------------------------
VktTimestampedCmdBuf* VktTimestampedCmdBuf::Create(const TimestampedCmdBufConfig& config)
{
    VktTimestampedCmdBuf* pOut = new VktTimestampedCmdBuf();

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
VktTimestampedCmdBuf::VktTimestampedCmdBuf() :
    m_pInstanceDT(nullptr),
    m_pDeviceDT(nullptr),
    m_cmdPool(VK_NULL_HANDLE),
    m_cmdBuf(VK_NULL_HANDLE)
{
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
VktTimestampedCmdBuf::~VktTimestampedCmdBuf()
{
    if (m_gpuRes.timestampQueryPool != NULL)
    {
        m_pDeviceDT->DestroyQueryPool(m_config.device, m_gpuRes.timestampQueryPool, nullptr);
        m_gpuRes.timestampQueryPool = NULL;
    }

    if (m_gpuRes.timestampBuffer != NULL)
    {
        m_pDeviceDT->DestroyBuffer(m_config.device, m_gpuRes.timestampBuffer, nullptr);
        m_gpuRes.timestampBuffer = NULL;
    }

    if (m_gpuRes.timestampMem != NULL)
    {
        m_pDeviceDT->FreeMemory(m_config.device, m_gpuRes.timestampMem, nullptr);
        m_gpuRes.timestampMem = NULL;
    }

    if (m_cmdBuf != VK_NULL_HANDLE)
    {
        m_pDeviceDT->FreeCommandBuffers(m_config.device, m_cmdPool, 1, &m_cmdBuf);
        m_cmdBuf = VK_NULL_HANDLE;
    }
}

//-----------------------------------------------------------------------------
/// Perform all initialization.
/// \param config A pointer to a profiler configuration structure.
/// \returns The result code for initialization.
//-----------------------------------------------------------------------------
VkResult VktTimestampedCmdBuf::Init(const TimestampedCmdBufConfig& config)   ///< [in] Pointer to profiler configuration
{
    VkResult result = VK_INCOMPLETE;

    memcpy(&m_config, &config, sizeof(m_config));

    if ((m_config.physicalDevice != VK_NULL_HANDLE) && (m_config.device != VK_NULL_HANDLE))
    {
        m_pInstanceDT = instance_dispatch_table(config.physicalDevice);
        m_pDeviceDT = device_dispatch_table(config.device);

        m_pInstanceDT->GetPhysicalDeviceMemoryProperties(m_config.physicalDevice, &m_memProps);

        // Create command pool
        VkCommandPoolCreateInfo cmdPoolCreateInfo = VkCommandPoolCreateInfo();
        cmdPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.pNext            = nullptr;
        cmdPoolCreateInfo.queueFamilyIndex = m_config.queueFamilyIndex;
        cmdPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        result = m_pDeviceDT->CreateCommandPool(m_config.device, &cmdPoolCreateInfo, nullptr, &m_cmdPool);

        // Create command buffer
        if (result == VK_SUCCESS)
        {
            VkCommandBufferAllocateInfo cmdBufAllocInfo = VkCommandBufferAllocateInfo() ;
            cmdBufAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocInfo.pNext              = nullptr;
            cmdBufAllocInfo.commandPool        = m_cmdPool;
            cmdBufAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocInfo.commandBufferCount = 1;
            result = m_pDeviceDT->AllocateCommandBuffers(m_config.device, &cmdBufAllocInfo, &m_cmdBuf);
        }

        // Create query pool
        if (result == VK_SUCCESS)
        {
            VkQueryPoolCreateInfo queryPoolCreateInfo = VkQueryPoolCreateInfo();
            queryPoolCreateInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            queryPoolCreateInfo.pNext      = nullptr;
            queryPoolCreateInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolCreateInfo.queryCount = 1;
            result = m_pDeviceDT->CreateQueryPool(m_config.device, &queryPoolCreateInfo, nullptr, &m_gpuRes.timestampQueryPool);
        }

        // Create timestamp buffer
        if (result == VK_SUCCESS)
        {
            VkBufferCreateInfo bufferCreateInfo = VkBufferCreateInfo();
            bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.pNext                 = nullptr;
            bufferCreateInfo.usage                 = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferCreateInfo.size                  = sizeof(UINT64);
            bufferCreateInfo.queueFamilyIndexCount = 0;
            bufferCreateInfo.pQueueFamilyIndices   = nullptr;
            bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
            bufferCreateInfo.flags                 = 0;
            result = m_pDeviceDT->CreateBuffer(m_config.device, &bufferCreateInfo, nullptr, &m_gpuRes.timestampBuffer);

            if (result == VK_SUCCESS)
            {
                VkMemoryRequirements memReqs = VkMemoryRequirements();
                m_pDeviceDT->GetBufferMemoryRequirements(m_config.device, m_gpuRes.timestampBuffer, &memReqs);

                VkMemoryAllocateInfo allocInfo = VkMemoryAllocateInfo();
                allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.pNext          = nullptr;
                allocInfo.allocationSize = memReqs.size;

                result = MemTypeFromProps(
                             memReqs.memoryTypeBits,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                             &allocInfo.memoryTypeIndex);

                if (result == VK_SUCCESS)
                {
                    result = m_pDeviceDT->AllocateMemory(m_config.device, &allocInfo, nullptr, &m_gpuRes.timestampMem);

                    if (result == VK_SUCCESS)
                    {
                        result = m_pDeviceDT->BindBufferMemory(m_config.device, m_gpuRes.timestampBuffer, m_gpuRes.timestampMem, 0);

                        if (result == VK_SUCCESS)
                        {
                            void* pMappedMem = nullptr;

                            result = m_pDeviceDT->MapMemory(
                                         m_config.device,
                                         m_gpuRes.timestampMem,
                                         0,
                                         VK_WHOLE_SIZE,
                                         0,
                                         &pMappedMem);

                            if (result == VK_SUCCESS)
                            {
                                memset(pMappedMem, 0, (size_t)memReqs.size);
                                m_pDeviceDT->UnmapMemory(m_config.device, m_gpuRes.timestampMem);
                            }
                        }
                    }
                }
            }
        }

        // Fill in the command buffer
        if (result == VK_SUCCESS)
        {
            VkCommandBufferInheritanceInfo cmdBufInheritInfo = VkCommandBufferInheritanceInfo();
            cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            cmdBufInheritInfo.pNext = nullptr;
            cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
            cmdBufInheritInfo.subpass = 0;
            cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
            cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
            cmdBufInheritInfo.queryFlags = 0;
            cmdBufInheritInfo.pipelineStatistics = 0;

            VkCommandBufferBeginInfo cmdBufBeginInfo = VkCommandBufferBeginInfo();
            cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufBeginInfo.pNext = nullptr;
            cmdBufBeginInfo.flags = 0;
            cmdBufBeginInfo.pInheritanceInfo = &cmdBufInheritInfo;

            result = m_pDeviceDT->BeginCommandBuffer(m_cmdBuf, &cmdBufBeginInfo);

            m_pDeviceDT->CmdResetQueryPool(m_cmdBuf, m_gpuRes.timestampQueryPool, 0, 1);

            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_cmdBuf,
                m_config.pipelineLoc,
                m_gpuRes.timestampQueryPool,
                0);

            if (m_config.mapTimestampMem == true)
            {
                m_pDeviceDT->CmdCopyQueryPoolResults(
                    m_cmdBuf,
                    m_gpuRes.timestampQueryPool,
                    0,
                    1,
                    m_gpuRes.timestampBuffer,
                    0,
                    sizeof(UINT64),
                    VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
            }

            result = m_pDeviceDT->EndCommandBuffer(m_cmdBuf);

        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Find a memory type from available heaps.
/// \param typeBits The requested mem type.
/// \param reqsMask Mem requirements.
/// \param pTypeIdx The output heap index.
/// \returns A Vulkan result code.
//-----------------------------------------------------------------------------
VkResult VktTimestampedCmdBuf::MemTypeFromProps(
    UINT    typeBits,
    VkFlags reqsMask,
    UINT*   pTypeIdx)
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
/// GetTimestampResult
/// \param pOutClock A pointer to a UINT64 which will hold our clock value.
/// \returns The result code for initialization.
//-----------------------------------------------------------------------------
VkResult VktTimestampedCmdBuf::GetTimestampResult(UINT64* pOutClock)
{
    VkResult result = VK_INCOMPLETE;

    if (pOutClock != nullptr)
    {
        UINT64* pTimestampData = nullptr;

        // We use vkCmdCopyQueryPoolResults
        if (m_config.mapTimestampMem == true)
        {
            result = m_pDeviceDT->MapMemory(
                         m_config.device,
                         m_gpuRes.timestampMem,
                         0,
                         VK_WHOLE_SIZE,
                         0,
                         (void**)&pTimestampData);

            *pOutClock = *pTimestampData;
        }

        // We use vkGetQueryPoolResults
        else
        {
            result = m_pDeviceDT->GetQueryPoolResults(
                         m_config.device,
                         m_gpuRes.timestampQueryPool,
                         0,
                         1,
                         sizeof(UINT64),
                         pOutClock,
                         sizeof(UINT64),
                         VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
        }
    }

    return result;
}
