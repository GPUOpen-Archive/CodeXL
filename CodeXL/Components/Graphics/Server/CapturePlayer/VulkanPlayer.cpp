#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>

#include <malloc.h>

#ifdef _DEBUG
#define CP_ASSERT(s) if (s == false) { __debugbreak(); }
#else
#define CP_ASSERT(s)
#endif

#include <stdio.h>

#include "VulkanPlayer.h"

/// Define some Vulkan state.
struct VulkanState
{
    HINSTANCE    hInstance; ///< Application instance.
    HWND         hWnd; ///< Window Handle.
    bool         initComplete; ///< Record if initialization has been done.

    VkSurfaceKHR             surface; ///< Render surface.
    VkInstance               inst; ///< Vulkan instance.
    VkPhysicalDevice         gpu; ///< Physical device.
    VkDevice                 device; ///< Vulkan device.
    VkQueue                  queue; ///< Vulkan queue.
    VkQueueFamilyProperties* pQueueProps; ///< Queue properties.
    VkFormat                 format; ///< Vulkan format.
    VkColorSpaceKHR          colorSpace; ///< Vulkan color space.
    VkSwapchainKHR           swapchain; ///< Swapchain.

    char* pExtNames[64]; ///< Extension names.

    UINT extCount; ///< Extension count.
    UINT width; ///< Render width.
    UINT height; ///< Render height.
    UINT currSwapchainBuffer; ///< Current swapchain render buffer.
    UINT queueCount; ///< Queue count.
    UINT gfxQueueNodeIdx; ///< Queue index
};

/// Vulkan state
static VulkanState s_vkState = {};

/// The application-defined function that processes messages sent to a window.Main message handler
/// \param hWnd A handle to the window.
/// \param uMsg The message.
/// \param wParam Additional message information. The contents of this parameter depend on the value of the uMsg parameter.
/// \param lParam Additional message information. The contents of this parameter depend on the value of the uMsg parameter.
/// \return The return value is the result of the message processing and depends on the message sent.
LRESULT CALLBACK VulkanWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Handle destroy/shutdown messages
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        if (s_vkState.initComplete)
        {
            VulkanPlayer::Present();
        }
        break;
    }

    // Handle any messages the switch statement didn't
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// Vulkan presentation code.
void VulkanPlayer::Present()
{
    VkResult result = VK_INCOMPLETE;

    VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;

    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {};
    presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    presentCompleteSemaphoreCreateInfo.pNext = nullptr;
    presentCompleteSemaphoreCreateInfo.flags = 0;

    result = vkCreateSemaphore(s_vkState.device, &presentCompleteSemaphoreCreateInfo, nullptr, &presentCompleteSemaphore);

    // Get the index of the next available swapchain image
    result = vkAcquireNextImageKHR(
        s_vkState.device,
        s_vkState.swapchain,
        UINT64_MAX,
        presentCompleteSemaphore,
        VK_NULL_HANDLE,
        &s_vkState.currSwapchainBuffer);

    VkPresentInfoKHR present = {};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &s_vkState.swapchain;
    present.pImageIndices = &s_vkState.currSwapchainBuffer;

    result = vkQueuePresentKHR(s_vkState.queue, &present);

    result = vkQueueWaitIdle(s_vkState.queue);

    vkDestroySemaphore(s_vkState.device, presentCompleteSemaphore, nullptr);
}

/// Initialize a render window for Windows.
/// \param hInstance Application instance
/// \param windowWidth The width of the player window
/// \param windowHeight The height of the player window
/// \return True if success, false if failure
bool VulkanPlayer::InitializeWindow(HINSTANCE hInstance, UINT windowWidth, UINT windowHeight)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    m_pPlayerWindow = new WindowsWindow(windowWidth, windowHeight, VulkanWindowProc);

    if (m_pPlayerWindow == NULL)
    {
        return false;
    }

    bool bWindowInitialied = m_pPlayerWindow->Initialize(hInstance);

    if (bWindowInitialied == false)
    {
        return false;
    }

    bool bOpenAndUpdated = m_pPlayerWindow->OpenAndUpdate(SW_MINIMIZE);


    return bOpenAndUpdated;
}

