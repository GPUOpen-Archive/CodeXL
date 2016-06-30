//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktIntercept.cpp
/// \brief  API interception happens here.
//=============================================================================

#include "vktIntercept.h"

#include "../vktDefines.h"
#include "../vktLayerManager.h"
#include "../vktInterceptManager.h"
#include "../FrameDebugger/vktFrameDebuggerLayer.h"
#include "../Profiling/vktFrameProfilerLayer.h"
#include "../Objects/Wrappers/vktWrappedCmdBuf.h"
#include "../Objects/Wrappers/vktWrappedQueue.h"
#include "../../../Common/misc.h"

static LOADER_PLATFORM_THREAD_ONCE_DECLARATION(initOnce);

//-----------------------------------------------------------------------------
/// Hold whether WSI is enabled in this device
//-----------------------------------------------------------------------------
struct devExts
{
    bool wsiEnabled; ///< Is WSI enabled?
};

//-----------------------------------------------------------------------------
/// Hold whether WSI is enabled in this instance
//-----------------------------------------------------------------------------
struct instExts
{
    bool wsiEnabled; ///< Is WSI enabled?
};

/// Global layer props
static const VkLayerProperties globalLayerProps[] =
{
    {
        "VulkanServer",
        VK_API_VERSION_1_0,
        VK_MAKE_VERSION(0, 1, 0),
        "layer: VulkanServer",
    }
};

/// Device layer props
static const VkLayerProperties deviceLayerProps[] =
{
    {
        "VulkanServer",
        VK_API_VERSION_1_0,
        VK_MAKE_VERSION(0, 1, 0),
        "layer: VulkanServer",
    }
};

/// Map of device extensions
static std::unordered_map<void*, devExts>  s_deviceExtMap;

/// Map of instance extensions
static std::unordered_map<void*, instExts> s_instanceExtMap;

/// Interception manager
static VktInterceptManager* g_pInterceptMgr = nullptr;

/// Profiling layer
static VktFrameProfilerLayer* g_pProfilingLayer = nullptr;

/// Frame debugger layer
static VktFrameDebuggerLayer* g_pFrameDebuggerLayer = nullptr;

/// Container of all commmand buffer wrappers
static WrappedCmdBufMap s_cmdBufWrappers;

/// Container of all commmand buffer wrappers mutex
static mutex s_cmdBufWrappersMutex;

/// Container of all queue wrappers
static WrappedQueueMap s_queueWrappers;

/// Container of all queue wrappers mutex
static mutex s_queueWrappersMutex;

/// Container of all command buffers that have been freed
static std::deque<VktWrappedCmdBuf*> s_cmdBufFreeList;

/// Container of all command buffers that have been freed mutex
static mutex s_cmdBufFreeListMutex;

//-----------------------------------------------------------------------------
/// Release old profiler memory and add given command buffer to delete queue.
/// \param pWrappedCmdBuf The command buffer wrapper being deleted.
//-----------------------------------------------------------------------------
void ProcessCmdBufFreeList(VktWrappedCmdBuf* pWrappedCmdBuf)
{
#if GATHER_PROFILER_RESULTS_WITH_WORKERS

    if (pWrappedCmdBuf != nullptr)
    {
        ScopeLock lock(&s_cmdBufFreeListMutex);

        for (std::deque<VktWrappedCmdBuf*>::iterator it = s_cmdBufFreeList.begin(); it != s_cmdBufFreeList.end();)
        {
            VktWrappedCmdBuf* pCurrCmdBuf = *it;

            bool advanceIter = true;

            if (pCurrCmdBuf != nullptr)
            {
                const UINT originFrame = pCurrCmdBuf->GetOriginFrame();
                const UINT currFrame = VktLayerManager::GetLayerManager()->GetCurrentFrameIndex();
                const UINT frameDifference = currFrame - originFrame;

                if (frameDifference > DEFERRED_RELEASE_FRAME_COUNT)
                {
                    pCurrCmdBuf->ReleaseProfilersMT();
                    it = s_cmdBufFreeList.erase(it);
                    advanceIter = false;
                }
            }

            if (advanceIter)
            {
                ++it;
            }
        }

        s_cmdBufFreeList.push_back(pWrappedCmdBuf);
    }

#else
    UNREFERENCED_PARAMETER(pWrappedCmdBuf);
#endif
}

//-----------------------------------------------------------------------------
/// Given an app's command buffer, obtain its wrapper.
//-----------------------------------------------------------------------------
VktWrappedCmdBuf* GetWrappedCmdBuf(VkCommandBuffer cmdBuffer)
{
    ScopeLock lock(&s_cmdBufWrappersMutex);

    VktWrappedCmdBuf* pOut = nullptr;

    if (cmdBuffer != VK_NULL_HANDLE)
    {
        pOut = s_cmdBufWrappers.at(cmdBuffer);
    }

    VKT_ASSERT(pOut != nullptr);

    return pOut;
}

//-----------------------------------------------------------------------------
/// Given an app's queue, obtain its wrapper.
//-----------------------------------------------------------------------------
VktWrappedQueue* GetWrappedQueue(VkQueue queue)
{
    VktWrappedQueue* pOut = nullptr;

    if (queue != VK_NULL_HANDLE)
    {
        pOut = s_queueWrappers.at(queue);
    }

    VKT_ASSERT(pOut != nullptr);

    return pOut;
}

//-----------------------------------------------------------------------------
/// Return a reference to all known queues.
//-----------------------------------------------------------------------------
const WrappedQueueMap& GetWrappedQueues()
{
    return s_queueWrappers;
}

//-----------------------------------------------------------------------------
/// Create a new command buffer wrapper.
/// \param device The parent device.
/// \param pAllocateInfo The command buffer's alloc info.
/// \param cmdBuf The command buffer's actual handle.
//-----------------------------------------------------------------------------
static void StashCmdBuf(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer cmdBuf)
{
    WrappedCmdBufCreateInfo createInfo = WrappedCmdBufCreateInfo();
    createInfo.physicalDevice = g_pInterceptMgr->FindDeviceInfo(device).physicalDevice;
    createInfo.device         = device;
    createInfo.allocInfo      = *pAllocateInfo;
    createInfo.appCmdBuf      = cmdBuf;
    createInfo.pInterceptMgr  = g_pInterceptMgr;

    VktWrappedCmdBuf* pNewWrapper = VktWrappedCmdBuf::Create(createInfo);

    if (pNewWrapper != nullptr)
    {
        ScopeLock lock(&s_cmdBufWrappersMutex);
        s_cmdBufWrappers[cmdBuf] = pNewWrapper;
    }
}

//-----------------------------------------------------------------------------
/// Create a new queue wrapper.
/// \param device The queue's parent device.
/// \param queueFamilyIndex The queue's family index.
/// \param queueIndex The queue index.
/// \param queue The queue app handle.
//-----------------------------------------------------------------------------
static void StashQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue queue)
{
    VkPhysicalDevice physicalDevice = g_pInterceptMgr->FindDeviceInfo(device).physicalDevice;

    // Query with nullptr data to get count
    UINT queueCount = 0;
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);

    VkQueueFamilyProperties* pQueueProps = (VkQueueFamilyProperties*)malloc(queueCount * sizeof(VkQueueFamilyProperties));
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, pQueueProps);

    WrappedQueueCreateInfo createInfo = WrappedQueueCreateInfo();
    createInfo.physicalDevice   = physicalDevice;
    createInfo.device           = device;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.queueIndex       = queueIndex;
    createInfo.appQueue         = queue;
    createInfo.queueFlags       = pQueueProps[queueIndex].queueFlags;
    createInfo.pInterceptMgr    = g_pInterceptMgr;

    VktWrappedQueue* pNewWrapper = VktWrappedQueue::Create(createInfo);

    if (pNewWrapper != nullptr)
    {
        ScopeLock lock(&s_queueWrappersMutex);
        s_queueWrappers[queue] = pNewWrapper;
    }
}

