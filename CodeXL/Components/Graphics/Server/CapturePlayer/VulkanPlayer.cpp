#include <malloc.h>
#include <stdio.h>

#include "../Common/Logger.h"
#include "VulkanPlayer.h"

#ifdef WIN32
#include "WindowsWindow.h"
#else
#include "XcbWindow.h"
#include <signal.h>
#include "WinDefs.h"
#define SW_MINIMIZE 1
#endif

#ifdef _DEBUG
    #ifdef WIN32
        #define CP_ASSERT(s) if (s == false) { __debugbreak(); }
    #else
        #define CP_ASSERT(s) if (!(s)) { raise(SIGTRAP); }
    #endif
#else
    #define CP_ASSERT(s)
#endif

/// Info about a swapchain
struct SwapchainBuffers {
    VkImage     image;
    VkImageView view;
};

/// Define some Vulkan state.
struct VulkanState
{
    NativeInstanceType       hInstance;      ///< Application instance.
    NativeWindowType         hWnd;           ///< Window Handle.

    VkSurfaceKHR             surface;        ///< Vulkan render surface.
    VkInstance               inst;           ///< Vulkan instance.
    VkPhysicalDevice         gpu;            ///< Vulkan physical device.
    VkDevice                 device;         ///< Vulkan device.
    VkQueue                  queue;          ///< Vulkan queue.
    VkQueueFamilyProperties* pQueueProps;    ///< Vulkan queue properties.
    VkFormat                 format;         ///< Vulkan format.
    VkColorSpaceKHR          colorSpace;     ///< Vulkan color space.
    VkSwapchainKHR           swapchain;      ///< Vulkan swapchain.

    VkRenderPass             renderPass;         ///< Vulkan render pass.
    VkCommandPool            cmdPool;            ///< Vulkan command pool.
    VkCommandBuffer          cmdBuf;             ///< Vulkan command pool.
    VkFramebuffer*           pFrameBuffers;      ///< Vulkan framebuffers.
    SwapchainBuffers*        pSwapchainBuffers;  ///< Wraps data about a swapchain.

    char* pExtNames[64];        ///< Extension names.

    UINT swapchainImgCount;     ///< Number of swapchain images.
    UINT extCount;              ///< Extension count.
    UINT width;                 ///< Render width.
    UINT height;                ///< Render height.
    UINT currSwapchainBuffer;   ///< Current swapchain render buffer.
    UINT queueCount;            ///< Queue count.
    UINT gfxQueueNodeIdx;       ///< Queue index.
};

/// Vulkan state
static VulkanState s_vkState = {};

// local Vulkan surface helper classes. Used to encapsulate the Vulkan surface info
// depending on the underlying window interface. Use methods to return the data type
// so code can be switched out easily
class VulkanSurfaceBase
{
public:
    VulkanSurfaceBase() {}

    virtual ~VulkanSurfaceBase() {}

    /// Is the value passed in a valid surface extension for this window type
    /// \param extensionName the name of the extension to check
    /// \return true if the extension name is valid, false if not
    virtual bool IsSurfaceExtension(const char* extensionName) = 0;

    /// Get the surface extension name for this window type
    /// \return this window type's surface extension name
    virtual char* GetSurfaceExtensionName() = 0;

    virtual VkResult CreateSurface() = 0;
};

static VulkanSurfaceBase* s_vulkanSurface = nullptr;

#ifdef _LINUX

// Linux-only XCB Surface helper class
class VulkanSurfaceXCB : public VulkanSurfaceBase
{
public:
    VulkanSurfaceXCB() {}
    virtual ~VulkanSurfaceXCB() {}

    /// Is the value passed in a valid surface extension for this window type
    /// \param extensionName the name of the extension to check
    /// \return true if the extension name is valid, false if not
    virtual bool IsSurfaceExtension(const char* extensionName)
    {
        if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, extensionName))
        {
            return true;
        }
        return false;
    }

    /// Get the surface extension name for this window type
    /// \return this window type's surface extension name
    virtual char* GetSurfaceExtensionName()
    {
        static char* extensionName = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        return extensionName;
    }

    virtual VkResult CreateSurface()
    {
        VkXcbSurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.connection = s_vkState.hInstance;
        createInfo.window = s_vkState.hWnd;

        return vkCreateXcbSurfaceKHR(s_vkState.inst, &createInfo, nullptr, &s_vkState.surface);
    }
};

