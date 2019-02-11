#include "Buffers.h"


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
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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