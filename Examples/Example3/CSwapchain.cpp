// * Copyright (C) 2017 by Rene Lindsay

#include "CSwapchain.h"

CSwapchain::CSwapchain(const CQueue& present_queue, CRenderpass& renderpass) {
    const CQueue& q = present_queue;
    prenderpass = &renderpass;
    if(!q.surface){ LOGE("This queue may not be presentable. (No surface attached.)"); }
    Init(q.gpu, q.device, q.surface);
    queue = q.handle;
    CreateCommandPool(q.family);

    // -- Create Semaphores --
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VKERRCHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &acquire_semaphore));
    VKERRCHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &submit_semaphore));
    // -----------------------

    depth_buffer.Create(gpu, device, info.imageExtent, prenderpass->depth_format);
    Apply();
}

CSwapchain::~CSwapchain(){
    if (device) vkDeviceWaitIdle(device);
    if (command_pool)      vkDestroyCommandPool(device, command_pool,      nullptr);
    if (acquire_semaphore) vkDestroySemaphore  (device, acquire_semaphore, nullptr);
    if (submit_semaphore)  vkDestroySemaphore  (device, submit_semaphore,  nullptr);

    if (swapchain) {
        for(auto& buf : buffers) {
            vkDestroyFence(device, buf.fence, nullptr);
            vkDestroyFramebuffer(device, buf.framebuffer, nullptr);
            vkDestroyImageView(device, buf.view, nullptr);
        }
        vkDestroySwapchainKHR(device, swapchain, 0);
        LOGI("Swapchain destroyed\n");
    }
}

void CSwapchain::Init(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface) {
    this->gpu     = gpu;
    this->surface = surface;
    this->device  = device;
    swapchain     = 0;
    //renderpass    = 0;
    is_acquired   = false;

    //--- surface caps ---
    VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
    assert(surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    //assert(surface_caps.supportedTransforms & surface_caps.currentTransform);
    assert(surface_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    assert(surface_caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    //--------------------

    info = {};
    info.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface               = surface;
    //info.minImageCount         = 2; // double-buffer
    //info.imageFormat           = format.format;
    //info.imageColorSpace       = format.colorSpace;

    info.imageFormat           = prenderpass->surface_format;
    info.imageColorSpace       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    //info.imageExtent           = {64, 64}; //extent;
    info.imageArrayLayers      = 1;  // 2 for stereo
    info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    //info.compositeAlpha        = composite_alpha;
    info.presentMode           = VK_PRESENT_MODE_FIFO_KHR;
    info.clipped               = true;
    //info.oldSwapchain          = swapchain;
    info.compositeAlpha = (surface_caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
                           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SetExtent();  // also initializes surface_caps
    SetFormat(VK_FORMAT_B8G8R8A8_UNORM);
    SetImageCount(2);
    
    //renderpass.Init(device, info.imageFormat, GetSupportedDepthFormat(gpu));
    //Apply();
}

//----------------------------------CommandPool------------------------------------
void CSwapchain::CreateCommandPool(uint32_t family) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = family;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VKERRCHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &command_pool));
}
//---------------------------------------------------------------------------------

int clamp(int val, int min, int max){ return (val < min ? min : val > max ? max : val); }

//void CSwapchain::SetExtent(uint32_t width, uint32_t height) { //provide width,height, in case its not available from surface
void CSwapchain::SetExtent() {
    VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
    VkExtent2D& curr = surface_caps.currentExtent;
    VkExtent2D& ext = info.imageExtent;

    //printf("swapchain: w=%d h=%d curr_w=%d curr_h=%d\n", width, height, curr.width, curr.height);

    if (curr.width == 0xFFFFFFFF) {  // 0xFFFFFFFF indicates surface size is set from extent
        const int default_width  = 256;
        const int default_height = 256;
        LOGW("Can't determine current window surface extent from surface caps. Using defaults instead. (%d x %d)\n", default_width, default_height);
        ext.width  = clamp(default_width,  surface_caps.minImageExtent.width,  surface_caps.maxImageExtent.width);
        ext.height = clamp(default_height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
    } else ext = curr;              // else, set extent from surface size
    if (!!swapchain) Apply();
}

void CSwapchain::SetFormat(VkFormat preferred_format){  // if preferred is not available, default to first available format
    //---Get Surface format list---
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, formats.data());
    //-----------------------------

    VkFormat& format = info.imageFormat;
    for (auto& f : formats) if (!format || f.format == preferred_format) format = f.format;
    if (!format) format = preferred_format;
    if (format != preferred_format) LOGW("Surface format %d not available. Using format %d instead.\n", preferred_format, format);
    if (!!swapchain) Apply();
}

bool CSwapchain::SetImageCount(uint32_t image_count){  // set number of framebuffers. (2 or 3)
    uint32_t count = max(image_count, surface_caps.minImageCount);                      //clamp to min limit
    if(surface_caps.maxImageCount > 0) count = min(count, surface_caps.maxImageCount);  //clamp to max limit
    info.minImageCount = count;
    if(count != image_count) LOGW("Swapchain using %d framebuffers, instead of %d.\n", count, image_count);
    if(!!swapchain) Apply();
    return (count == image_count);
}

// Returns the list of avialable present modes for this gpu + surface.
std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice gpu, VkSurfaceKHR surface){
    uint32_t count = 0;
    std::vector<VkPresentModeKHR> modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, nullptr);
    assert(count > 0);
    modes.resize(count);
    VKERRCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, modes.data()));
    return modes;
}