#if 0
// Linux-only X11 Surface helper class
class VulkanSurfaceX11 : public VulkanSurfaceBase
{
public:
    VulkanX11() {}
    virtual ~VulkanSurfaceX11() {}

    /// Is the value passed in a valid surface extension for this window type
    /// \param extensionName the name of the extension to check
    /// \return true if the extension name is valid, false if not
    virtual bool IsSurfaceExtension(const char* extensionName)
    {
        if (!strcmp(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, extensionName))
        {
            return true;
        }
        return false;
    }

    /// Get the surface extension name for this window type
    /// \return this window type's surface extension name
    virtual char* GetSurfaceExtensionName()
    {
        static char* extensionName = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
        return extensionName;
    }

    virtual VkResult CreateSurface()
    {
        VkXlibSurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.dpy = s_vkState.hInstance;
        createInfo.window = s_vkState.hWnd;

        return vkCreateXlibSurfaceKHR(s_vkState.inst, &createInfo, nullptr, &s_vkState.surface);
    }
};
#endif

#endif // LINUX

/// The application-defined function that processes messages sent to a window.Main message handler
/// \param hWnd A handle to the window.
/// \param uMsg The message.
/// \param wParam Additional message information. The contents of this parameter depend on the value of the uMsg parameter.
/// \param lParam Additional message information. The contents of this parameter depend on the value of the uMsg parameter.
/// \return The return value is the result of the message processing and depends on the message sent.
#ifdef WIN32
LRESULT CALLBACK VulkanWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Handle destroy/shutdown messages
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif

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

    ClearSwapchainImage();

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
#ifdef WIN32
    m_pPlayerWindow = new WindowsWindow(windowWidth, windowHeight, hInstance, VulkanWindowProc);
#else
    // choose window type
    m_pPlayerWindow = new XcbWindow(windowWidth, windowHeight);

    // choose surface helper type
    s_vulkanSurface = new VulkanSurfaceXCB();
#endif

    if (m_pPlayerWindow == nullptr)
    {
        return false;
    }

    bool bWindowInitialied = m_pPlayerWindow->Initialize();

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
    VkResult result = VK_INCOMPLETE;

    s_vkState.width = m_pPlayerWindow->GetWindowWidth();
    s_vkState.height = m_pPlayerWindow->GetWindowHeight();
    s_vkState.hInstance = m_pPlayerWindow->GetInstance();
    s_vkState.hWnd = m_pPlayerWindow->GetWindowHandle();

    UINT instExtCount = 0;
    s_vkState.extCount = 0;

    VkBool32 surfaceExtFound = 0;
    VkBool32 platformSurfaceExtFound = 0;
    memset(s_vkState.pExtNames, 0, sizeof(s_vkState.pExtNames));

    result = vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, nullptr);
    CP_ASSERT(result == VK_SUCCESS);

    if (instExtCount > 0)
    {
        VkExtensionProperties* pInstExts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instExtCount);
        CP_ASSERT(pInstExts != nullptr);

        result = vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, pInstExts);
        CP_ASSERT(result == VK_SUCCESS);

        for (UINT i = 0; i < instExtCount; i++)
        {
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, pInstExts[i].extensionName))
            {
                surfaceExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = (char*)VK_KHR_SURFACE_EXTENSION_NAME;
            }
