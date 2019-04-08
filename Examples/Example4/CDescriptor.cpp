#include "CDescriptor.h"

CDescriptors::CDescriptors(VkDevice device, uint32_t numSwapChainImages) 
    : device(device), descriptorSetLayout(), descriptorPool(), numSwapChainImages(numSwapChainImages) {};

CDescriptors::~CDescriptors(){
    if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    if (descriptorPool)      vkDestroyDescriptorPool(device, descriptorPool, nullptr);
};

void CDescriptors::CreateDescriptorSetLayout() {
    // UBO
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    // Texture
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding >bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings.data(); //&uboLayoutBinding;
    VKERRCHECK( vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) );
}

void CDescriptors::CreateDescriptorPool() {
/*
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = numSwapChainImages;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
*/

    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = numSwapChainImages;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = numSwapChainImages;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes    = poolSizes;

    VKERRCHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool); );
};

void CDescriptors::CreateDescriptorSet(VkBuffer ubo, uint32_t size, VkImageView imageView, VkSampler sampler) {
    std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = numSwapChainImages;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(numSwapChainImages);
    VKERRCHECK( vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()); );

    for (size_t i = 0; i < numSwapChainImages; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = ubo;  //uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;  //size; //sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 2, descriptorWrites, 0, nullptr);
    }
}