//-----------------------------------------------------------------------------
/// Initialize our Vulkan server
//-----------------------------------------------------------------------------
static void InitVulkanServer()
{
    g_pInterceptMgr = VktLayerManager::GetLayerManager()->GetInterceptMgr();
    g_pProfilingLayer = static_cast<VktFrameProfilerLayer*>(g_pInterceptMgr->GetParentLayerManager()->GetFrameProfilerLayer());
    g_pFrameDebuggerLayer = static_cast<VktFrameDebuggerLayer*>(g_pInterceptMgr->GetParentLayerManager()->GetFrameDebuggerLayer());
}

//-----------------------------------------------------------------------------
/// CreateDeviceRegisterExtensions
//-----------------------------------------------------------------------------
static void CreateDeviceRegisterExtensions(const VkDeviceCreateInfo* pCreateInfo, VkDevice device)
{
    VkLayerDispatchTable* pDisp = device_dispatch_table(device);
    PFN_vkGetDeviceProcAddr gpa = pDisp->GetDeviceProcAddr;

    pDisp->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)gpa(device, "vkCreateSwapchainKHR");
    pDisp->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)gpa(device, "vkDestroySwapchainKHR");
    pDisp->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)gpa(device, "vkGetSwapchainImagesKHR");
    pDisp->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)gpa(device, "vkAcquireNextImageKHR");
    pDisp->QueuePresentKHR = (PFN_vkQueuePresentKHR)gpa(device, "vkQueuePresentKHR");

    s_deviceExtMap[pDisp].wsiEnabled = false;

    for (UINT i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
        if (strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            s_deviceExtMap[pDisp].wsiEnabled = true;
        }
    }
}

//-----------------------------------------------------------------------------
/// CreateInstanceRegisterExtensions
//-----------------------------------------------------------------------------
static void CreateInstanceRegisterExtensions(const VkInstanceCreateInfo* pCreateInfo, VkInstance instance)
{
    VkLayerInstanceDispatchTable* pDisp = instance_dispatch_table(instance);
    PFN_vkGetInstanceProcAddr gpa = pDisp->GetInstanceProcAddr;

    pDisp->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)gpa(instance, "vkDestroySurfaceKHR");
    pDisp->GetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    pDisp->GetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    pDisp->GetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    pDisp->GetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    pDisp->GetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    pDisp->GetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    pDisp->GetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)gpa(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    pDisp->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)gpa(instance, "vkGetDisplayModePropertiesKHR");
    pDisp->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)gpa(instance, "vkCreateDisplayModeKHR");
    pDisp->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)gpa(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    pDisp->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)gpa(instance, "vkCreateDisplayPlaneSurfaceKHR");

#if VK_USE_PLATFORM_WIN32_KHR
    pDisp->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)gpa(instance, "vkCreateWin32SurfaceKHR");
    pDisp->GetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    pDisp->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)gpa(instance, "vkCreateXcbSurfaceKHR");
    pDisp->GetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    pDisp->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)gpa(instance, "vkCreateXlibSurfaceKHR");
    pDisp->GetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_MIR_KHR
    pDisp->CreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR)gpa(instance, "vkCreateMirSurfaceKHR");
    pDisp->GetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif // VK_USE_PLATFORM_MIR_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    pDisp->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)gpa(instance, "vkCreateWaylandSurfaceKHR");
    pDisp->GetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif //  VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    pDisp->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)gpa(instance, "vkCreateAndroidSurfaceKHR");
#endif // VK_USE_PLATFORM_ANDROID_KHR

    pDisp->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)gpa(instance, "vkCreateDebugReportCallbackEXT");
    pDisp->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)gpa(instance, "vkDestroyDebugReportCallbackEXT");
    pDisp->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)gpa(instance, "vkDebugReportMessageEXT");

    s_instanceExtMap[pDisp].wsiEnabled = false;

    for (UINT i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
        if (strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            s_instanceExtMap[pDisp].wsiEnabled = true;
        }
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    const FuncId funcId = FuncId_vkCreateInstance;

    VkResult result = VK_INCOMPLETE;

    VkLayerInstanceCreateInfo* pChainInfo = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = pChainInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(nullptr, "vkCreateInstance");

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pInstance));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    }

    if (result == VK_SUCCESS)
    {
        initInstanceTable(*pInstance, fpGetInstanceProcAddr);

        CreateInstanceRegisterExtensions(pCreateInfo, *pInstance);
    }

    return result;
}


VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyInstance;

    VkLayerInstanceDispatchTable* pDisp = instance_dispatch_table(instance);

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        pDisp->DestroyInstance(instance, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        pDisp->DestroyInstance(instance, pAllocator);
    }

    s_instanceExtMap.erase(pDisp);

    destroy_instance_dispatch_table(get_dispatch_key(instance));
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
{
    const FuncId funcId = FuncId_vkEnumeratePhysicalDevices;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(instance),
                  *pPhysicalDeviceCount,
                  PrintArrayWithFormatter(*pPhysicalDeviceCount, pPhysicalDevices, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(instance)->EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(instance)->EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceFeatures;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WritePointerAsString(pFeatures));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceFormatProperties;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteFormatEnumAsString(format),
                  VktUtil::WritePointerAsString(pFormatProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceImageFormatProperties;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s, %s, %s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteFormatEnumAsString(format),
                  VktUtil::WriteImageTypeEnumAsString(type),
                  VktUtil::WriteImageTilingEnumAsString(tiling),
                  VktUtil::DecomposeImageUsageFlagsEnumAsString(usage).c_str(),
                  VktUtil::DecomposeImageCreateFlagsEnumAsString(flags).c_str(),
                  VktUtil::WritePointerAsString(pImageFormatProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceProperties;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceProperties(physicalDevice, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceProperties(physicalDevice, pProperties);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceQueueFamilyProperties;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  *pQueueFamilyPropertyCount,
                  VktUtil::WritePointerAsString(pQueueFamilyProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceMemoryProperties;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WritePointerAsString(pMemoryProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    const FuncId funcId = FuncId_vkCreateDevice;

    VkResult result = VK_INCOMPLETE;

    VkLayerDeviceCreateInfo* pChainInfo = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = pChainInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = pChainInfo->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(nullptr, "vkCreateDevice");

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pDevice));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    }

    if (result == VK_SUCCESS)
    {
        initDeviceTable(*pDevice, fpGetDeviceProcAddr);

        CreateDeviceRegisterExtensions(pCreateInfo, *pDevice);

        g_pInterceptMgr->InitializeDeviceInfo(*pDevice, physicalDevice);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyDevice;

    dispatch_key key = get_dispatch_key(device);

    VkLayerDispatchTable* pDisp = device_dispatch_table(device);

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        pDisp->DestroyDevice(device, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        pDisp->DestroyDevice(device, pAllocator);
    }

    s_deviceExtMap.erase(pDisp);

    destroy_device_dispatch_table(key);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties)
{
    const FuncId funcId = FuncId_vkEnumerateInstanceExtensionProperties;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(pLayerName),
                  *pCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = util_GetExtensionProperties(0, nullptr, pCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = util_GetExtensionProperties(0, nullptr, pCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties)
{
    const FuncId funcId = FuncId_vkEnumerateInstanceLayerProperties;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%u, %s",
                  *pCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = util_GetLayerProperties(ARRAY_SIZE(globalLayerProps), globalLayerProps, pCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = util_GetLayerProperties(ARRAY_SIZE(globalLayerProps), globalLayerProps, pCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount, VkLayerProperties* pProperties)
{
    const FuncId funcId = FuncId_vkEnumerateDeviceLayerProperties;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  *pCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = util_GetLayerProperties(ARRAY_SIZE(deviceLayerProps), deviceLayerProps, pCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = util_GetLayerProperties(ARRAY_SIZE(deviceLayerProps), deviceLayerProps, pCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{
    const FuncId funcId = FuncId_vkEnumerateInstanceExtensionProperties;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  pLayerName,
                  *pPropertyCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    const FuncId funcId = FuncId_vkGetDeviceQueue;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  queueFamilyIndex,
                  queueIndex,
                  VktUtil::WritePointerAsString(pQueue));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    }

    StashQueue(device, queueFamilyIndex, queueIndex, *pQueue);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedQueue* pWrappedQueue = GetWrappedQueue(queue);

    if (pWrappedQueue != nullptr)
    {
        result = pWrappedQueue->QueueSubmit(queue, submitCount, pSubmits, fence);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueueWaitIdle(VkQueue queue)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedQueue* pWrappedQueue = GetWrappedQueue(queue);

    if (pWrappedQueue != nullptr)
    {
        result = pWrappedQueue->QueueWaitIdle(queue);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkDeviceWaitIdle(VkDevice device)
{
    const FuncId funcId = FuncId_vkDeviceWaitIdle;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s", VktUtil::WritePointerAsString(device));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->DeviceWaitIdle(device);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->DeviceWaitIdle(device);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    const FuncId funcId = FuncId_vkAllocateMemory;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pAllocateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pMemory));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkFreeMemory;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)memory),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->FreeMemory(device, memory, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->FreeMemory(device, memory, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    const FuncId funcId = FuncId_vkMapMemory;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %llu, %llu, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)memory),
                  offset,
                  size,
                  VktUtil::DecomposeMemoryMapFlagsEnumAsString(flags).c_str(),
                  VktUtil::WritePointerAsString(ppData));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->MapMemory(device, memory, offset, size, flags, ppData);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->MapMemory(device, memory, offset, size, flags, ppData);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
    const FuncId funcId = FuncId_vkUnmapMemory;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)memory));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->UnmapMemory(device, memory);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->UnmapMemory(device, memory);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
{
    const FuncId funcId = FuncId_vkFlushMappedMemoryRanges;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  memoryRangeCount,
                  PrintArrayWithFormatter(memoryRangeCount, pMemoryRanges, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
{
    const FuncId funcId = FuncId_vkInvalidateMappedMemoryRanges;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  memoryRangeCount,
                  PrintArrayWithFormatter(memoryRangeCount, pMemoryRanges, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
{
    const FuncId funcId = FuncId_vkGetDeviceMemoryCommitment;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %llu",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)memory),
                  *pCommittedMemoryInBytes);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    const FuncId funcId = FuncId_vkBindBufferMemory;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %llu",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)buffer),
                  VktUtil::WriteUint64AsString((uint64_t)memory),
                  memoryOffset);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->BindBufferMemory(device, buffer, memory, memoryOffset);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->BindBufferMemory(device, buffer, memory, memoryOffset);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    const FuncId funcId = FuncId_vkBindImageMemory;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %llu",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)image),
                  VktUtil::WriteUint64AsString((uint64_t)memory),
                  memoryOffset);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->BindImageMemory(device, image, memory, memoryOffset);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->BindImageMemory(device, image, memory, memoryOffset);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
    const FuncId funcId = FuncId_vkGetBufferMemoryRequirements;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)buffer),
                  VktUtil::WritePointerAsString(pMemoryRequirements));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
{
    const FuncId funcId = FuncId_vkGetImageMemoryRequirements;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)image),
                  VktUtil::WritePointerAsString(pMemoryRequirements));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetImageMemoryRequirements(device, image, pMemoryRequirements);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetImageMemoryRequirements(device, image, pMemoryRequirements);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
    const FuncId funcId = FuncId_vkGetImageSparseMemoryRequirements;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)image),
                  *pSparseMemoryRequirementCount,
                  PrintArrayWithFormatter(*pSparseMemoryRequirementCount, pSparseMemoryRequirements, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceSparseImageFormatProperties;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %u, %s, %s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteFormatEnumAsString(format),
                  VktUtil::WriteImageTypeEnumAsString(type),
                  samples,
                  VktUtil::DecomposeImageUsageFlagsEnumAsString(usage).c_str(),
                  VktUtil::WriteImageTilingEnumAsString(tiling),
                  *pPropertyCount,
                  PrintArrayWithFormatter(*pPropertyCount, pProperties, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedQueue* pWrappedQueue = GetWrappedQueue(queue);

    if (pWrappedQueue != nullptr)
    {
        result = pWrappedQueue->QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
    const FuncId funcId = FuncId_vkCreateFence;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pFence));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateFence(device, pCreateInfo, pAllocator, pFence);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateFence(device, pCreateInfo, pAllocator, pFence);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyFence;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)fence),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyFence(device, fence, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyFence(device, fence, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
{
    const FuncId funcId = FuncId_vkResetFences;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  fenceCount,
                  PrintArrayWithFormatter(fenceCount, pFences, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->ResetFences(device, fenceCount, pFences);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->ResetFences(device, fenceCount, pFences);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetFenceStatus(VkDevice device, VkFence fence)
{
    const FuncId funcId = FuncId_vkGetFenceStatus;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)fence));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->GetFenceStatus(device, fence);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->GetFenceStatus(device, fence);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
    const FuncId funcId = FuncId_vkWaitForFences;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s, %u, %llu",
                  VktUtil::WritePointerAsString(device),
                  fenceCount,
                  PrintArrayWithFormatter(fenceCount, pFences, POINTER_SUFFIX "%p").c_str(),
                  waitAll,
                  timeout);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->WaitForFences(device, fenceCount, pFences, waitAll, timeout);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
    const FuncId funcId = FuncId_vkCreateSemaphore;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSemaphore));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
#ifdef WIN32
        result = device_dispatch_table(device)->CreateSemaphoreA(device, pCreateInfo, pAllocator, pSemaphore);
#else
        result = device_dispatch_table(device)->CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
#endif
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
#ifdef WIN32
        result = device_dispatch_table(device)->CreateSemaphoreA(device, pCreateInfo, pAllocator, pSemaphore);
#else
        result = device_dispatch_table(device)->CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
#endif
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroySemaphore;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)semaphore),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroySemaphore(device, semaphore, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroySemaphore(device, semaphore, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
    const FuncId funcId = FuncId_vkCreateEvent;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pEvent));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);

#ifdef WIN32
        result = device_dispatch_table(device)->CreateEventA(device, pCreateInfo, pAllocator, pEvent);
