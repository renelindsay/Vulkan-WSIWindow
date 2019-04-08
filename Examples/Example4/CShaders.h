#ifndef CSHADERS_H
#define CSHADERS_H

#include "WSIWindow.h"
//#include "CDevices.h"
#include "Buffers.h"

#include "spirv_reflect.h"


class CShaders {
    VkDevice device;
    VkShaderModule         vertShaderModule;
    VkShaderModule         fragShaderModule;
    VkDescriptorSetLayout  descriptorSetLayout;
    VkDescriptorPool       descriptorPool;
    VkDescriptorSet        descriptorSet; 

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::string vert_entry_point_name;
    std::string frag_entry_point_name;
    
    //Vertex Attributes
    VkVertexInputBindingDescription binding_description;
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
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
    //void ParseIO(SpvReflectShaderModule& module);


    void PrintModuleInfo        (const SpvReflectShaderModule& module);
    void PrintDescriptorSet     (const SpvReflectDescriptorSet& set);
    std::string ToStringDescriptorType(SpvReflectDescriptorType value);
    //void PrintIOVars();


public:
    CShaders(VkDevice device);
    ~CShaders();
    bool LoadVertShader(const char* filename);
    bool LoadFragShader(const char* filename);

    VkDescriptorSetLayout& CreateDescriptorSetLayout();
    VkDescriptorPool&      CreateDescriptorPool();
    VkDescriptorSet&       CreateDescriptorSet();

    void Bind(std::string name, CUBO& ubo);
    void Bind(std::string name, VkImageView imageView, VkSampler sampler);


    VkDescriptorSetLayout DescriptorSetLayout() { return descriptorSetLayout;}
    VkPipelineShaderStageCreateInfo* ShaderStages() {return shaderStages.data();}
    VkPipelineVertexInputStateCreateInfo* VertexInputs() {return &vertex_input_info;}
};

#endif
