// * Copyright (C) 2019 by Rene Lindsay

/*
*  CSwapchain uses CDepthBuffer to allocate a depth buffer for the swapchain framebuffer.
*  CSwapchain will automatically resize the depth buffer if the window gets resized.
*
*  CAllocator wraps the "Vulkan Memory Allocator" library.
*  Pass a CAllocator instance to the contructor of buffer and image contructors.
*
*  CBuffer is the base class for CVBO, CIBO and CUBO classes.
*  Buffer types:
*      CVBO : Vertex buffer object   : Array of structs (interleaved data)
*      CIBO : Index buffer object    : Array of type uint16_t or uint32_t
*      CUBO : Uniform buffer object  : A single struct instance
*
*  Image types:
*      (WIP)
*
*  NOTE: "Vulkan Memory Allocator" fails to compile on VS2013. 
*        Therefore, minimum supported compiler is now VS2015.
*
*/

#ifndef BUFFERS_H
#define BUFFERS_H

#include "WSIWindow.h"
#include "CDevices.h"
#include "vk_mem_alloc.h"

//----------------------------------Depth Buffer----------------------------------
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
//--------------------------------------------------------------------------------
//------------------------------------Allocator-----------------------------------

class CAllocator {
    VmaAllocator     allocator;
    VkPhysicalDevice gpu;
    VkDevice         device;
    VkQueue          queue;
    VkCommandPool    command_pool;
    VkCommandBuffer  command_buffer;
    void BeginCmd();
    void EndCmd();
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    friend class CvkBuffer;
    friend class CvkImage;

    void CreateBuffer(const void* data, uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage memtype, VkBuffer& buffer, VmaAllocation& alloc, void** mapped = 0);
    void DestroyBuffer(VkBuffer buffer, VmaAllocation alloc);
    
    void CreateImage(const void* data, VkExtent3D extent, VkFormat format, bool mipmap, VkImage& image, VmaAllocation& alloc, VkImageView& view);
    void DestroyImage(VkImage image, VkImageView view, VmaAllocation alloc);

public:
    CAllocator(const CQueue& queue, VkDeviceSize blockSize=256);
    virtual ~CAllocator();
};
//--------------------------------------------------------------------------------
//-------------------------------------Buffers------------------------------------
class CvkBuffer {
    CAllocator*   allocator;
    VmaAllocation allocation;
    VkBuffer      buffer;
    uint32_t      count;
protected:
    uint32_t      stride;
public:
    void* mapped = nullptr;

    CvkBuffer(CAllocator& allocator);
    virtual ~CvkBuffer();
    void Clear();
    void Data(const void* data, uint32_t count, uint32_t stride, VkBufferUsageFlagBits usage, VmaMemoryUsage memtype=VMA_MEMORY_USAGE_GPU_ONLY, void** mapped = nullptr);
    uint32_t Count() { return count; }
    operator VkBuffer () {return buffer;}
};

class CVBO : public CvkBuffer {
public:
    using CvkBuffer::CvkBuffer;
    void Data(void* data, uint32_t count, uint32_t stride);
};

class CIBO : public CvkBuffer {
public:
    using CvkBuffer::CvkBuffer;
    void Data(const uint16_t* data, uint32_t count);
    void Data(const uint32_t* data, uint32_t count);
};

class CUBO : public CvkBuffer {
public:
    using CvkBuffer::CvkBuffer;
    //void Data(void* data, uint32_t size);
    void Allocate(uint32_t size);
    void Update(void* data);

    uint32_t size(){ return stride; }
};
//--------------------------------------------------------------------------------
//-------------------------------------Images-------------------------------------

class CvkImage {
    CAllocator*   allocator;
    VmaAllocation allocation;
    VkImage       image;
    VkExtent2D    extent;
    VkFormat      format;
public:
    VkImageView   view;
    VkSampler     sampler;

    CvkImage(CAllocator& allocator);
    virtual ~CvkImage();
    void Clear();
    void Data(const void* data, VkExtent3D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipmap = false);

    void CreateSampler();

    //operator VkImage () {return image;}
    //operator VkImageView () {return view;}
    //operator VkSampler   () {return sampler;}
};

//--------------------------------------------------------------------------------


#endif