#ifndef CSHADERS_H
#define CSHADERS_H

#include "WSIWindow.h"
//#include "CDevices.h"
#include "Buffers.h"

#include "spirv_reflect.h"

class CShaders {
    std::vector<char> LoadShader(const char* filename);
public:

    void Parse();

    void PrintModuleInfo(std::ostream& os, const SpvReflectShaderModule& obj, const char* indent = "");
    void PrintDescriptorSet(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent = "");
    void PrintDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "");
    //void PrintInterfaceVariable(std::ostream& os, SpvSourceLanguage src_lang, const SpvReflectInterfaceVariable& obj, const char* indent);

};


//-------------------------------------------------------------


class CShaders2 {
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
    CShaders2(VkDevice device);
    ~CShaders2();
    bool LoadVertShader(const char* filename);
    bool LoadFragShader(const char* filename);

    VkDescriptorSetLayout& CreateDescriptorSetLayout();
    VkDescriptorPool&      CreateDescriptorPool();
    VkDescriptorSet&       CreateDescriptorSet();

    void Bind(std::string name, CUBO& ubo);
    void Bind(std::string name, VkImageView imageView, VkSampler sampler);


};

#endif