/// Overriden in derived class to initialize the graphics required for a render loop. The render loop acts as a message pump to the user clients.
/// \return True if success, false if failure
bool VulkanPlayer::InitializeGraphics()
{
    m_lastErrorResult = VK_INCOMPLETE;
    UINT instExtCount = 0;
    s_vkState.extCount = 0;

    VkBool32 surfaceExtFound = 0;
    VkBool32 platformSurfaceExtFound = 0;
    memset(s_vkState.pExtNames, 0, sizeof(s_vkState.pExtNames));

    m_lastErrorResult = vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, nullptr);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    if (instExtCount > 0)
    {
        VkExtensionProperties* pInstExts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instExtCount);
        CP_ASSERT(pInstExts != nullptr);

        m_lastErrorResult = vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, pInstExts);
        CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

        for (UINT i = 0; i < instExtCount; i++)
        {
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, pInstExts[i].extensionName))
            {
                surfaceExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }

            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, pInstExts[i].extensionName))
            {
                platformSurfaceExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
        }

        free(pInstExts);
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "APIReplayWindow";
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = "APIReplayWindow";
    appInfo.engineVersion = 0;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instCreateInfo = {};
    instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instCreateInfo.pNext = nullptr;
    instCreateInfo.pApplicationInfo = &appInfo;
    instCreateInfo.enabledLayerCount = 0;
    instCreateInfo.ppEnabledLayerNames = nullptr;
    instCreateInfo.enabledExtensionCount = s_vkState.extCount;
    instCreateInfo.ppEnabledExtensionNames = (const char* const*)s_vkState.pExtNames;

    UINT gpu_count = 0;

    m_lastErrorResult = vkCreateInstance(&instCreateInfo, nullptr, &s_vkState.inst);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    m_lastErrorResult = vkEnumeratePhysicalDevices(s_vkState.inst, &gpu_count, nullptr);
    CP_ASSERT((m_lastErrorResult == VK_SUCCESS) && (gpu_count > 0));

    if (gpu_count > 0)
    {
        VkPhysicalDevice* pPhysicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
        CP_ASSERT(pPhysicalDevices != nullptr);

        m_lastErrorResult = vkEnumeratePhysicalDevices(s_vkState.inst, &gpu_count, pPhysicalDevices);
        CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

        s_vkState.gpu = pPhysicalDevices[0];
        free(pPhysicalDevices);
    }

    UINT deviceExtCount = 0;
    VkBool32 swapchainExtFound = 0;
    s_vkState.extCount = 0;
    memset(s_vkState.pExtNames, 0, sizeof(s_vkState.pExtNames));

    m_lastErrorResult = vkEnumerateDeviceExtensionProperties(s_vkState.gpu, nullptr, &deviceExtCount, nullptr);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    if (deviceExtCount > 0)
    {
        VkExtensionProperties* pDeviceExts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtCount);
        CP_ASSERT(pDeviceExts != nullptr);

        m_lastErrorResult = vkEnumerateDeviceExtensionProperties(s_vkState.gpu, nullptr, &deviceExtCount, pDeviceExts);
        CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

        for (UINT i = 0; i < deviceExtCount; i++)
        {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, pDeviceExts[i].extensionName))
            {
                swapchainExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
        }

        free(pDeviceExts);
    }

    vkGetPhysicalDeviceQueueFamilyProperties(s_vkState.gpu, &s_vkState.queueCount, nullptr);

    s_vkState.pQueueProps = (VkQueueFamilyProperties*)malloc(s_vkState.queueCount * sizeof(VkQueueFamilyProperties));
    CP_ASSERT(s_vkState.pQueueProps != nullptr);

    vkGetPhysicalDeviceQueueFamilyProperties(s_vkState.gpu, &s_vkState.queueCount, s_vkState.pQueueProps);
    CP_ASSERT((s_vkState.queueCount >= 1) == true);

    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = s_vkState.hInstance;
    createInfo.hwnd = s_vkState.hWnd;

    m_lastErrorResult = vkCreateWin32SurfaceKHR(s_vkState.inst, &createInfo, nullptr, &s_vkState.surface);

    VkBool32* pSupportsPresent = (VkBool32 *)malloc(s_vkState.queueCount * sizeof(VkBool32));
    CP_ASSERT(pSupportsPresent != nullptr);

    for (UINT i = 0; i < s_vkState.queueCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(s_vkState.gpu, i, s_vkState.surface, &pSupportsPresent[i]);
    }

    UINT graphicsQueueNodeIndex = UINT32_MAX;
    UINT presentQueueNodeIndex = UINT32_MAX;
    for (UINT i = 0; i < s_vkState.queueCount; i++)
    {
        if ((s_vkState.pQueueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (graphicsQueueNodeIndex == UINT32_MAX)
            {
                graphicsQueueNodeIndex = i;
            }

            if (pSupportsPresent[i] == VK_TRUE)
            {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    if (presentQueueNodeIndex == UINT32_MAX)
    {
        for (UINT i = 0; i < s_vkState.queueCount; ++i)
        {
            if (pSupportsPresent[i] == VK_TRUE)
            {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    free(pSupportsPresent);

    s_vkState.gfxQueueNodeIdx = graphicsQueueNodeIndex;

    float queuePriorities[1] = { 0.0 };

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.queueFamilyIndex = s_vkState.gfxQueueNodeIdx;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo device = {};
    device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device.pNext = nullptr;
    device.queueCreateInfoCount = 1;
    device.pQueueCreateInfos = &queueCreateInfo;
    device.enabledLayerCount = 0;
    device.ppEnabledLayerNames = nullptr;
    device.enabledExtensionCount = s_vkState.extCount;
    device.ppEnabledExtensionNames = (const char* const*)s_vkState.pExtNames;

    m_lastErrorResult = vkCreateDevice(s_vkState.gpu, &device, nullptr, &s_vkState.device);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    vkGetDeviceQueue(s_vkState.device, s_vkState.gfxQueueNodeIdx, 0, &s_vkState.queue);

    UINT formatCount = 0;
    m_lastErrorResult = vkGetPhysicalDeviceSurfaceFormatsKHR(s_vkState.gpu, s_vkState.surface, &formatCount, nullptr);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    VkSurfaceFormatKHR* pSurfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    CP_ASSERT(pSurfFormats != nullptr);

    m_lastErrorResult = vkGetPhysicalDeviceSurfaceFormatsKHR(s_vkState.gpu, s_vkState.surface, &formatCount, pSurfFormats);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    if ((formatCount == 1) && (pSurfFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        s_vkState.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        CP_ASSERT((formatCount >= 1) == true);
        s_vkState.format = pSurfFormats[0].format;
    }

    s_vkState.colorSpace = pSurfFormats[0].colorSpace;

    VkSwapchainKHR oldSwapchain = s_vkState.swapchain;

    VkSurfaceCapabilitiesKHR surfCapabilities = {};
    m_lastErrorResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_vkState.gpu, s_vkState.surface, &surfCapabilities);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    UINT presentModeCount = 0;
    m_lastErrorResult = vkGetPhysicalDeviceSurfacePresentModesKHR(s_vkState.gpu, s_vkState.surface, &presentModeCount, nullptr);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    VkPresentModeKHR* pPresentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    CP_ASSERT(pPresentModes != nullptr);

    m_lastErrorResult = vkGetPhysicalDeviceSurfacePresentModesKHR(s_vkState.gpu, s_vkState.surface, &presentModeCount, pPresentModes);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    free(pPresentModes);

    VkExtent2D swapchainExtent = {};
    if (surfCapabilities.currentExtent.width == (UINT)-1)
    {
        swapchainExtent.width = s_vkState.width;
        swapchainExtent.height = s_vkState.height;
    }
    else
    {
        swapchainExtent = surfCapabilities.currentExtent;
        s_vkState.width = surfCapabilities.currentExtent.width;
        s_vkState.height = surfCapabilities.currentExtent.height;
    }

    UINT desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
    if ((surfCapabilities.maxImageCount > 0) &&
        (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount))
    {
        desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagBitsKHR preTransform = surfCapabilities.currentTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.surface = s_vkState.surface;
    swapchainCreateInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapchainCreateInfo.imageFormat = s_vkState.format;
    swapchainCreateInfo.imageColorSpace = s_vkState.colorSpace;
    swapchainCreateInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = preTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.oldSwapchain = oldSwapchain;
    swapchainCreateInfo.clipped = true;

    m_lastErrorResult = vkCreateSwapchainKHR(s_vkState.device, &swapchainCreateInfo, nullptr, &s_vkState.swapchain);
    CP_ASSERT(m_lastErrorResult == VK_SUCCESS);

    if (oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(s_vkState.device, oldSwapchain, nullptr);
    }

    s_vkState.currSwapchainBuffer = 0;
    s_vkState.initComplete = true;


    if (m_lastErrorResult == VK_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/// Implement the render loop.
void VulkanPlayer::RenderLoop()
{
    s_vkState.width = m_windowWidth;
    s_vkState.height = m_windowHeight;
    s_vkState.hInstance = m_pPlayerWindow->GetHINSTANCE();
    s_vkState.hWnd = m_pPlayerWindow->GetWindowHandle();

    //VkResult result = VK_INCOMPLETE;
    //result = VulkanInit();
    //CP_ASSERT(result == VK_SUCCESS);

    MSG msg = {};

    for (;;)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }

        RedrawWindow(s_vkState.hWnd, nullptr, nullptr, RDW_INTERNALPAINT);
    }
}

/// Perfrom any cleanup here
void VulkanPlayer::Destroy()
{
    s_vkState.initComplete = false;

    vkDestroySwapchainKHR(s_vkState.device, s_vkState.swapchain, nullptr);
    vkDestroyDevice(s_vkState.device, nullptr);
    vkDestroySurfaceKHR(s_vkState.inst, s_vkState.surface, nullptr);
    vkDestroyInstance(s_vkState.inst, nullptr);

    free(s_vkState.pQueueProps);
}
