#ifndef CPIPELINE_H
#define CPIPELINE_H

#include "WSIWindow.h"
#include "CSwapchain.h"

class CPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline       graphicsPipeline;
    VkShaderModule   vertShaderModule;
    VkShaderModule   fragShaderModule;

    VkShaderModule LoadShader(const char* filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    VkDevice     device;
    VkRenderPass renderpass;

  public:
    CPipeline(VkDevice device, VkRenderPass renderpass);
    ~CPipeline();
    bool LoadVertShader(const char* filename);
    bool LoadFragShader(const char* filename);

    VkPipeline CreateGraphicsPipeline(VkExtent2D extent = {64,64});
    operator VkPipeline() const { return graphicsPipeline; }
};

#endif

