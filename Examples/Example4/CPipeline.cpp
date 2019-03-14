#define _CRT_SECURE_NO_WARNINGS
#include "CPipeline.h"
#include "WSIWindow.h"

CPipeline::CPipeline(VkDevice device, VkRenderPass renderpass) :
    device(device), renderpass(renderpass), 
    vertShaderModule(), fragShaderModule(),
    pipelineLayout(), graphicsPipeline() {}

CPipeline::~CPipeline(){
    if (device) vkDeviceWaitIdle(device);
    //if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    if (pipelineLayout)      vkDestroyPipelineLayout     (device, pipelineLayout,      nullptr);
    if (graphicsPipeline)    vkDestroyPipeline           (device, graphicsPipeline,    nullptr);
    if (vertShaderModule)    vkDestroyShaderModule       (device, vertShaderModule,    nullptr);
    if (fragShaderModule)    vkDestroyShaderModule       (device, fragShaderModule,    nullptr);
}

// -- Shader modules ---
bool CPipeline::LoadVertShader(const char* filename) {
    vertShaderModule = LoadShader(filename);
    return !!vertShaderModule;
}

bool CPipeline::LoadFragShader(const char* filename) {
    fragShaderModule = LoadShader(filename);
    return !!fragShaderModule;
}

VkShaderModule CPipeline::LoadShader(const char* filename) {
    // Read File
    printf("Load Shader: %s... ", filename);
    FILE* file = fopen(filename, "rb");
    printf("%s\n", (file?"Found":"Not found"));
    assert(!!file && "File not found");

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    std::vector<char> buffer(file_size);
    rewind(file);
    fread(buffer.data(), 1, file_size, file);
    fclose(file);

    return CreateShaderModule(buffer); 
}

VkShaderModule CPipeline::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();

    std::vector<uint32_t> codeAligned(code.size() / 4 + 1);
    memcpy(codeAligned.data(), code.data(), code.size());
    createInfo.pCode = codeAligned.data();

    VkShaderModule shaderModule = 0;
    VKERRCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}
// ---------------------
/*
void CPipeline::CreateDescriptorSetLayout() {
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
*/
void CPipeline::DescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout){
    this->descriptorSetLayout = descriptorSetLayout;
}


// --- Pipeline ---
VkPipeline CPipeline::CreateGraphicsPipeline(VkExtent2D extent) {
    //vertShaderModule = LoadShader("shaders/vert.spv");
    //fragShaderModule = LoadShader("shaders/frag.spv");
    ASSERT(!!vertShaderModule, "No Vertex Shader loaded. ");
    ASSERT(!!fragShaderModule, "No Fragment Shader loaded. ");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


    // VertexInputState
    VkVertexInputBindingDescription vi_binding;
    vi_binding.binding = 0;
    vi_binding.stride  = 28;  // sizeof vertex
    vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vi_attribs[3];
    vi_attribs[0].binding = 0;
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    vi_attribs[0].offset = 0;

    vi_attribs[1].binding = 0;
    vi_attribs[1].location = 1;
    vi_attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vi_attribs[1].offset = 8;

    vi_attribs[2].binding = 0;
    vi_attribs[2].location = 2;
    vi_attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vi_attribs[2].offset = 20;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vi_binding;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexAttributeDescriptions = vi_attribs;
    //-----------------

/*
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
*/


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extent.width;
    viewport.height = (float) extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable       = VK_TRUE;
    depthStencilState.depthWriteEnable      = VK_TRUE;
    depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable     = VK_FALSE;
    //depthStencilState.front.failOp          = VK_STENCIL_OP_KEEP;
    //depthStencilState.front.passOp          = VK_STENCIL_OP_KEEP;
    //depthStencilState.front.compareOp       = VK_COMPARE_OP_ALWAYS;
    //depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
    //depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
    //depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    //depthStencilState.minDepthBounds        = 0;
    //depthStencilState.maxDepthBounds        = MAXFLOAT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    VKERRCHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    //pipelineInfo.pTessellationState = 0;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlending;
    //pipelineInfo.pDynamicState = 0;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderpass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    //pipelineInfo.basePipelineIndex = 0;

    VKERRCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
    return graphicsPipeline;
}