#ifdef WIN32
            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, pInstExts[i].extensionName))
            {
                platformSurfaceExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = (char*)VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
#else
            if (s_vulkanSurface->IsSurfaceExtension(pInstExts[i].extensionName))
            {
                platformSurfaceExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = s_vulkanSurface->GetSurfaceExtensionName();
            }
#endif
        }

        free(pInstExts);
    }

    if (!surfaceExtFound)
    {
        Log(logERROR, "Failed to find the " VK_KHR_SURFACE_EXTENSION_NAME " extension.\n");
        return false;
    }

    if (!platformSurfaceExtFound)
    {
#ifdef WIN32
        Log(logERROR, "Failed to find the %s extension.\n", (char*)VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
        Log(logERROR, "Failed to find the %s extension.\n", s_vulkanSurface->GetSurfaceExtensionName());
#endif
        return false;
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

    result = vkCreateInstance(&instCreateInfo, nullptr, &s_vkState.inst);
    CP_ASSERT(result == VK_SUCCESS);

    UINT gpu_count = 0;
    result = vkEnumeratePhysicalDevices(s_vkState.inst, &gpu_count, nullptr);
    CP_ASSERT((result == VK_SUCCESS) && (gpu_count > 0));

    if (gpu_count > 0)
    {
        VkPhysicalDevice* pPhysicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
        CP_ASSERT(pPhysicalDevices != nullptr);

        result = vkEnumeratePhysicalDevices(s_vkState.inst, &gpu_count, pPhysicalDevices);
        CP_ASSERT(result == VK_SUCCESS);

        s_vkState.gpu = pPhysicalDevices[0];
        free(pPhysicalDevices);
    }

    UINT deviceExtCount = 0;
    VkBool32 swapchainExtFound = 0;
    s_vkState.extCount = 0;
    memset(s_vkState.pExtNames, 0, sizeof(s_vkState.pExtNames));

    result = vkEnumerateDeviceExtensionProperties(s_vkState.gpu, nullptr, &deviceExtCount, nullptr);
    CP_ASSERT(result == VK_SUCCESS);

    if (deviceExtCount > 0)
    {
        VkExtensionProperties* pDeviceExts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtCount);
        CP_ASSERT(pDeviceExts != nullptr);

        result = vkEnumerateDeviceExtensionProperties(s_vkState.gpu, nullptr, &deviceExtCount, pDeviceExts);
        CP_ASSERT(result == VK_SUCCESS);

        for (UINT i = 0; i < deviceExtCount; i++)
        {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, pDeviceExts[i].extensionName))
            {
                swapchainExtFound = 1;
                s_vkState.pExtNames[s_vkState.extCount++] = (char*)VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
        }

        free(pDeviceExts);
    }

    vkGetPhysicalDeviceQueueFamilyProperties(s_vkState.gpu, &s_vkState.queueCount, nullptr);

    s_vkState.pQueueProps = (VkQueueFamilyProperties*)malloc(s_vkState.queueCount * sizeof(VkQueueFamilyProperties));
    CP_ASSERT(s_vkState.pQueueProps != nullptr);

    vkGetPhysicalDeviceQueueFamilyProperties(s_vkState.gpu, &s_vkState.queueCount, s_vkState.pQueueProps);
    CP_ASSERT((s_vkState.queueCount >= 1) == true);

#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = s_vkState.hInstance;
    createInfo.hwnd = s_vkState.hWnd;

    result = vkCreateWin32SurfaceKHR(s_vkState.inst, &createInfo, nullptr, &s_vkState.surface);
#else
    result = s_vulkanSurface->CreateSurface();
