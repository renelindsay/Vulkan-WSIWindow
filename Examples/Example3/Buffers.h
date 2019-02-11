#ifndef BUFFERS_H
#define BUFFERS_H

#include "WSIWindow.h"

class CDepthBuffer {
    VkPhysicalDevice gpu;
    VkDevice device;
public:
    VkFormat       format;
    VkImage        Image;
    VkDeviceMemory ImageMemory;
    VkImageView    ImageView;

    CDepthBuffer();
    virtual ~CDepthBuffer();
    void Create(VkPhysicalDevice gpu, VkDevice device, VkExtent2D extent, VkFormat format = VK_FORMAT_D32_SFLOAT);
    void Destroy();
    void Resize(VkExtent2D extent);
};

#endif