#else
        result = device_dispatch_table(device)->CreateEvent(device, pCreateInfo, pAllocator, pEvent);
#endif

        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
#ifdef WIN32
        result = device_dispatch_table(device)->CreateEventA(device, pCreateInfo, pAllocator, pEvent);
#else
        result = device_dispatch_table(device)->CreateEvent(device, pCreateInfo, pAllocator, pEvent);
#endif
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyEvent;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)event),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyEvent(device, event, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyEvent(device, event, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetEventStatus(VkDevice device, VkEvent event)
{
    const FuncId funcId = FuncId_vkGetEventStatus;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)event));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->GetEventStatus(device, event);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->GetEventStatus(device, event);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkSetEvent(VkDevice device, VkEvent event)
{
    const FuncId funcId = FuncId_vkSetEvent;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)event));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->SetEvent(device, event);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->SetEvent(device, event);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetEvent(VkDevice device, VkEvent event)
{
    const FuncId funcId = FuncId_vkResetEvent;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)event));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->ResetEvent(device, event);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->ResetEvent(device, event);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
{
    const FuncId funcId = FuncId_vkCreateQueryPool;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pQueryPool));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyQueryPool;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)queryPool),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyQueryPool(device, queryPool, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyQueryPool(device, queryPool, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
    const FuncId funcId = FuncId_vkGetQueryPoolResults;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %u, %zu, %s, %llu, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)queryPool),
                  firstQuery,
                  queryCount,
                  dataSize,
                  VktUtil::WritePointerAsString(pData),
                  stride,
                  VktUtil::DecomposeQueryResultFlagsEnumAsString(flags).c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    const FuncId funcId = FuncId_vkCreateBuffer;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pBuffer));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyBuffer;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)buffer),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyBuffer(device, buffer, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyBuffer(device, buffer, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
{
    const FuncId funcId = FuncId_vkCreateBufferView;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pView));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateBufferView(device, pCreateInfo, pAllocator, pView);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateBufferView(device, pCreateInfo, pAllocator, pView);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyBufferView;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)bufferView),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyBufferView(device, bufferView, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyBufferView(device, bufferView, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
    const FuncId funcId = FuncId_vkCreateImage;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pImage));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateImage(device, pCreateInfo, pAllocator, pImage);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateImage(device, pCreateInfo, pAllocator, pImage);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyImage;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)image),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyImage(device, image, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyImage(device, image, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
{
    const FuncId funcId = FuncId_vkGetImageSubresourceLayout;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)image),
                  VktUtil::WritePointerAsString(pSubresource),
                  VktUtil::WritePointerAsString(pLayout));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetImageSubresourceLayout(device, image, pSubresource, pLayout);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
    const FuncId funcId = FuncId_vkCreateImageView;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pView));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateImageView(device, pCreateInfo, pAllocator, pView);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateImageView(device, pCreateInfo, pAllocator, pView);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyImageView;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)imageView),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyImageView(device, imageView, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyImageView(device, imageView, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
    const FuncId funcId = FuncId_vkCreateShaderModule;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pShaderModule));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyShaderModule;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)shaderModule),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyShaderModule(device, shaderModule, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyShaderModule(device, shaderModule, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
{
    const FuncId funcId = FuncId_vkCreatePipelineCache;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pPipelineCache));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyPipelineCache;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)pipelineCache),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyPipelineCache(device, pipelineCache, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyPipelineCache(device, pipelineCache, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
{
    const FuncId funcId = FuncId_vkGetPipelineCacheData;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %zu, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)pipelineCache),
                  *pDataSize,
                  VktUtil::WritePointerAsString(pData));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
    const FuncId funcId = FuncId_vkMergePipelineCaches;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)dstCache),
                  srcCacheCount,
                  PrintArrayWithFormatter(srcCacheCount, pSrcCaches, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    const FuncId funcId = FuncId_vkCreateGraphicsPipelines;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)pipelineCache),
                  createInfoCount,
                  PrintArrayWithFormatter(createInfoCount, pCreateInfos, POINTER_SUFFIX "%p").c_str(),
                  VktUtil::WritePointerAsString(pAllocator),
                  PrintArrayWithFormatter(createInfoCount, pPipelines, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    const FuncId funcId = FuncId_vkCreateComputePipelines;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)pipelineCache),
                  createInfoCount,
                  PrintArrayWithFormatter(createInfoCount, pCreateInfos, POINTER_SUFFIX "%p").c_str(),
                  VktUtil::WritePointerAsString(pAllocator),
                  PrintArrayWithFormatter(createInfoCount, pPipelines, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyPipeline;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)pipeline),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyPipeline(device, pipeline, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyPipeline(device, pipeline, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
    const FuncId funcId = FuncId_vkCreatePipelineLayout;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pPipelineLayout));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyPipelineLayout;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)pipelineLayout),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyPipelineLayout(device, pipelineLayout, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyPipelineLayout(device, pipelineLayout, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    const FuncId funcId = FuncId_vkCreateSampler;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSampler));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateSampler(device, pCreateInfo, pAllocator, pSampler);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroySampler;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)sampler),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroySampler(device, sampler, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroySampler(device, sampler, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
    const FuncId funcId = FuncId_vkCreateDescriptorSetLayout;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSetLayout));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyDescriptorSetLayout;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)descriptorSetLayout),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
    const FuncId funcId = FuncId_vkCreateDescriptorPool;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pDescriptorPool));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyDescriptorPool;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)descriptorPool),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyDescriptorPool(device, descriptorPool, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
    const FuncId funcId = FuncId_vkResetDescriptorPool;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)descriptorPool),
                  VktUtil::DecomposeDescriptorPoolCreateFlagsEnumAsString(flags).c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->ResetDescriptorPool(device, descriptorPool, flags);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->ResetDescriptorPool(device, descriptorPool, flags);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
    const FuncId funcId = FuncId_vkAllocateDescriptorSets;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pAllocateInfo),
                  VktUtil::WritePointerAsString(pDescriptorSets));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
{
    const FuncId funcId = FuncId_vkFreeDescriptorSets;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)descriptorPool),
                  descriptorSetCount,
                  VktUtil::WritePointerAsString(pDescriptorSets));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    const FuncId funcId = FuncId_vkUpdateDescriptorSets;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  descriptorWriteCount,
                  PrintArrayWithFormatter(descriptorWriteCount, pDescriptorWrites, POINTER_SUFFIX "%p").c_str(),
                  descriptorCopyCount,
                  PrintArrayWithFormatter(descriptorCopyCount, pDescriptorCopies, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
    const FuncId funcId = FuncId_vkCreateFramebuffer;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pFramebuffer));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyFramebuffer;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)framebuffer),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyFramebuffer(device, framebuffer, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyFramebuffer(device, framebuffer, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    const FuncId funcId = FuncId_vkCreateRenderPass;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pRenderPass));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyRenderPass;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)renderPass),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyRenderPass(device, renderPass, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyRenderPass(device, renderPass, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
{
    const FuncId funcId = FuncId_vkGetRenderAreaGranularity;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)renderPass),
                  VktUtil::WritePointerAsString(pGranularity));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->GetRenderAreaGranularity(device, renderPass, pGranularity);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->GetRenderAreaGranularity(device, renderPass, pGranularity);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    const FuncId funcId = FuncId_vkCreateCommandPool;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pCommandPool));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyCommandPool;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)commandPool),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroyCommandPool(device, commandPool, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroyCommandPool(device, commandPool, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    const FuncId funcId = FuncId_vkResetCommandPool;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)commandPool),
                  VktUtil::DecomposeCmdPoolResetFlagsEnumAsString(flags).c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->ResetCommandPool(device, commandPool, flags);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->ResetCommandPool(device, commandPool, flags);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
    const FuncId funcId = FuncId_vkAllocateCommandBuffers;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pAllocateInfo),
                  VktUtil::WritePointerAsString(pCommandBuffers));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }

    if (result == VK_SUCCESS)
    {
        for (UINT i = 0; i < pAllocateInfo->commandBufferCount; i++)
        {
            StashCmdBuf(device, pAllocateInfo, pCommandBuffers[i]);
        }
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    const FuncId funcId = FuncId_vkFreeCommandBuffers;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)commandPool),
                  commandBufferCount,
                  PrintArrayWithFormatter(commandBufferCount, pCommandBuffers, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }

    for (UINT i = 0; i < commandBufferCount; i++)
    {
        VkCommandBuffer cmdBuf = pCommandBuffers[i];

        if (cmdBuf != VK_NULL_HANDLE)
        {
            VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(cmdBuf);

            if (pWrappedCmdBuf != nullptr)
            {
                pWrappedCmdBuf->Free();

                ProcessCmdBufFreeList(pWrappedCmdBuf);
            }
        }
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        result = pWrappedCmdBuf->BeginCommandBuffer(commandBuffer, pBeginInfo);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        result = pWrappedCmdBuf->EndCommandBuffer(commandBuffer);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        result = pWrappedCmdBuf->ResetCommandBuffer(commandBuffer, flags);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetLineWidth(commandBuffer, lineWidth);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetBlendConstants(commandBuffer, blendConstants);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetStencilReference(commandBuffer, faceMask, reference);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdDispatch(commandBuffer, x, y, z);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdDispatchIndirect(commandBuffer, buffer, offset);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const uint32_t* pData)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdSetEvent(commandBuffer, event, stageMask);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdResetEvent(commandBuffer, event, stageMask);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBeginQuery(commandBuffer, queryPool, query, flags);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdEndQuery(commandBuffer, queryPool, query);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdNextSubpass(commandBuffer, contents);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdEndRenderPass(commandBuffer);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(commandBuffer);

    if (pWrappedCmdBuf != nullptr)
    {
        pWrappedCmdBuf->CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroySurfaceKHR;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WriteUint64AsString((uint64_t)surface),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(instance)->DestroySurfaceKHR(instance, surface, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(instance)->DestroySurfaceKHR(instance, surface, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceSurfaceSupportKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s, %u",
                  VktUtil::WritePointerAsString(physicalDevice),
                  queueFamilyIndex,
                  VktUtil::WriteUint64AsString((uint64_t)surface),
                  *pSupported);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceSurfaceCapabilitiesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteUint64AsString((uint64_t)surface),
                  VktUtil::WritePointerAsString(pSurfaceCapabilities));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceSurfaceFormatsKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteUint64AsString((uint64_t)surface),
                  *pSurfaceFormatCount,
                  PrintArrayWithFormatter(*pSurfaceFormatCount, pSurfaceFormats, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceSurfacePresentModesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteUint64AsString((uint64_t)surface),
                  *pPresentModeCount,
                  PrintArrayWithFormatter(*pPresentModeCount, pPresentModes, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    const FuncId funcId = FuncId_vkCreateSwapchainKHR;

    VkResult result = VK_INCOMPLETE;

    // Need to hack the swapchain image to be readable by shader
    VkSwapchainCreateInfoKHR createInfo = VkSwapchainCreateInfoKHR();
    memcpy(&createInfo, pCreateInfo, sizeof(createInfo));
    createInfo.imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSwapchain));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->CreateSwapchainKHR(device, &createInfo, pAllocator, pSwapchain);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->CreateSwapchainKHR(device, &createInfo, pAllocator, pSwapchain);
    }

    if (result == VK_SUCCESS)
    {
        g_pFrameDebuggerLayer->OnSwapchainCreated(device, *pSwapchain, pCreateInfo->imageExtent);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroySwapchainKHR;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)swapchain),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        device_dispatch_table(device)->DestroySwapchainKHR(device, swapchain, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(device)->DestroySwapchainKHR(device, swapchain, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
{
    const FuncId funcId = FuncId_vkGetSwapchainImagesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)swapchain),
                  *pSwapchainImageCount,
                  PrintArrayWithFormatter(*pSwapchainImageCount, pSwapchainImages, POINTER_SUFFIX "%p").c_str());

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    const FuncId funcId = FuncId_vkAcquireNextImageKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %llu, %s, %s, %s",
                  VktUtil::WritePointerAsString(device),
                  VktUtil::WriteUint64AsString((uint64_t)swapchain),
                  timeout,
                  VktUtil::WriteUint64AsString((uint64_t)semaphore),
                  VktUtil::WriteUint64AsString((uint64_t)fence),
                  VktUtil::WritePointerAsString(pImageIndex));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(device)->AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(device)->AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    VkResult result = VK_INCOMPLETE;

    VktWrappedQueue* pWrappedQueue = GetWrappedQueue(queue);

    if (pWrappedQueue != nullptr)
    {
        result = pWrappedQueue->QueuePresentKHR(queue, pPresentInfo);
    }

    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    const FuncId funcId = FuncId_vkCreateWin32SurfaceKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSurface));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(instance)->CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(instance)->CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkBool32 VKAPI_CALL Mine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceWin32PresentationSupportKHR;

    VkBool32 result = false;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u",
                  VktUtil::WritePointerAsString(physicalDevice),
                  queueFamilyIndex);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    }

    return result;
}

