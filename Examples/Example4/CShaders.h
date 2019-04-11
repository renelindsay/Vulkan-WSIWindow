/*
// Copyright (C) 2019 by Rene Lindsay
*
*  CSHADERS
*  --------
*  Use this class to load shaders, bind resources and generate descriptors.
*
*  Use the LoadVertShader/LoadFragShader functions to load SPIRV shader files.
*  CShaders uses SPIRV reflection, to determine what binding points the shader expects.
*  and determine descriptor set layouts and input variable names.
*  You can then use the Bind functions to bind UBO and Image resources by name.

*  Finally, call CreateDescriptorSet() to generate the following descriptor structs.
*      VkDescriptorSetLayout                   // Used by CPipeline
*      VkDescriptorPool                        // Used internally only
*      VkDescriptorSet                         // Used when you call vkCmdBindDescriptorSets
*      VkPipelineShaderStageCreateInfo         // Used by CPipeline
*      VkPipelineVertexInputStateCreateInfo    // Used by CPipeline
*
*  Use the returned VkDescriptorSet when calling vkCmdBindDescriptorSets in your renderpass.
*  The rest are used internally by the CPipeline class, when creating your pipeline.
*
*  Example:
*  --------
*    CShaders shaders(device);
*    shaders.LoadVertShader("shaders/vert.spv");                       // Load Vertex shader
*    shaders.LoadFragShader("shaders/frag.spv");                       // Load Fragment shader
*    shaders.Bind("ubo", ubo);                                         // Bind Uniform buffer to shader binding point, named "ubo"
*    shaders.Bind("texSampler", image);                                // Bind an Image to the shader binding point named "texSampler"
*    VkDescriptorSet descriptorSet = shaders.CreateDescriptorSet();    // Generate the descriptor set

*    CPipeline pipeline(device, renderpass, shaders);                  // Pass the other structs to CPipeline
*    pipeline.CreateGraphicsPipeline();
*
*
*  TODO:
*  -----
*  Add support for Tesselation, Geometry and Compute shaders
*  Add support for UBO arrays and constant buffers
*  Works with simple shaders, but needs to be tested with more complex shaders.
*
*/


#ifndef CSHADERS_H
#define CSHADERS_H

#include "WSIWindow.h"
#include "Buffers.h"
#include "spirv_reflect.h"


class CShaders {
    VkDevice device;
    VkShaderModule         vertShaderModule;
    VkShaderModule         fragShaderModule;
    VkDescriptorPool       descriptorPool;
    VkDescriptorSet        descriptorSet; 

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    std::string vert_entry_point_name;
    std::string frag_entry_point_name;
    
    //Vertex Inputs
    VkVertexInputBindingDescription binding_description;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

    struct ds_info_t {
        std::string name;
        union {
            VkDescriptorBufferInfo bufferInfo;
            VkDescriptorImageInfo  imageInfo;
        };
    };
    std::vector<ds_info_t> dsInfo;

    std::vector<char> LoadShader(const char* filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& spirv);
    void Parse(const std::vector<char>& spirv);
    void ParseInputs(SpvReflectShaderModule& module);

    void PrintModuleInfo        (const SpvReflectShaderModule& module);
    void PrintDescriptorSet     (const SpvReflectDescriptorSet& set);
    std::string ToStringDescriptorType(SpvReflectDescriptorType value);
    std::string ToStringGLSLType(const SpvReflectTypeDescription& type);

    void CheckBindings();
    VkDescriptorSetLayout& CreateDescriptorSetLayout();
    VkDescriptorPool&  CreateDescriptorPool();

public:
    CShaders(VkDevice device);
    ~CShaders();
    bool LoadVertShader(const char* filename);
    bool LoadFragShader(const char* filename);

    void Bind(std::string name, CUBO& ubo);
    void Bind(std::string name, VkImageView imageView, VkSampler sampler);
    void Bind(std::string name, const CvkImage& image);

    VkDescriptorSet& CreateDescriptorSet();

    // ---used by CPipeline--
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputs;
};

#endif