// ---------------------------- Present Mode ----------------------------
// no_tearing : TRUE = Wait for next vsync, to swap buffers.  FALSE = faster fps.
// powersave  : TRUE = Limit framerate to vsync (60 fps).     FALSE = lower latency.
bool CSwapchain::PresentMode(bool no_tearing, bool powersave){
    return PresentMode(VkPresentModeKHR ((no_tearing ? 1 : 0) ^ (powersave ? 3 : 0)));  // if not found, use FIFO
}

bool CSwapchain::PresentMode(VkPresentModeKHR pref_mode){
    VkPresentModeKHR& mode = info.presentMode;
    auto modes = GetPresentModes(gpu, surface);
    mode = VK_PRESENT_MODE_FIFO_KHR;                           // default to FIFO mode
    for (auto m : modes) if(m == pref_mode) mode = pref_mode;  // if prefered mode is available, select it.
    if (mode != pref_mode) LOGW("Requested present-mode is not supported. Reverting to FIFO mode.\n");
    if (!!swapchain) Apply();
    return (mode == pref_mode);
}
//-----------------------------------------------------------------------


const char* FormatStr(VkFormat fmt) {
#define STR(f) case f: return #f
    switch (fmt) {
        STR(VK_FORMAT_UNDEFINED);            //  0
        //Color
        STR(VK_FORMAT_R5G6B5_UNORM_PACK16);  //  4
        STR(VK_FORMAT_R8G8B8A8_UNORM);       // 37
        STR(VK_FORMAT_R8G8B8A8_SRGB);        // 43
        STR(VK_FORMAT_B8G8R8A8_UNORM);       // 44
        STR(VK_FORMAT_B8G8R8A8_SRGB);        // 50
        //Depth
        STR(VK_FORMAT_D32_SFLOAT);           //126
        STR(VK_FORMAT_D32_SFLOAT_S8_UINT);   //130
        STR(VK_FORMAT_D24_UNORM_S8_UINT);    //129
        STR(VK_FORMAT_D16_UNORM_S8_UINT);    //128
        STR(VK_FORMAT_D16_UNORM);            //124
        default: return "";
    }
#undef STR
}

void CSwapchain::Print() {
    printf("Swapchain:\n");
    printf("\tFormat  = %3d : %s\n", info.imageFormat,    FormatStr(info.imageFormat));
    printf("\tDepth   = %3d : %s\n", depth_buffer.format, FormatStr(depth_buffer.format));

    VkExtent2D& extent = info.imageExtent;
    printf("\tExtent  = %d x %d\n", extent.width, extent.height);
    printf("\tBuffers = %d\n", (int)buffers.size());

    auto modes = GetPresentModes(gpu, surface);
    printf("\tPresentMode:\n");
    const char* mode_names[] = {"VK_PRESENT_MODE_IMMEDIATE_KHR", "VK_PRESENT_MODE_MAILBOX_KHR",
                                "VK_PRESENT_MODE_FIFO_KHR", "VK_PRESENT_MODE_FIFO_RELAXED_KHR"};
    VkPresentModeKHR& mode = info.presentMode;
    for (auto m : modes) print((m == mode) ? eRESET : eFAINT, "\t\t%s %s\n", (m == mode) ? cTICK : " ",mode_names[m]);
}