#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    const FuncId funcId = FuncId_vkCreateXcbSurfaceKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSurface));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(instance)->CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(instance)->CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkBool32 VKAPI_CALL Mine_vkGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceXcbPresentationSupportKHR;

    VkBool32 result = false;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u",
                  VktUtil::WritePointerAsString(physicalDevice),
                  queueFamilyIndex);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    }

    return result;
}

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    const FuncId funcId = FuncId_vkCreateXlibSurfaceKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSurface));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(instance)->CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(instance)->CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkBool32 VKAPI_CALL Mine_vkGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceXlibPresentationSupportKHR;

    VkBool32 result = false;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u",
                  VktUtil::WritePointerAsString(physicalDevice),
                  queueFamilyIndex);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    }

    return result;
}

#endif // VK_USE_PLATFORM_XLIB_KHR

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceDisplayPropertiesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  *pPropertyCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties)
{
    const FuncId funcId = FuncId_vkGetPhysicalDeviceDisplayPlanePropertiesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  *pPropertyCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays)
{
    const FuncId funcId = FuncId_vkGetDisplayPlaneSupportedDisplaysKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  planeIndex,
                  *pDisplayCount,
                  VktUtil::WritePointerAsString(pDisplays));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties)
{
    const FuncId funcId = FuncId_vkGetDisplayModePropertiesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteUint64AsString((uint64_t)display),
                  *pPropertyCount,
                  VktUtil::WritePointerAsString(pProperties));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode)
{
    const FuncId funcId = FuncId_vkCreateDisplayModeKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteUint64AsString((uint64_t)display),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pMode));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities)
{
    const FuncId funcId = FuncId_vkGetDisplayPlaneCapabilitiesKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %u, %s",
                  VktUtil::WritePointerAsString(physicalDevice),
                  VktUtil::WriteUint64AsString((uint64_t)mode),
                  planeIndex,
                  VktUtil::WritePointerAsString(pCapabilities));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(physicalDevice)->GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(physicalDevice)->GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    const FuncId funcId = FuncId_vkCreateDisplayPlaneSurfaceKHR;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pSurface));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(instance)->CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(instance)->CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    const FuncId funcId = FuncId_vkCreateDebugReportCallbackEXT;

    VkResult result = VK_INCOMPLETE;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WritePointerAsString(pCreateInfo),
                  VktUtil::WritePointerAsString(pAllocator),
                  VktUtil::WritePointerAsString(pCallback));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = instance_dispatch_table(instance)->CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
        g_pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = instance_dispatch_table(instance)->CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }

    return result;
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    const FuncId funcId = FuncId_vkDestroyDebugReportCallbackEXT;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  VktUtil::WriteUint64AsString((uint64_t)callback),
                  VktUtil::WritePointerAsString(pAllocator));

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(instance)->DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(instance)->DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
}

VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
{
    const FuncId funcId = FuncId_vkDebugReportMessageEXT;

    if (g_pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %u, %u, %llu, %zu, %d, %s, %s",
                  VktUtil::WritePointerAsString(instance),
                  flags,
                  objectType,
                  object,
                  location,
                  messageCode,
                  pLayerPrefix,
                  pMessage);

        VktAPIEntry* pNewEntry = g_pInterceptMgr->PreCall(funcId, argumentsBuffer);
        instance_dispatch_table(instance)->DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
        g_pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        instance_dispatch_table(instance)->DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    }
}

//-----------------------------------------------------------------------------
/// Mine function for GetDeviceProcAddr. This is the function returned if
/// GetDeviceProcAddr is passed in "GetDeviceProcAddr". The name is
/// explicitly called Mine_GetDeviceProcAddr to prevent name clashes with
/// functions of the same name. On Linux, if vkGetDeviceProcAddr is returned,
/// the function pointer in the loader is returned and not the function here
//-----------------------------------------------------------------------------
VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL Mine_vkGetDeviceProcAddr(VkDevice device, const char* funcName)
{
    loader_platform_thread_once(&initOnce, InitVulkanServer);

    // Core interception
    if (!strcmp(funcName, "vkGetDeviceProcAddr"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetDeviceProcAddr;
    }

    if (!strcmp(funcName, "vkDestroyDevice"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyDevice;
    }

    if (!strcmp(funcName, "vkGetDeviceQueue"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetDeviceQueue;
    }

    if (!strcmp(funcName, "vkQueueSubmit"))
    {
        return (PFN_vkVoidFunction)Mine_vkQueueSubmit;
    }

    if (!strcmp(funcName, "vkQueueWaitIdle"))
    {
        return (PFN_vkVoidFunction)Mine_vkQueueWaitIdle;
    }

    if (!strcmp(funcName, "vkDeviceWaitIdle"))
    {
        return (PFN_vkVoidFunction)Mine_vkDeviceWaitIdle;
    }

    if (!strcmp(funcName, "vkAllocateMemory"))
    {
        return (PFN_vkVoidFunction)Mine_vkAllocateMemory;
    }

    if (!strcmp(funcName, "vkMapMemory"))
    {
        return (PFN_vkVoidFunction)Mine_vkMapMemory;
    }

    if (!strcmp(funcName, "vkFlushMappedMemoryRanges"))
    {
        return (PFN_vkVoidFunction)Mine_vkFlushMappedMemoryRanges;
    }

    if (!strcmp(funcName, "vkInvalidateMappedMemoryRanges"))
    {
        return (PFN_vkVoidFunction)Mine_vkInvalidateMappedMemoryRanges;
    }

    if (!strcmp(funcName, "vkCreateFence"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateFence;
    }

    if (!strcmp(funcName, "vkResetFences"))
    {
        return (PFN_vkVoidFunction)Mine_vkResetFences;
    }

    if (!strcmp(funcName, "vkGetFenceStatus"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetFenceStatus;
    }

    if (!strcmp(funcName, "vkWaitForFences"))
    {
        return (PFN_vkVoidFunction)Mine_vkWaitForFences;
    }

    if (!strcmp(funcName, "vkCreateSemaphore"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateSemaphore;
    }

    if (!strcmp(funcName, "vkCreateEvent"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateEvent;
    }

    if (!strcmp(funcName, "vkGetEventStatus"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetEventStatus;
    }

    if (!strcmp(funcName, "vkSetEvent"))
    {
        return (PFN_vkVoidFunction)Mine_vkSetEvent;
    }

    if (!strcmp(funcName, "vkResetEvent"))
    {
        return (PFN_vkVoidFunction)Mine_vkResetEvent;
    }

    if (!strcmp(funcName, "vkCreateQueryPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateQueryPool;
    }

    if (!strcmp(funcName, "vkGetQueryPoolResults"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetQueryPoolResults;
    }

    if (!strcmp(funcName, "vkCreateBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateBuffer;
    }

    if (!strcmp(funcName, "vkCreateBufferView"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateBufferView;
    }

    if (!strcmp(funcName, "vkCreateImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateImage;
    }

    if (!strcmp(funcName, "vkGetImageSubresourceLayout"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetImageSubresourceLayout;
    }

    if (!strcmp(funcName, "vkCreateImageView"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateImageView;
    }

    if (!strcmp(funcName, "vkCreateShaderModule"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateShaderModule;
    }

    if (!strcmp(funcName, "vkCreateGraphicsPipelines"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateGraphicsPipelines;
    }

    if (!strcmp(funcName, "vkCreateComputePipelines"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateComputePipelines;
    }

    if (!strcmp(funcName, "vkCreatePipelineLayout"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreatePipelineLayout;
    }

    if (!strcmp(funcName, "vkCreateSampler"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateSampler;
    }

    if (!strcmp(funcName, "vkCreateDescriptorSetLayout"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateDescriptorSetLayout;
    }

    if (!strcmp(funcName, "vkCreateDescriptorPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateDescriptorPool;
    }

    if (!strcmp(funcName, "vkResetDescriptorPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkResetDescriptorPool;
    }

    if (!strcmp(funcName, "vkAllocateDescriptorSets"))
    {
        return (PFN_vkVoidFunction)Mine_vkAllocateDescriptorSets;
    }

    if (!strcmp(funcName, "vkCmdSetViewport"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetViewport;
    }

    if (!strcmp(funcName, "vkCmdSetScissor"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetScissor;
    }

    if (!strcmp(funcName, "vkCmdSetLineWidth"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetLineWidth;
    }

    if (!strcmp(funcName, "vkCmdSetDepthBias"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetDepthBias;
    }

    if (!strcmp(funcName, "vkCmdSetBlendConstants"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetBlendConstants;
    }

    if (!strcmp(funcName, "vkCmdSetDepthBounds"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetDepthBounds;
    }

    if (!strcmp(funcName, "vkCmdSetStencilCompareMask"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetStencilCompareMask;
    }

    if (!strcmp(funcName, "vkCmdSetStencilWriteMask"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetStencilWriteMask;
    }

    if (!strcmp(funcName, "vkCmdSetStencilReference"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetStencilReference;
    }

    if (!strcmp(funcName, "vkAllocateCommandBuffers"))
    {
        return (PFN_vkVoidFunction)Mine_vkAllocateCommandBuffers;
    }

    if (!strcmp(funcName, "vkBeginCommandBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkBeginCommandBuffer;
    }

    if (!strcmp(funcName, "vkEndCommandBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkEndCommandBuffer;
    }

    if (!strcmp(funcName, "vkResetCommandBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkResetCommandBuffer;
    }

    if (!strcmp(funcName, "vkCmdBindPipeline"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBindPipeline;
    }

    if (!strcmp(funcName, "vkCmdBindDescriptorSets"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBindDescriptorSets;
    }

    if (!strcmp(funcName, "vkCmdBindVertexBuffers"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBindVertexBuffers;
    }

    if (!strcmp(funcName, "vkCmdBindIndexBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBindIndexBuffer;
    }

    if (!strcmp(funcName, "vkCmdDraw"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdDraw;
    }

    if (!strcmp(funcName, "vkCmdDrawIndexed"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdDrawIndexed;
    }

    if (!strcmp(funcName, "vkCmdDrawIndirect"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdDrawIndirect;
    }

    if (!strcmp(funcName, "vkCmdDrawIndexedIndirect"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdDrawIndexedIndirect;
    }

    if (!strcmp(funcName, "vkCmdDispatch"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdDispatch;
    }

    if (!strcmp(funcName, "vkCmdDispatchIndirect"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdDispatchIndirect;
    }

    if (!strcmp(funcName, "vkCmdCopyBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdCopyBuffer;
    }

    if (!strcmp(funcName, "vkCmdCopyImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdCopyImage;
    }

    if (!strcmp(funcName, "vkCmdBlitImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBlitImage;
    }

    if (!strcmp(funcName, "vkCmdCopyBufferToImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdCopyBufferToImage;
    }

    if (!strcmp(funcName, "vkCmdCopyImageToBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdCopyImageToBuffer;
    }

    if (!strcmp(funcName, "vkCmdUpdateBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdUpdateBuffer;
    }

    if (!strcmp(funcName, "vkCmdFillBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdFillBuffer;
    }

    if (!strcmp(funcName, "vkCmdClearColorImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdClearColorImage;
    }

    if (!strcmp(funcName, "vkCmdResolveImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdResolveImage;
    }

    if (!strcmp(funcName, "vkCmdSetEvent"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdSetEvent;
    }

    if (!strcmp(funcName, "vkCmdResetEvent"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdResetEvent;
    }

    if (!strcmp(funcName, "vkCmdWaitEvents"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdWaitEvents;
    }

    if (!strcmp(funcName, "vkCmdPipelineBarrier"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdPipelineBarrier;
    }

    if (!strcmp(funcName, "vkCmdBeginQuery"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBeginQuery;
    }

    if (!strcmp(funcName, "vkCmdEndQuery"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdEndQuery;
    }

    if (!strcmp(funcName, "vkCmdResetQueryPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdResetQueryPool;
    }

    if (!strcmp(funcName, "vkCmdWriteTimestamp"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdWriteTimestamp;
    }

    if (!strcmp(funcName, "vkCmdCopyQueryPoolResults"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdCopyQueryPoolResults;
    }

    if (!strcmp(funcName, "vkCreateFramebuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateFramebuffer;
    }

    if (!strcmp(funcName, "vkCreateRenderPass"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateRenderPass;
    }

    if (!strcmp(funcName, "vkCmdBeginRenderPass"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdBeginRenderPass;
    }

    if (!strcmp(funcName, "vkCmdNextSubpass"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdNextSubpass;
    }

    if (!strcmp(funcName, "vkFreeMemory"))
    {
        return (PFN_vkVoidFunction)Mine_vkFreeMemory;
    }

    if (!strcmp(funcName, "vkUnmapMemory"))
    {
        return (PFN_vkVoidFunction)Mine_vkUnmapMemory;
    }

    if (!strcmp(funcName, "vkGetDeviceMemoryCommitment"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetDeviceMemoryCommitment;
    }

    if (!strcmp(funcName, "vkGetImageSparseMemoryRequirements"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetImageSparseMemoryRequirements;
    }

    if (!strcmp(funcName, "vkGetImageMemoryRequirements"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetImageMemoryRequirements;
    }

    if (!strcmp(funcName, "vkGetBufferMemoryRequirements"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetBufferMemoryRequirements;
    }

    if (!strcmp(funcName, "vkBindImageMemory"))
    {
        return (PFN_vkVoidFunction)Mine_vkBindImageMemory;
    }

    if (!strcmp(funcName, "vkBindBufferMemory"))
    {
        return (PFN_vkVoidFunction)Mine_vkBindBufferMemory;
    }

    if (!strcmp(funcName, "vkQueueBindSparse"))
    {
        return (PFN_vkVoidFunction)Mine_vkQueueBindSparse;
    }

    if (!strcmp(funcName, "vkDestroyFence"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyFence;
    }

    if (!strcmp(funcName, "vkDestroySemaphore"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroySemaphore;
    }

    if (!strcmp(funcName, "vkDestroyEvent"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyEvent;
    }

    if (!strcmp(funcName, "vkDestroyQueryPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyQueryPool;
    }

    if (!strcmp(funcName, "vkDestroyBuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyBuffer;
    }

    if (!strcmp(funcName, "vkDestroyBufferView"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyBufferView;
    }

    if (!strcmp(funcName, "vkDestroyImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyImage;
    }

    if (!strcmp(funcName, "vkDestroyImageView"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyImageView;
    }

    if (!strcmp(funcName, "vkDestroyShaderModule"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyShaderModule;
    }

    if (!strcmp(funcName, "vkCreatePipelineCache"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreatePipelineCache;
    }

    if (!strcmp(funcName, "vkDestroyPipelineCache"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyPipelineCache;
    }

    if (!strcmp(funcName, "vkGetPipelineCacheData"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPipelineCacheData;
    }

    if (!strcmp(funcName, "vkMergePipelineCaches"))
    {
        return (PFN_vkVoidFunction)Mine_vkMergePipelineCaches;
    }

    if (!strcmp(funcName, "vkDestroyPipeline"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyPipeline;
    }

    if (!strcmp(funcName, "vkDestroyPipelineLayout"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyPipelineLayout;
    }

    if (!strcmp(funcName, "vkDestroySampler"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroySampler;
    }

    if (!strcmp(funcName, "vkDestroyDescriptorSetLayout"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyDescriptorSetLayout;
    }

    if (!strcmp(funcName, "vkDestroyDescriptorPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyDescriptorPool;
    }

    if (!strcmp(funcName, "vkFreeDescriptorSets"))
    {
        return (PFN_vkVoidFunction)Mine_vkFreeDescriptorSets;
    }

    if (!strcmp(funcName, "vkUpdateDescriptorSets"))
    {
        return (PFN_vkVoidFunction)Mine_vkUpdateDescriptorSets;
    }

    if (!strcmp(funcName, "vkDestroyFramebuffer"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyFramebuffer;
    }

    if (!strcmp(funcName, "vkDestroyRenderPass"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyRenderPass;
    }

    if (!strcmp(funcName, "vkGetRenderAreaGranularity"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetRenderAreaGranularity;
    }

    if (!strcmp(funcName, "vkCreateCommandPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateCommandPool;
    }

    if (!strcmp(funcName, "vkDestroyCommandPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyCommandPool;
    }

    if (!strcmp(funcName, "vkResetCommandPool"))
    {
        return (PFN_vkVoidFunction)Mine_vkResetCommandPool;
    }

    if (!strcmp(funcName, "vkFreeCommandBuffers"))
    {
        return (PFN_vkVoidFunction)Mine_vkFreeCommandBuffers;
    }

    if (!strcmp(funcName, "vkCmdClearDepthStencilImage"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdClearDepthStencilImage;
    }

    if (!strcmp(funcName, "vkCmdClearAttachments"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdClearAttachments;
    }

    if (!strcmp(funcName, "vkCmdPushConstants"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdPushConstants;
    }

    if (!strcmp(funcName, "vkCmdEndRenderPass"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdEndRenderPass;
    }

    if (!strcmp(funcName, "vkCmdExecuteCommands"))
    {
        return (PFN_vkVoidFunction)Mine_vkCmdExecuteCommands;
    }

    // Extension interception
    VkLayerDispatchTable* pDisp = device_dispatch_table(device);

    if (s_deviceExtMap.size() != 0 && s_deviceExtMap[pDisp].wsiEnabled)
    {
        if (!strcmp("vkCreateSwapchainKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateSwapchainKHR);
        }

        if (!strcmp("vkDestroySwapchainKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkDestroySwapchainKHR);
        }

        if (!strcmp("vkGetSwapchainImagesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetSwapchainImagesKHR);
        }

        if (!strcmp("vkAcquireNextImageKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkAcquireNextImageKHR);
        }

        if (!strcmp("vkQueuePresentKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkQueuePresentKHR);
        }
    }

    if (device == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    if (device_dispatch_table(device)->GetDeviceProcAddr == nullptr)
    {
        return nullptr;
    }

    return device_dispatch_table(device)->GetDeviceProcAddr(device, funcName);
}

//-----------------------------------------------------------------------------
/// External vkGetDeviceProcAddr function.
//-----------------------------------------------------------------------------
VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* funcName)
{
    return Mine_vkGetDeviceProcAddr(device, funcName);
}

//-----------------------------------------------------------------------------
/// Mine function for GetInstanceProcAddr. This is the function returned if
/// GetInstanceProcAddr is passed in "GetInstanceProcAddr". The name is
/// explicitly called Mine_GetDeviceProcAddr to prevent name clashes with
/// functions of the same name. On Linux, if vkGetInstanceProcAddr is returned,
/// the function pointer in the loader is returned and not the function here.
//-----------------------------------------------------------------------------
VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL Mine_vkGetInstanceProcAddr(VkInstance instance, const char* funcName)
{
    loader_platform_thread_once(&initOnce, InitVulkanServer);

    // Core interception
    if (!strcmp(funcName, "vkGetInstanceProcAddr"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetInstanceProcAddr;
    }

    if (!strcmp(funcName, "vkCreateInstance"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateInstance;
    }

    if (!strcmp(funcName, "vkDestroyInstance"))
    {
        return (PFN_vkVoidFunction)Mine_vkDestroyInstance;
    }

    if (!strcmp(funcName, "vkCreateDevice"))
    {
        return (PFN_vkVoidFunction)Mine_vkCreateDevice;
    }

    if (!strcmp(funcName, "vkEnumeratePhysicalDevices"))
    {
        return (PFN_vkVoidFunction)Mine_vkEnumeratePhysicalDevices;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceImageFormatProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceImageFormatProperties;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceProperties;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceQueueFamilyProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceQueueFamilyProperties;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceMemoryProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceMemoryProperties;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceFeatures"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceFeatures;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceFormatProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceFormatProperties;
    }

    if (!strcmp(funcName, "vkGetPhysicalDeviceSparseImageFormatProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkGetPhysicalDeviceSparseImageFormatProperties;
    }

    if (!strcmp(funcName, "vkEnumerateInstanceLayerProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkEnumerateInstanceLayerProperties;
    }

    if (!strcmp(funcName, "vkEnumerateInstanceExtensionProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkEnumerateInstanceExtensionProperties;
    }

    if (!strcmp(funcName, "vkEnumerateDeviceLayerProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkEnumerateDeviceLayerProperties;
    }

    if (!strcmp(funcName, "vkEnumerateDeviceExtensionProperties"))
    {
        return (PFN_vkVoidFunction)Mine_vkEnumerateDeviceExtensionProperties;
    }

    // Extension interception
    VkLayerInstanceDispatchTable* pDisp = instance_dispatch_table(instance);

    if (s_instanceExtMap.size() != 0 && s_instanceExtMap[pDisp].wsiEnabled)
    {
#ifdef VK_USE_PLATFORM_WIN32_KHR

        if (!strcmp("vkCreateWin32SurfaceKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateWin32SurfaceKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceWin32PresentationSupportKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceWin32PresentationSupportKHR);
        }

#endif

#ifdef VK_USE_PLATFORM_XCB_KHR

        if (!strcmp("vkCreateXcbSurfaceKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateXcbSurfaceKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceXcbPresentationSupportKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceXcbPresentationSupportKHR);
        }

#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR

        if (!strcmp("vkCreateXlibSurfaceKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateXlibSurfaceKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceXlibPresentationSupportKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceXlibPresentationSupportKHR);
        }

#endif

        if (!strcmp("vkDestroySurfaceKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkDestroySurfaceKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceSurfaceSupportKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceSurfaceSupportKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceSurfaceFormatsKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceSurfaceFormatsKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceSurfacePresentModesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceSurfacePresentModesKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceDisplayPropertiesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceDisplayPropertiesKHR);
        }

        if (!strcmp("vkGetPhysicalDeviceDisplayPlanePropertiesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetPhysicalDeviceDisplayPlanePropertiesKHR);
        }

        if (!strcmp("vkGetDisplayPlaneSupportedDisplaysKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetDisplayPlaneSupportedDisplaysKHR);
        }

        if (!strcmp("vkGetDisplayModePropertiesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetDisplayModePropertiesKHR);
        }

        if (!strcmp("vkCreateDisplayModeKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateDisplayModeKHR);
        }

        if (!strcmp("vkGetDisplayPlaneCapabilitiesKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkGetDisplayPlaneCapabilitiesKHR);
        }

        if (!strcmp("vkCreateDisplayPlaneSurfaceKHR", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateDisplayPlaneSurfaceKHR);
        }

        if (!strcmp("vkCreateDebugReportCallbackEXT", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkCreateDebugReportCallbackEXT);
        }

        if (!strcmp("vkDestroyDebugReportCallbackEXT", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkDestroyDebugReportCallbackEXT);
        }

        if (!strcmp("vkDebugReportMessageEXT", funcName))
        {
            return reinterpret_cast<PFN_vkVoidFunction>(Mine_vkDebugReportMessageEXT);
        }
    }

    if (instance == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    if (instance_dispatch_table(instance)->GetInstanceProcAddr == nullptr)
    {
        return nullptr;
    }

    return instance_dispatch_table(instance)->GetInstanceProcAddr(instance, funcName);
}

//-----------------------------------------------------------------------------
/// External vkGetInstanceProcAddr function.
//-----------------------------------------------------------------------------
VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* funcName)
{
    return Mine_vkGetInstanceProcAddr(instance, funcName);
}