#endif

    CP_ASSERT(result == VK_SUCCESS);

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

    result = vkCreateDevice(s_vkState.gpu, &device, nullptr, &s_vkState.device);
    CP_ASSERT(result == VK_SUCCESS);

    vkGetDeviceQueue(s_vkState.device, s_vkState.gfxQueueNodeIdx, 0, &s_vkState.queue);

    UINT formatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(s_vkState.gpu, s_vkState.surface, &formatCount, nullptr);
    CP_ASSERT(result == VK_SUCCESS);

    VkSurfaceFormatKHR* pSurfFormats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    CP_ASSERT(pSurfFormats != nullptr);

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(s_vkState.gpu, s_vkState.surface, &formatCount, pSurfFormats);
    CP_ASSERT(result == VK_SUCCESS);

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
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_vkState.gpu, s_vkState.surface, &surfCapabilities);
    CP_ASSERT(result == VK_SUCCESS);

    UINT presentModeCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(s_vkState.gpu, s_vkState.surface, &presentModeCount, nullptr);
    CP_ASSERT(result == VK_SUCCESS);

    VkPresentModeKHR* pPresentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    CP_ASSERT(pPresentModes != nullptr);

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(s_vkState.gpu, s_vkState.surface, &presentModeCount, pPresentModes);
    CP_ASSERT(result == VK_SUCCESS);

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

    s_vkState.swapchainImgCount = surfCapabilities.minImageCount + 1;

    if ((surfCapabilities.maxImageCount > 0) && (s_vkState.swapchainImgCount > surfCapabilities.maxImageCount))
    {
        s_vkState.swapchainImgCount = surfCapabilities.maxImageCount;
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
    swapchainCreateInfo.minImageCount = s_vkState.swapchainImgCount;
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

    result = vkCreateSwapchainKHR(s_vkState.device, &swapchainCreateInfo, nullptr, &s_vkState.swapchain);
    CP_ASSERT(result == VK_SUCCESS);

    if (oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(s_vkState.device, oldSwapchain, nullptr);
    }

    result = vkGetSwapchainImagesKHR(s_vkState.device, s_vkState.swapchain, &s_vkState.swapchainImgCount, NULL);
    VkImage* pSwapchainImages = (VkImage*)malloc(s_vkState.swapchainImgCount * sizeof(VkImage));

    result = vkGetSwapchainImagesKHR(s_vkState.device, s_vkState.swapchain, &s_vkState.swapchainImgCount, pSwapchainImages);
    s_vkState.pSwapchainBuffers = (SwapchainBuffers *)malloc(s_vkState.swapchainImgCount * sizeof(SwapchainBuffers));

    for (UINT i = 0; i < s_vkState.swapchainImgCount; i++)
    {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = NULL;
        colorAttachmentView.format = VK_FORMAT_B8G8R8A8_UNORM;
        colorAttachmentView.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        colorAttachmentView.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.flags = 0;

        s_vkState.pSwapchainBuffers[i].image = pSwapchainImages[i];

        colorAttachmentView.image = s_vkState.pSwapchainBuffers[i].image;

        result = vkCreateImageView(s_vkState.device, &colorAttachmentView, NULL, &s_vkState.pSwapchainBuffers[i].view);
    }

    s_vkState.currSwapchainBuffer = 0;

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo rendePassCreateInfo = {};
    rendePassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rendePassCreateInfo.pNext = nullptr;
    rendePassCreateInfo.attachmentCount = 1;
    rendePassCreateInfo.pAttachments = &attachmentDescription;
    rendePassCreateInfo.subpassCount = 1;
    rendePassCreateInfo.pSubpasses = &subpass;
    rendePassCreateInfo.dependencyCount = 0;
    rendePassCreateInfo.pDependencies = nullptr;
    result = vkCreateRenderPass(s_vkState.device, &rendePassCreateInfo, nullptr, &s_vkState.renderPass);
    CP_ASSERT(result == VK_SUCCESS);

    VkImageView attachment = {};

    VkFramebufferCreateInfo fbInfo = {};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext = NULL;
    fbInfo.renderPass = s_vkState.renderPass;
    fbInfo.attachmentCount = 1;
    fbInfo.pAttachments = &attachment;
    fbInfo.width = s_vkState.width;
    fbInfo.height = s_vkState.height;
    fbInfo.layers = 1;

    s_vkState.pFrameBuffers = (VkFramebuffer*)malloc(s_vkState.swapchainImgCount * sizeof(VkFramebuffer));

    for (UINT i = 0; i < s_vkState.swapchainImgCount; i++)
    {
        attachment = s_vkState.pSwapchainBuffers[i].view;
        result = vkCreateFramebuffer(s_vkState.device, &fbInfo, NULL, &s_vkState.pFrameBuffers[i]);
        CP_ASSERT(result == VK_SUCCESS);
    }

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext = NULL;
    cmdPoolInfo.queueFamilyIndex = graphicsQueueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(s_vkState.device, &cmdPoolInfo, NULL, &s_vkState.cmdPool);
    CP_ASSERT(result == VK_SUCCESS);

    VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.pNext = NULL;
    cmdBufAllocInfo.commandPool = s_vkState.cmdPool;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocInfo.commandBufferCount = 1;
    result = vkAllocateCommandBuffers(s_vkState.device, &cmdBufAllocInfo, &s_vkState.cmdBuf);
    CP_ASSERT(result == VK_SUCCESS);

    return result == VK_SUCCESS;
}

