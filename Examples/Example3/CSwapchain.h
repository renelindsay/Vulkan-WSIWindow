/*
// Copyright (C) 2017 by Rene Lindsay
*
*  This unit wraps the swapchain.
*
*  WARNING: This unit is a work in progress.
*  Interfaces are experimental and very likely to change.
*/

#ifndef CSWAPCHAIN_H
#define CSWAPCHAIN_H

#include "CDevices.h"

#define IS_ANDROID false // PC: default to low-latency (no fps limit)
#ifdef ANDROID
#define IS_ANDROID true  // ANDROID: default to power-save (limit to 60fps)
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
    VkRenderPass       renderpass;
    VkCommandPool      command_pool;

    std::vector<CSwapchainBuffer> buffers;
    uint32_t acquired_index;  // index of last acquired image
    bool is_acquired;

    VkSemaphore acquire_semaphore;
    VkSemaphore submit_semaphore;

    void Init(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface);
    void CreateCommandPool(uint32_t family);
    void CreateCommandBuffers();
    void Apply();

public:
    VkSurfaceCapabilitiesKHR surface_caps;
    VkSwapchainCreateInfoKHR info;

    CSwapchain(const CQueue& present_queue);
    ~CSwapchain();

    bool PresentMode(bool no_tearing, bool powersave = IS_ANDROID);  // ANDROID: default to power-save mode (limit to 60fps)
    bool PresentMode(VkPresentModeKHR preferred_mode);               // If mode is not available, returns false and uses FIFO.

    void SetFormat(VkFormat preferred_format = VK_FORMAT_B8G8R8A8_UNORM);
    void SetExtent(uint32_t width, uint32_t height);
    bool SetImageCount(uint32_t image_count = 2);                    // 2=doublebuffer 3=tripplebuffer
    void SetRenderPass(VkRenderPass renderpass);

    VkExtent2D GetExtent(){return info.imageExtent;}
    void Print();    

    CSwapchainBuffer& AcquireNext();
    void Present();

};

#endif


