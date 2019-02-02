/*
// Copyright (C) 2017 by Rene Lindsay
*
*  This unit wraps the swapchain.
*
*  WARNING: This unit is a work in progress.
*  Interfaces are experimental and very likely to change.
*
*  The CSwapchain constructor requires a CQueue object as parameter,
*  which must be a presentable queue, linked to the window surface.
*
*  Use SetFormat() to change the color format, or just leave it on the default setting.
*
*  Use Swapchain.renderpass to add Color and depth attachments, and configure
*  and add subpasses and dependencies between subpasses.
*  eg.
*    swapchain.renderpass.AddColorAttachment();  // Add a color attachment. (optionally you may specify the VkFormat)
     swapchain.renderpass.AddDepthAttachment();  // Add a depth-stencil attachment. ()
     swapchain.renderpass.AddSubpass({0,1});     // Create one subpass, which uses attachments 1 and 2.
     swapchain.Apply();                          // Create these settings. (Do not make further changes to renderpass after this call.)
*
*  Use the PresentMode() function to select vsync behaviour (FIFO / MAILBOX / ...)
*  Use the SetImageCount() to select double or tripple buffering. (default is 2: double-buffering)
*
*  PRESENTING:
*  Call AcquireNext() to get the next CSwapchainBuffer struct.
*  Record commands into its command_buffer member variable,
*  and call Present() when done.
*
*
*/



#ifndef CSWAPCHAIN_H
#define CSWAPCHAIN_H

#include "WSIWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"

#ifdef ANDROID
#define IS_ANDROID true  // ANDROID: default to power-save (limit to 60fps)
#else
#define IS_ANDROID false // PC: default to low-latency (no fps limit)
#endif

struct CSwapchainBuffer {
  VkImage         image;
  VkImageView     view;
  VkExtent2D      extent;
  VkFramebuffer   framebuffer;
  VkCommandBuffer command_buffer;
  VkFence         fence;
};

class CSwapchain {
    VkPhysicalDevice   gpu;
    VkDevice           device;
    VkQueue            queue;
    VkSurfaceKHR       surface;
    VkSwapchainKHR     swapchain;
    //VkRenderPass       renderpass;
    VkCommandPool      command_pool;

    std::vector<CSwapchainBuffer> buffers;
    uint32_t acquired_index;  // index of last acquired image
    bool is_acquired;

    VkSemaphore acquire_semaphore;
    VkSemaphore submit_semaphore;

    void Init(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface);
    void CreateCommandPool(uint32_t family);
    void CreateCommandBuffers();
    void SetExtent();  //resize FrameBuffer image to match window surface

public:
    VkSurfaceCapabilitiesKHR surface_caps;
    VkSwapchainCreateInfoKHR info;
    CRenderpass renderpass;

    CSwapchain(const CQueue& present_queue);
    ~CSwapchain();

    bool PresentMode(bool no_tearing, bool powersave = IS_ANDROID);  // ANDROID: default to power-save mode (limit to 60fps)
    bool PresentMode(VkPresentModeKHR preferred_mode);               // If mode is not available, returns false and uses FIFO.

    void SetFormat(VkFormat preferred_format = VK_FORMAT_B8G8R8A8_UNORM);
    //void SetExtent(uint32_t width=64, uint32_t height=64);
    bool SetImageCount(uint32_t image_count = 2);                    // 2=doublebuffer 3=tripplebuffer
    //void SetRenderPass(VkRenderPass renderpass);

    VkExtent2D GetExtent(){return info.imageExtent;}
    void Print();
    void Apply();

    CSwapchainBuffer& AcquireNext();
    void Present();
};

#endif


