#include "CDescriptor.h"

CDescriptors::CDescriptors(VkDevice device, uint32_t numSwapChainImages) 
    : device(device), descriptorSetLayout(), descriptorPool(), numSwapChainImages(numSwapChainImages) {};

CDescriptors::~CDescriptors(){
    if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    if (descriptorPool)      vkDestroyDescriptorPool(device, descriptorPool, nullptr);
};

void CDescriptors::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    VKERRCHECK( vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) );
}

void CDescriptors::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = numSwapChainImages;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    VKERRCHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool); );
};

void CDescriptors::CreateDescriptorSet(VkBuffer ubo, uint32_t size) {
    std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(numSwapChainImages);
    VKERRCHECK( vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()); );

    for (size_t i = 0; i < numSwapChainImages; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = ubo;  //uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = size; //sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}
