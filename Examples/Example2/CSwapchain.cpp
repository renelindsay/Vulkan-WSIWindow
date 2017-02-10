#include "CSwapchain.h"

//CSwapchain::CSwapchain(CDevice device){
//}

int clamp(int val, int min, int max){ return (val < min ? min : val > max ? max : val); }


CSwapchain::CSwapchain(CDevice* device, VkSurfaceKHR surface, uint32_t image_count){
    Init(device, surface, image_count);
}

void CSwapchain::Init(CDevice* device, VkSurfaceKHR surface, uint32_t image_count){
    this->device = device;
    this->surface= surface;

    // ---Surface format---
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, surface, &count, formats.data());
    format = formats[0];                                                                 // Just pick the default format.
    if (format.format == VK_FORMAT_UNDEFINED) format.format = VK_FORMAT_B8G8R8A8_UNORM;  // If its UNDEFINED, try BGRA instead.
    //---------------------

    swapchain   = nullptr;
    extent      = {0, 0};

    //---- Image Count and back_buffers ----
    VkSurfaceCapabilitiesKHR caps;
    VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, surface, &caps));
    image_count = clamp(image_count, caps.minImageCount, caps.maxImageCount);
    CreateBackBuffers(image_count);
    //--------------------------------------
    //--- caps checks ---
    assert(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    assert(caps.supportedTransforms & caps.currentTransform);
    assert(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    //-------------------
    Resize(0, 0);

    LOGI("Creating swapchain with surface format: %d\n", (int)format.format);
}

CSwapchain::~CSwapchain(){
    DestroyBackBuffers();
}

void CSwapchain::Print(){
    printf("Swapchain:");
    printf("\t Surface format=%d\n", (int)format.format);
    printf("\t Extent=%d x %d\n", extent.width, extent.height);
    printf("\t Buffers=%d\n", (int)back_buffers.size());
    printf("\t vsync=%s\n", vsync ? "True" : "False");
}



void CSwapchain::Resize(uint width, uint height){
    //--- Surface extent ---  TODO: Move this to CSurface?
    VkSurfaceCapabilitiesKHR caps;
    VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, surface, &caps));
    VkExtent2D& curr = caps.currentExtent;
    printf("swapchain: w=%d h=%d curr_w=%d curr_h=%d\n", width, height, curr.width, curr.height);
    if(curr.width > 0 && curr.width > 0) extent = curr;
    else {
        extent.width  = clamp(width,  caps.minImageExtent.width,  caps.maxImageExtent.width);
        extent.height = clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    //----------------------
    //--- composite alpha ---
    VkCompositeAlphaFlagBitsKHR composite_alpha =
        (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //-----------------------

    //--- Present Modes ---  //TODO: Move out
    uint32_t mcount = 0;
    std::vector<VkPresentModeKHR> modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, surface, &mcount, NULL);
    modes.resize(mcount);
    VKERRCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(*device, surface, &mcount, modes.data()));
    assert(mcount > 0);
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;  // FIFO is the only mode universally supported
#ifdef ANDROID
    VkPresentModeKHR pref_mode = vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_FIFO_RELAXED_KHR;  // limit FPS to save battery
#else
    VkPresentModeKHR pref_mode = vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
#endif
    for (auto m : modes) { if(m == pref_mode) mode = pref_mode; break; }
    //---------------------

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface          = surface;
    swapchain_info.minImageCount    = back_buffers.size();
    swapchain_info.imageFormat      = format.format;
    swapchain_info.imageColorSpace  = format.colorSpace;
    swapchain_info.imageExtent      = extent;
    swapchain_info.imageArrayLayers = 1;  // 2 for stereo
    swapchain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // TODO: Multiple present queues
    //swapchain_info.queueFamilyIndexCount = (uint32_t)queue_families.size();
    //swapchain_info.pQueueFamilyIndices = queue_families.data();

    swapchain_info.preTransform    = caps.currentTransform;;
    swapchain_info.compositeAlpha  = composite_alpha;
    swapchain_info.presentMode     = mode;
    swapchain_info.clipped         = true;
    swapchain_info.oldSwapchain    = swapchain;

    VKERRCHECK(vkCreateSwapchainKHR(*device, &swapchain_info, nullptr, &swapchain));
}

void CSwapchain::CreateBackBuffers(const int count){
    VkSemaphoreCreateInfo sem_info = {};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < count; i++) {
        BackBuffer buf = {};
        VKERRCHECK(vkCreateSemaphore(*device, &sem_info,   nullptr, &buf.acquire_semaphore));
        VKERRCHECK(vkCreateSemaphore(*device, &sem_info,   nullptr, &buf.render_semaphore));
        VKERRCHECK(vkCreateFence    (*device, &fence_info, nullptr, &buf.present_fence));
        back_buffers.push_back(buf);
    }
}

void CSwapchain::DestroyBackBuffers(){
    while (!back_buffers.empty()) {
        const auto &buf = back_buffers.back();
        vkDestroySemaphore(*device, buf.acquire_semaphore, nullptr);
        vkDestroySemaphore(*device, buf.render_semaphore, nullptr);
        vkDestroyFence    (*device, buf.present_fence, nullptr);
        back_buffers.pop_back();
    }
}

