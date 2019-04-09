#ifndef CPIPELINE_H
#define CPIPELINE_H

#include "WSIWindow.h"
#include "CSwapchain.h"

class CPipeline {
    VkDescriptorSetLayout descriptorSetLayout;
    //VkPipelineLayout      pipelineLayout;
    VkPipeline            graphicsPipeline;
    VkShaderModule        vertShaderModule;
    VkShaderModule        fragShaderModule;

    VkShaderModule LoadShader(const char* filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    VkDevice     device;
    VkRenderPass renderpass;

  public:
    VkPipelineLayout      pipelineLayout;


    CPipeline(VkDevice device, VkRenderPass renderpass);
    ~CPipeline();
    bool LoadVertShader(const char* filename);
    bool LoadFragShader(const char* filename);

    //void CreateDescriptorSetLayout();
    void DescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

    VkPipeline CreateGraphicsPipeline(VkExtent2D extent = {64,64});
    operator VkPipeline() const { return graphicsPipeline; }
    //operator VkPipelineLayout() const { return pipelineLayout; }
};


//--------------------------------------------------------------------------



class CPipeline2 {
    VkDevice     device;
    VkRenderPass renderpass;
    VkPipeline   graphicsPipeline;
  public:
    VkPipelineLayout      pipelineLayout;

    CPipeline2(VkDevice device, VkRenderPass renderpass);
    ~CPipeline2();

    //VkPipeline CreateGraphicsPipeline(VkExtent2D extent = {64,64});
    //operator VkPipeline() const { return graphicsPipeline; }

};



#endif

