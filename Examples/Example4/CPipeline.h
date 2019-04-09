#ifndef CPIPELINE_H
#define CPIPELINE_H

#include "WSIWindow.h"
#include "CSwapchain.h"
#include "CShaders.h"

class CPipeline {
    VkDevice     device;
    VkRenderPass renderpass;
    VkPipeline   graphicsPipeline;
    CShaders*    shaders;
  public:
    VkPipelineLayout pipelineLayout;

    CPipeline(VkDevice device, VkRenderPass renderpass, CShaders& shaders);
    ~CPipeline();

    VkPipeline CreateGraphicsPipeline();
    operator VkPipeline() const { return graphicsPipeline; }
};



#endif

