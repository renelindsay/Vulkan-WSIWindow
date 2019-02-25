#include "Buffers.h"

#define NOMINMAX
//#define VMA_RECORDING_ENABLED   0
//#define VMA_DEDICATED_ALLOCATION   0
//#define VMA_STATS_STRING_ENABLED   1
#define VMA_STATIC_VULKAN_FUNCTIONS 0

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

uint32_t findMemoryType(VkPhysicalDevice gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    LOGE("failed to find suitable memory type!");
    return 0;
}

void createImage(VkPhysicalDevice gpu, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;  //VK_FORMAT_D32_SFLOAT
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = tiling;  //VK_IMAGE_TILING_OPTIMAL
    imageInfo.usage = usage;    //VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VKERRCHECK(vkCreateImage(device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(gpu, memRequirements.memoryTypeBits, properties);
    VKERRCHECK(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));
    VKERRCHECK(vkBindImageMemory(device, image, imageMemory, 0));
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VKERRCHECK(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
    return imageView;
}

/*
void SetLayout(){
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.srcAccessMask       = 0;
    imageBarrier.dstAccessMask       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    imageBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.newLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    imageBarrier.srcQueueFamilyIndex = 0;
    imageBarrier.dstQueueFamilyIndex = 0;
    imageBarrier.image               = Image;
    imageBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageBarrier.subresourceRange.baseMipLevel   = 0;
    imageBarrier.subresourceRange.levelCount     = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount     = 1;
    vkCmdPipelineBarrier(command_buffer, 
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &imageBarrier);
}
*/

//------------------------------------------------------------------------------------------------

CDepthBuffer::CDepthBuffer() : gpu(0), device(0), format(), Image(0), ImageMemory(0), ImageView(0) {}
CDepthBuffer::~CDepthBuffer() { Destroy(); }

void CDepthBuffer::Create(VkPhysicalDevice gpu, VkDevice device, VkExtent2D extent, VkFormat format) {
    this->gpu    = gpu;
    this->device = device;
    this->format = format;
    Resize(extent);
}

void CDepthBuffer::Destroy(){
    if(ImageView)   vkDestroyImageView(device, ImageView, nullptr);
    if(Image)       vkDestroyImage(device, Image, nullptr);
    if(ImageMemory) vkFreeMemory(device, ImageMemory, nullptr);
}

