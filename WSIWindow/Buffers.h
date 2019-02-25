// * Copyright (C) 2019 by Rene Lindsay

/*
*  CSwapchain uses this class to allocate a depth buffer for the swapchain framebuffer.
*  CSwapchain will automatically resize the depth buffer if the window gets resized.
*
*  CAllocator wraps the "Vulkan Memory Allocator" library.
*  Pass a CAllocator instance to the contructor of buffer and image contructors.
*  Buffer types:
*      CVBO : Vertex buffer object   : Array of structs (interleaved data)
*      CIBO : Index buffer object    : Array of type uint16_t or uint32_t
*      CUBO : Uniform buffer object  : A single struct instance
*
*  Image types:
*      (WIP)
*
*  NOTE: "Vulkan Memory Allocator" fails to compile on VS2013. 
*        Minimum supported compiler is now VS2015.
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

    friend class CBuffer;
    friend class CImage;

    void CreateBuffer(const void* data, uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage memtype, VkBuffer& buffer, VmaAllocation& alloc);
    void DestroyBuffer(VkBuffer buffer, VmaAllocation alloc);
    
    //--UNTESTED--
    //void CreateImage(const void* data, VkExtent3D extent, VkImageUsageFlags usage, VmaMemoryUsage memtype, VkImage& image, VmaAllocation& alloc, VkImageView& imageView);
    void CreateImage(const void* data, VkExtent3D extent, VkImage& image, VmaAllocation& alloc, VkImageView& imageView);
    void DestroyImage(VkImage image, VmaAllocation alloc);
    //------------
public:
    CAllocator(const CQueue& queue, VkDeviceSize blockSize=256);
    virtual ~CAllocator();
};
//--------------------------------------------------------------------------------
//-------------------------------------Buffers------------------------------------
class CBuffer {
    CAllocator*   allocator;
    VmaAllocation allocation;
    VkBuffer      buffer;
    uint32_t      stride;
    uint32_t      count;
public:
    CBuffer(CAllocator& allocator);
    virtual ~CBuffer();
    void Clear();
    void Data(const void* data, uint32_t count, uint32_t stride, VkBufferUsageFlagBits usage, VmaMemoryUsage memtype=VMA_MEMORY_USAGE_GPU_ONLY);
    uint32_t Count() { return count; }
    operator VkBuffer () {return buffer;}
};

class CVBO : public CBuffer {
public:
    using CBuffer::CBuffer;
    void Data(void* data, uint32_t count, uint32_t stride);
};

class CIBO : public CBuffer {
public:
    using CBuffer::CBuffer;
    void Data(const uint16_t* data, uint32_t count);
    void Data(const uint32_t* data, uint32_t count);
};

class CUBO : public CBuffer {
public:
    using CBuffer::CBuffer;
    void Data(void* data, uint32_t size);
};
//--------------------------------------------------------------------------------
//-------------------------------------Images-------------------------------------
/*
class CImage {
    CAllocator*   allocator;
    VmaAllocation allocation;
    VkImage       image;
    VkImageView   view;
    VkExtent2D    extent;
    VkFormat      format;
public:
    CImage(CAllocator& allocator);
    virtual ~CImage();

    operator VkImage () {return image;}
};
*/
//--------------------------------------------------------------------------------


#endif