void CSwapchain::Apply() {
    info.oldSwapchain = swapchain;
    VKERRCHECK(vkCreateSwapchainKHR(device, &info, nullptr, &swapchain));

    //-- Delete old swapchain --
    if (info.oldSwapchain) {
        vkDeviceWaitIdle(device);
        for(auto& buf : buffers) {
            vkDestroyFence(device, buf.fence, nullptr);
            vkDestroyFramebuffer(device, buf.framebuffer, nullptr);
            vkDestroyImageView(device, buf.view, nullptr);
        }
        vkDestroySwapchainKHR(device, info.oldSwapchain, 0);
    }
    //--------------------------

    //-- Allocate array of images for swapchain--
    std::vector<VkImage> images;
    uint32_t count = 0;
    VKERRCHECK(vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr));
    images.resize(count);
    VKERRCHECK(vkGetSwapchainImagesKHR(device, swapchain, &count, images.data()));
    //-------------------------------------------

    depth_buffer.Resize(info.imageExtent);  //resize depth buffer

    buffers.resize(count);
    repeat(count){
        auto& buf = buffers[i];
        buf.image = images[i];
        //---ImageView---
        VkImageViewCreateInfo ivCreateInfo = {};
        ivCreateInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivCreateInfo.pNext    = NULL;
        ivCreateInfo.flags    = 0;
        ivCreateInfo.image    = images[i];
        ivCreateInfo.format   = info.imageFormat;
        ivCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivCreateInfo.components = {};
        ivCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        ivCreateInfo.subresourceRange.baseMipLevel   = 0;
        ivCreateInfo.subresourceRange.levelCount     = 1;
        ivCreateInfo.subresourceRange.baseArrayLayer = 0;
        ivCreateInfo.subresourceRange.layerCount     = 1;
        VKERRCHECK(vkCreateImageView(device, &ivCreateInfo, nullptr, &buf.view));
        //---------------

        // -- View list --
        std::vector<VkImageView> views;                                       // List of views
        views.push_back(buf.view);                                            // Add color buffer (unique)
        if(depth_buffer.ImageView) views.push_back(depth_buffer.ImageView);   // Add depth buffer (shared)

        //--Framebuffer--
        VkFramebufferCreateInfo fbCreateInfo = {};
        fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCreateInfo.renderPass      = *prenderpass;
        fbCreateInfo.attachmentCount = (uint32_t)views.size();  // 1/2
        fbCreateInfo.pAttachments    =           views.data();  // views for color and depth buffer
        fbCreateInfo.width  = info.imageExtent.width;
        fbCreateInfo.height = info.imageExtent.height;
        fbCreateInfo.layers = 1;
        VKERRCHECK(vkCreateFramebuffer(device, &fbCreateInfo, NULL, &buf.framebuffer));
        //---------------
        //--CommandBuffer--
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = command_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VKERRCHECK(vkAllocateCommandBuffers(device, &allocInfo, &buf.command_buffer));
        //-----------------
        //---Fence---
        VkFenceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(device, &createInfo, nullptr, &buf.fence);
        //-----------

        //printf("---Extent = %d x %d\n", info.imageExtent.width, info.imageExtent.height);
    }
    if (!info.oldSwapchain) LOGI("Swapchain created\n");
}
//---------------------------------------------------------------------------------

CSwapchainBuffer& CSwapchain::AcquireNext() {
    ASSERT(!is_acquired, "CSwapchain: Previous swapchain buffer has not yet been presented.\n");
    //ASSERT(!!renderpass, "CSwapchain: renderpass was not set.\n");

    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &acquired_index);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) SetExtent();  // window resize

    CSwapchainBuffer& buf = buffers[acquired_index];
    buf.extent = info.imageExtent;
    vkWaitForFences(device, 1, &buf.fence, VK_TRUE, UINT64_MAX);
    is_acquired = true;
    return buf;
}

void CSwapchain::Present() {
    ASSERT(!!is_acquired, "CSwapchain: A buffer must be acquired before presenting.\n");
    // --- Submit ---
    CSwapchainBuffer& buffer = buffers[acquired_index];
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &acquire_semaphore;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &buffer.command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &submit_semaphore;
    vkResetFences(device, 1, &buffer.fence);
    VKERRCHECK(vkQueueSubmit(queue, 1, &submitInfo, buffer.fence));
    // --- Present ---
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &submit_semaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchain;
    presentInfo.pImageIndices      = &acquired_index;
    //VKERRCHECK(vkQueuePresentKHR(queue, &presentInfo));

    VkResult result = vkQueuePresentKHR(queue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) SetExtent();  // window resize
    else ShowVkResult(result);

    is_acquired = false;
}
