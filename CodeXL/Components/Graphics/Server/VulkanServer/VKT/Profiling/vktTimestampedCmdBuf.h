//=================================================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktTimestampedCmdBuf.h
/// \brief  A wrapper for internal command buffer which holds 1 timestamp.
//=================================================================================================

#ifndef __VKT_TIMESTAMPED_CMD_BUF_H__
#define __VKT_TIMESTAMPED_CMD_BUF_H__

#include <vulkan.h>
#include "../Util/vktUtil.h"
#include "../Profiling/vktCmdBufProfiler.h"

//-----------------------------------------------------------------------------
/// Config structure for this object.
//-----------------------------------------------------------------------------
struct TimestampedCmdBufConfig
{
    VkPhysicalDevice        physicalDevice;    ///< The parent device
    VkDevice                device;            ///< The parent device
    UINT                    queueFamilyIndex;  ///< The family index for the queue receiving this cmdBuf
    bool                    mapTimestampMem;   ///< Use vkCmdCopyQueryPoolResults or vkGetQueryPoolResults
    VkPipelineStageFlagBits pipelineLoc;       ///< Where to insert the timestamp
};

//-----------------------------------------------------------------------------
/// The timestamped command buffer.
//-----------------------------------------------------------------------------
class VktTimestampedCmdBuf
{
public:
    static VktTimestampedCmdBuf* Create(const TimestampedCmdBufConfig& config);
    ~VktTimestampedCmdBuf();

    VkResult GetTimestampResult(UINT64* pOutClock);

    VkCommandBuffer CmdBufHandle() { return m_cmdBuf; }

private:
    VktTimestampedCmdBuf();
    VkResult Init(const TimestampedCmdBufConfig& config);

    VkResult MemTypeFromProps(UINT typeBits, VkFlags reqsMask, UINT* pTypeIdx);

    /// Instance dispatch table
    VkLayerInstanceDispatchTable* m_pInstanceDT;

    /// Device dispatch table
    VkLayerDispatchTable* m_pDeviceDT;

    TimestampedCmdBufConfig m_config;

    VkCommandPool        m_cmdPool;
    VkCommandBuffer      m_cmdBuf;
    ProfilerGpuResources m_gpuRes;

    /// GPU memory heap properties
    VkPhysicalDeviceMemoryProperties m_memProps;
};
#endif // __VKT_TIMESTAMPED_CMD_BUF_H__
