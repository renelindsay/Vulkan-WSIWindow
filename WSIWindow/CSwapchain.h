/*
// Copyright (C) 2019 by Rene Lindsay
*
*  This unit wraps the swapchain.
*
*  WARNING: This unit is a work in progress.
*  Interfaces are experimental and likely to change.
*
*  The CSwapchain constructor requires a CQueue and CRenderpass as parameters, so create these first.
*  The CQueue must be presentable, and linked to the window surface.
*  The CRenderpass requires at least one color attachment, and optionally a depth attachment.
*
*  Use the PresentMode() function to select vsync behaviour (FIFO / MAILBOX / ...)
*  Use the SetImageCount() to select double or tripple buffering. (default is 2: double-buffering)
*
*  PRESENTING:
*  Call BeginFrame() to acquire the next frame's command buffer.
*  Record vkCmd* commands, using the returned command buffer.
*  Call EndFrame() to execure and Present image, when done.
*
*/

#ifndef CSWAPCHAIN_H
#define CSWAPCHAIN_H

#include "WSIWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"
#include "Buffers.h"

#ifdef ANDROID
#define IS_ANDROID true  // ANDROID: default to power-save (limit to 60fps)
#else
#define IS_ANDROID false // PC: default to low-latency (no fps limit)
#endif

struct CSwapchainBuffer {
    VkImage         image;
    VkImageView     view;  // TODO: MRT?
    VkExtent2D      extent;
    VkFramebuffer   framebuffer;
    VkCommandBuffer command_buffer;
    VkFence         fence;
};
/*
struct CCmd : public CSwapchainBuffer {
    void BindPipeline(VkPipeline graphicsPipeline);
    void Draw(uint32_t vertexCount, uint32_t instanceCount=1, uint32_t firstVertex=0, uint32_t  firstInstance=0);
};
*/
class CSwapchain {
    VkPhysicalDevice   gpu;
    VkDevice           device;
    VkQueue            graphics_queue;
    VkQueue            present_queue;
    VkSurfaceKHR       surface;
    VkSwapchainKHR     swapchain;
    VkCommandPool      command_pool;
    //VkRenderPass       renderpass;
    CRenderpass*       renderpass;

    CDepthBuffer depth_buffer;
    std::vector<CSwapchainBuffer> buffers;
    uint32_t acquired_index;  // index of last acquired image
    bool is_acquired;

    VkSemaphore acquire_semaphore;
    VkSemaphore submit_semaphore;

    void Init(const CQueue* present_queue, const CQueue* graphics_queue=0);

    //void CreateCommandPool(uint32_t family);
    void SetExtent();  //resize FrameBuffer image to match window surface
    //void SetFormat(VkFormat preferred_format = VK_FORMAT_B8G8R8A8_UNORM);
    void Apply();
    CSwapchainBuffer& AcquireNext();
    void Present();
public:
    VkSurfaceCapabilitiesKHR surface_caps;
    VkSwapchainCreateInfoKHR info;

    CSwapchain(CRenderpass& renderpass, const CQueue* present_queue, const CQueue* graphics_queue);
    ~CSwapchain();

    bool PresentMode(bool no_tearing, bool powersave = IS_ANDROID);  // ANDROID: default to power-save mode (limit to 60fps)
    bool PresentMode(VkPresentModeKHR preferred_mode);               // If mode is not available, returns false and uses FIFO.
    bool SetImageCount(uint32_t image_count = 2);                    // 2=doublebuffer 3=tripplebuffer

    VkExtent2D GetExtent(){return info.imageExtent;}
    void Print();

    VkCommandBuffer BeginFrame();  // Get next cmd buffer and start recording commands
    void EndFrame();               // End the renderpass and present
};

#endif