/// Fill in a command buffer that clears swapchain, and submit it.
void VulkanPlayer::ClearSwapchainImage()
{
    VkResult result = VK_INCOMPLETE;

    VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
    cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cmdBufInheritInfo.pNext = NULL;
    cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
    cmdBufInheritInfo.subpass = 0;
    cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
    cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
    cmdBufInheritInfo.queryFlags = 0;
    cmdBufInheritInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = NULL;
    cmdBufInfo.flags = 0;
    cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;

    VkRenderPassBeginInfo rpBeginInfo = {};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.pNext = NULL;
    rpBeginInfo.renderPass = s_vkState.renderPass;
    rpBeginInfo.framebuffer = s_vkState.pFrameBuffers[s_vkState.currSwapchainBuffer];
    rpBeginInfo.renderArea.offset.x = 0;
    rpBeginInfo.renderArea.offset.y = 0;
    rpBeginInfo.renderArea.extent.width = s_vkState.width;
    rpBeginInfo.renderArea.extent.height = s_vkState.height;
    rpBeginInfo.clearValueCount = 0;
    rpBeginInfo.pClearValues = nullptr;

    result = vkBeginCommandBuffer(s_vkState.cmdBuf, &cmdBufInfo);
    CP_ASSERT(result == VK_SUCCESS);

    VkImageSubresourceRange subResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkImageMemoryBarrier imgMemBarrier = {};
    imgMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgMemBarrier.pNext = NULL;
    imgMemBarrier.srcAccessMask = 0;
    imgMemBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgMemBarrier.image = s_vkState.pSwapchainBuffers[s_vkState.currSwapchainBuffer].image;
    imgMemBarrier.subresourceRange = subResRange;

    vkCmdPipelineBarrier(s_vkState.cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &imgMemBarrier);
    vkCmdBeginRenderPass(s_vkState.cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    vkCmdClearColorImage(s_vkState.cmdBuf, s_vkState.pSwapchainBuffers[s_vkState.currSwapchainBuffer].image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subResRange);

    vkCmdEndRenderPass(s_vkState.cmdBuf);

    VkImageMemoryBarrier prePresentBarrier = {};
    prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    prePresentBarrier.pNext = NULL;
    prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    prePresentBarrier.image = s_vkState.pSwapchainBuffers[s_vkState.currSwapchainBuffer].image;

    vkCmdPipelineBarrier(s_vkState.cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &prePresentBarrier);

    result = vkEndCommandBuffer(s_vkState.cmdBuf);
    CP_ASSERT(result == VK_SUCCESS);

    VkPipelineStageFlags pipeStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = &pipeStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &s_vkState.cmdBuf;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;

    result = vkQueueSubmit(s_vkState.queue, 1, &submitInfo, VK_NULL_HANDLE);
    CP_ASSERT(result == VK_SUCCESS);

    result = vkQueueWaitIdle(s_vkState.queue);
    CP_ASSERT(result == VK_SUCCESS);
}

/// Implement the render loop.
void VulkanPlayer::RenderLoop()
{
    s_vkState.width = m_pPlayerWindow->GetWindowWidth();
    s_vkState.height = m_pPlayerWindow->GetWindowHeight();
    s_vkState.hInstance = m_pPlayerWindow->GetInstance();
    s_vkState.hWnd = m_pPlayerWindow->GetWindowHandle();

    while (m_pPlayerWindow->Update())
    {
        VulkanPlayer::Present();
    }
}

/// Perform any cleanup here
void VulkanPlayer::Destroy()
{
    vkDestroyRenderPass(s_vkState.device, s_vkState.renderPass, nullptr);
    vkFreeCommandBuffers(s_vkState.device, s_vkState.cmdPool, 1, &s_vkState.cmdBuf);

    for (UINT i = 0; i < s_vkState.swapchainImgCount; i++)
    {
        vkDestroyFramebuffer(s_vkState.device, s_vkState.pFrameBuffers[i], nullptr);
        vkDestroyImageView(s_vkState.device, s_vkState.pSwapchainBuffers[i].view, nullptr);
    }

    vkDestroySwapchainKHR(s_vkState.device, s_vkState.swapchain, nullptr);
    vkDestroyDevice(s_vkState.device, nullptr);
    vkDestroySurfaceKHR(s_vkState.inst, s_vkState.surface, nullptr);
    vkDestroyInstance(s_vkState.inst, nullptr);

    free(s_vkState.pQueueProps);

    delete m_pPlayerWindow;
    delete s_vulkanSurface;
}
