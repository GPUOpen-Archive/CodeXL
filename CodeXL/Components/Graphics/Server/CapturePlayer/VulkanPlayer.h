//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Vulkan specialization of the base player functionality
//==============================================================================

#ifndef VULKAN_PLAYER_H
#define VULKAN_PLAYER_H

#include "BasePlayer.h"
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
typedef void* HINSTANCE;
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

/// This class implements the features required for a Vulkan capture player
class VulkanPlayer: public BasePlayer
{
    /// Record the last error
    VkResult m_lastErrorResult;

public:

    /// Constructor
    VulkanPlayer()
    {
    }

    /// Destructor
    ~VulkanPlayer()
    {
    }

    bool InitializeWindow(HINSTANCE hInstance, UINT windowWidth, UINT windowHeight);

    bool InitializeGraphics();

    void RenderLoop();

    void Destroy();

    static void Present();
};

#endif //VULKAN_PLAYER_H