void CDepthBuffer::Resize(VkExtent2D extent){
    Destroy();
    if(format == VK_FORMAT_UNDEFINED) return;
    createImage(gpu, device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image, ImageMemory);
    ImageView = createImageView(device, Image, format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

//---------------------------------------------------


//---------------------------------------------------


CAllocator::CAllocator(const CQueue& queue, VkDeviceSize blockSize) : allocator(0) {
    gpu            = queue.gpu;
    device         = queue.device;
    this->queue    = queue.handle;
    command_pool   = queue.CreateCommandPool();
    command_buffer = queue.CreateCommandBuffer(command_pool);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = gpu;
    allocatorInfo.device = device;
    allocatorInfo.preferredLargeHeapBlockSize = blockSize;

    VmaVulkanFunctions fn;
    fn.vkAllocateMemory                    = (PFN_vkAllocateMemory)vkAllocateMemory;
    fn.vkBindBufferMemory                  = (PFN_vkBindBufferMemory)vkBindBufferMemory;
    fn.vkBindImageMemory                   = (PFN_vkBindImageMemory)vkBindImageMemory;
    fn.vkCmdCopyBuffer                     = (PFN_vkCmdCopyBuffer)vkCmdCopyBuffer;
    fn.vkCreateBuffer                      = (PFN_vkCreateBuffer)vkCreateBuffer;
    fn.vkCreateImage                       = (PFN_vkCreateImage)vkCreateImage;
    fn.vkDestroyBuffer                     = (PFN_vkDestroyBuffer)vkDestroyBuffer;
    fn.vkDestroyImage                      = (PFN_vkDestroyImage)vkDestroyImage;
    fn.vkFlushMappedMemoryRanges           = (PFN_vkFlushMappedMemoryRanges)vkFlushMappedMemoryRanges;
    fn.vkFreeMemory                        = (PFN_vkFreeMemory)vkFreeMemory;
    fn.vkGetBufferMemoryRequirements       = (PFN_vkGetBufferMemoryRequirements)vkGetBufferMemoryRequirements;
    fn.vkGetImageMemoryRequirements        = (PFN_vkGetImageMemoryRequirements)vkGetImageMemoryRequirements;
    fn.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetPhysicalDeviceMemoryProperties;
    fn.vkGetPhysicalDeviceProperties       = (PFN_vkGetPhysicalDeviceProperties)vkGetPhysicalDeviceProperties;
    fn.vkInvalidateMappedMemoryRanges      = (PFN_vkInvalidateMappedMemoryRanges)vkInvalidateMappedMemoryRanges;
    fn.vkMapMemory                         = (PFN_vkMapMemory)vkMapMemory;
    fn.vkUnmapMemory                       = (PFN_vkUnmapMemory)vkUnmapMemory;
    fn.vkGetBufferMemoryRequirements2KHR   = 0;  //(PFN_vkGetBufferMemoryRequirements2KHR)vkGetBufferMemoryRequirements2KHR;
    fn.vkGetImageMemoryRequirements2KHR    = 0;  //(PFN_vkGetImageMemoryRequirements2KHR)vkGetImageMemoryRequirements2KHR;
    allocatorInfo.pVulkanFunctions = &fn;
    vmaCreateAllocator(&allocatorInfo, &allocator);
}

CAllocator::~CAllocator() {
    if(command_buffer)  vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    if(command_pool)    vkDestroyCommandPool(device, command_pool, nullptr);
    if(allocator)       vmaDestroyAllocator(allocator);
}

//--- Command Buffer ---
void CAllocator::BeginCmd() {
    VkCommandBufferBeginInfo cmdBufBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VKERRCHECK( vkBeginCommandBuffer(command_buffer, &cmdBufBeginInfo) );
}

void CAllocator::EndCmd() {
    VKERRCHECK(vkEndCommandBuffer(command_buffer));
    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;
    VKERRCHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VKERRCHECK(vkQueueWaitIdle(queue));
}
//-----------------------

void CAllocator::CreateBuffer(const void* data, uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage memtype, VkBuffer& buffer, VmaAllocation& alloc) {
    VkBufferCreateInfo bufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufInfo.size = size;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo allocInfo =  {};

    // For CPU-visible memory, skip staging buffer
    if(memtype != VMA_MEMORY_USAGE_GPU_ONLY) {
        bufInfo.usage = usage;
        VKERRCHECK(vmaCreateBuffer(allocator, &bufInfo, &allocCreateInfo, &buffer, &alloc, &allocInfo));
        memcpy(allocInfo.pMappedData, data, size);
        return;
    }

    // For GPU-only memory, copy via staging buffer.  // TODO: Also skip staging buffer on integrated gpus.
    VkBuffer      stageBuffer      = VK_NULL_HANDLE;
    VmaAllocation stageBufferAlloc = VK_NULL_HANDLE;

    VKERRCHECK(vmaCreateBuffer(allocator, &bufInfo, &allocCreateInfo, &stageBuffer, &stageBufferAlloc, &allocInfo));
    memcpy(allocInfo.pMappedData, data, size);

    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;  // | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    allocCreateInfo.usage = memtype;                           // VMA_MEMORY_USAGE_GPU_ONLY;
    allocCreateInfo.flags = 0;
    VKERRCHECK(vmaCreateBuffer(allocator, &bufInfo, &allocCreateInfo, &buffer, &alloc, nullptr));

    BeginCmd();
        VkBufferCopy bufCopyRegion = {};
        bufCopyRegion.srcOffset = 0;
        bufCopyRegion.dstOffset = 0;
        bufCopyRegion.size = bufInfo.size;
        vkCmdCopyBuffer(command_buffer, stageBuffer, buffer, 1, &bufCopyRegion);
    EndCmd();

    vmaDestroyBuffer(allocator, stageBuffer, stageBufferAlloc);
}

void CAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation alloc) {
    vmaDestroyBuffer(allocator, buffer, alloc);
}
//---------------------------------------------------

//void CAllocator::CreateImage(const void* data, VkExtent3D extent, VkImageUsageFlags usage, VmaMemoryUsage memtype, VkImage& image, VmaAllocation& alloc, VkImageView& imageView) {
void CAllocator::CreateImage(const void* data, VkExtent3D extent, VkImage& image, VmaAllocation& alloc, VkImageView& imageView) {
    // Copy image data to staging buffer in CPU memory
    uint64_t size = extent.width * extent.height * extent.depth * 4;

    VkBufferCreateInfo bufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufInfo.size = size;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo allocInfo =  {};
    VkBuffer      stageBuffer      = VK_NULL_HANDLE;
    VmaAllocation stageBufferAlloc = VK_NULL_HANDLE;
    VKERRCHECK(vmaCreateBuffer(allocator, &bufInfo, &allocCreateInfo, &stageBuffer, &stageBufferAlloc, &allocInfo));
    memcpy(allocInfo.pMappedData, data, size);

    // Create image in GPU memory
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.flags       = 0;
    imageInfo.imageType   = VK_IMAGE_TYPE_2D;
    imageInfo.format      = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent      = extent;
    imageInfo.mipLevels   = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage       = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //imageInfo.queueFamilyIndexCount = 0;
    //imageInfo.pQueueFamilyIndices = 0;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;  //memtype
    allocCreateInfo.flags = 0;
    vmaCreateImage(allocator, &imageInfo, &allocCreateInfo, &image, &alloc, nullptr);

    //  Copy image from staging buffer to image
    BeginCmd();
        TransitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region = {};
        region.bufferOffset      = 0;
        region.bufferRowLength   = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = extent;  // { width, height, 1};
        vkCmdCopyBufferToImage(command_buffer, stageBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    EndCmd();

    vmaDestroyBuffer(allocator, stageBuffer, stageBufferAlloc);

    // Create ImageView
    VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;
    imageViewInfo.subresourceRange.levelCount     = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount     = 1;
    VKERRCHECK( vkCreateImageView(device, &imageViewInfo, nullptr, &imageView) );
}

void CAllocator::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    //BeginCmd();
        VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        //barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        //barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // NOTE: subresourceRange is same as the one in ImageView
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            LOGE("Unsupported layout transition\n");
        }

        vkCmdPipelineBarrier(
            command_buffer,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    //EndCmd();
}

void CAllocator::DestroyImage(VkImage image, VmaAllocation alloc) {
    vmaDestroyImage(allocator, image, alloc);
}
//---------------------------------------------------

//------------------------CBuffer--------------------
CBuffer::CBuffer(CAllocator& allocator) : allocator(&allocator), allocation(), buffer(), count(), stride() {}
CBuffer::~CBuffer() { Clear(); }

void CBuffer::Clear() {
    vkQueueWaitIdle(allocator->queue);
    if(buffer) allocator->DestroyBuffer(buffer, allocation);
    buffer = 0;
    count  = 0;
    stride = 0;
}

void CBuffer::Data(const void* data, uint32_t count, uint32_t stride, VkBufferUsageFlagBits usage, VmaMemoryUsage memtype) {
    Clear();
    allocator->CreateBuffer(data, count * stride, usage, memtype, buffer, allocation);
    if(!buffer) return;
    this->count = count;
    this->stride = stride;
}
//----------------------------------------------------
//--------------------------VBO-----------------------
void CVBO::Data(void* data, uint32_t count, uint32_t stride) {
    CBuffer::Data(data, count, stride, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}
//----------------------------------------------------
//--------------------------IBO-----------------------
void CIBO::Data(const uint16_t* data, uint32_t count) {
    CBuffer::Data(data, count, 2, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

void CIBO::Data(const uint32_t* data, uint32_t count) {
    CBuffer::Data(data, count, 4, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}
//----------------------------------------------------
//--------------------------UBO-----------------------
void CUBO::Data(void* data, uint32_t size) {
    CBuffer::Data(data, 1, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}
//----------------------------------------------------

