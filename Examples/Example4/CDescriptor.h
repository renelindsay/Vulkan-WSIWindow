#ifndef CDESCRIPTOR_H
#define CDESCRIPTOR_H

#include "WSIWindow.h"
#include "CDevices.h"
#include "Buffers.h"

class CDescriptors {
    VkDevice                     device;
    VkDescriptorSetLayout        descriptorSetLayout;
    VkDescriptorPool             descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;  // TODO: Move to CSwapchain?
    uint32_t numSwapChainImages = 1;

public:
    CDescriptors(VkDevice device, uint32_t numSwapChainImages = 1);
    virtual ~CDescriptors();

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSet(VkBuffer ubo, uint32_t size);

    VkDescriptorSet* getDescriptorSet() {return &descriptorSets[0];};  // tmp

    operator VkDescriptorSetLayout () {return descriptorSetLayout;}
};


#endif
