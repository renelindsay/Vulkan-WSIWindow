#ifndef CSWAPCHAIN_H
#define CSWAPCHAIN_H

#include "CDevices.h"

struct BackBuffer {
//    uint32_t    image_index;
    VkSemaphore acquire_semaphore;
    VkSemaphore render_semaphore;
    VkFence     present_fence;  // signaled when this buffer is ready for reuse
};


class CSwapchain{
  public:
    CSwapchain(CDevice* device, VkSurfaceKHR surface, uint32_t image_count = 3);
    ~CSwapchain();

    CDevice*           device;
    VkSurfaceKHR       surface;      // TODO: Replace with CSurface, or move to CDevice?
    VkSurfaceFormatKHR format;       // TODO: Move this to CSurface?
    VkSwapchainKHR     swapchain;    //
    VkExtent2D         extent;       // TODO: Move this to CSurface?

    //uint32_t           image_count;  // defaults to 3 (tripple-buffering)
    std::vector<BackBuffer> back_buffers;
    BackBuffer     acquired_back_buffer;

    bool vsync = true;

    void Init(CDevice* device, VkSurfaceKHR surface, uint32_t image_count = 3);
    void Resize(uint width, uint height);

    void CreateBackBuffers(const int count);
    void DestroyBackBuffers();
    void Print();
};

#endif
