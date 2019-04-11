#define _CRT_SECURE_NO_WARNINGS
#include "CShaders.h"
#include "VkFormats.h"

#include <iostream>
#include <algorithm>

CShaders::CShaders(VkDevice device) : device(device), vertShaderModule(), fragShaderModule(),
                                        descriptorSetLayout(), descriptorPool(), descriptorSet() {}

CShaders::~CShaders() {
    if (device) vkDeviceWaitIdle(device);
    if (descriptorPool)      vkDestroyDescriptorPool     (device, descriptorPool,      nullptr);
    if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    if (vertShaderModule)    vkDestroyShaderModule       (device, vertShaderModule,    nullptr);
    if (fragShaderModule)    vkDestroyShaderModule       (device, fragShaderModule,    nullptr);
}

bool CShaders::LoadVertShader(const char* filename) {
    assert(!vertShaderModule && "Vertex shader already loaded.");
    auto spirv = LoadShader(filename);
    vertShaderModule = CreateShaderModule(spirv);
    Parse(spirv);
    return !!vertShaderModule;
}

bool CShaders::LoadFragShader(const char* filename) {
    assert(!fragShaderModule && "Fragment shader already loaded.");
    auto spirv = LoadShader(filename);
    fragShaderModule = CreateShaderModule(spirv);
    Parse(spirv);
    return !!fragShaderModule;
}

std::vector<char> CShaders::LoadShader(const char* filename) {
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

    return buffer;
}

VkShaderModule CShaders::CreateShaderModule(const std::vector<char>& spirv) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size();

    std::vector<uint32_t> codeAligned(spirv.size() / 4 + 1);
    memcpy(codeAligned.data(), spirv.data(), spirv.size());
    createInfo.pCode = codeAligned.data();

    VkShaderModule shaderModule = 0;
    VKERRCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

void CShaders::Parse(const std::vector<char>& spirv) {
    SpvReflectShaderModule module = {};
    SpvReflectResult result = spvReflectCreateShaderModule(spirv.size(), spirv.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    VkShaderStageFlagBits stage = (VkShaderStageFlagBits)module.shader_stage;

    //SpvReflectDescriptorBinding
    for(uint32_t i = 0; i<sets.size(); ++i) {
        SpvReflectDescriptorSet& set = *(sets[i]);
        for(uint32_t j = 0; j<set.binding_count; ++j) {
            SpvReflectDescriptorBinding& setBinding = *(set.bindings[j]);
            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.binding         = setBinding.binding;
            layoutBinding.descriptorType  = (VkDescriptorType)setBinding.descriptor_type; 
            layoutBinding.descriptorCount = 1;
            for (uint32_t i_dim = 0; i_dim < setBinding.array.dims_count; ++i_dim) {
                layoutBinding.descriptorCount *= setBinding.array.dims[i_dim];
            }
            layoutBinding.stageFlags         = stage;
            layoutBinding.pImmutableSamplers = nullptr; // Optional
            bindings.push_back(layoutBinding);
        }
    }

    // WriteDescriptorSets
    for(auto& pset : sets) {
        auto& set = *pset;
        for (uint32_t i = 0; i < set.binding_count; ++i) {
            const SpvReflectDescriptorBinding& binding = *set.bindings[i];
            VkDescriptorType type = (VkDescriptorType)binding.descriptor_type;

            dsInfo.push_back({binding.name});

            VkWriteDescriptorSet writeDS = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            writeDS.dstSet = descriptorSet;
            writeDS.dstBinding = binding.binding; //0;
            writeDS.dstArrayElement = 0;
            writeDS.descriptorType  = type;  //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDS.descriptorCount = 1;
            //writeDS.pBufferInfo = &bufferInfo;
            //if(type==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)         writeDS.pBufferInfo = &dsInfo.back().bufferInfo;
            //if(type==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) writeDS.pImageInfo  = &dsInfo.back().imageInfo;
            descriptorWrites.push_back(writeDS);
        }
    }

    // ShaderStages
    VkPipelineShaderStageCreateInfo stageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stageInfo.stage = stage;  //VK_SHADER_STAGE_VERTEX_BIT;
    if(stage == VK_SHADER_STAGE_VERTEX_BIT) {
        stageInfo.module = vertShaderModule;
        vert_entry_point_name = module.entry_point_name;
        stageInfo.pName = vert_entry_point_name.c_str(); //"main";
    }
    if(stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        stageInfo.module = fragShaderModule;
        frag_entry_point_name = module.entry_point_name;
        stageInfo.pName = frag_entry_point_name.c_str(); //"main";
    }
    shaderStages.push_back(stageInfo);

#ifdef ENABLE_LOGGING
    PrintModuleInfo(module);
    for(auto& set : sets) PrintDescriptorSet(*set);
#endif

    if(stage == VK_SHADER_STAGE_VERTEX_BIT) ParseInputs(module);

    spvReflectDestroyShaderModule(&module);
}

void CShaders::ParseInputs(SpvReflectShaderModule& module) {
    uint32_t count = 0;
    SpvReflectResult result;
    result = spvReflectEnumerateInputVariables(&module, &count, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectInterfaceVariable*> input_vars(count);
    result = spvReflectEnumerateInputVariables(&module, &count, input_vars.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // Sort attributes by location
    std::sort(std::begin(input_vars), std::end(input_vars),
      [](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) {
      return a->location < b->location; });

    //Pupulate VkPipelineVertexInputStateCreateInfo structure
    //VkVertexInputBindingDescription binding_description = {};
    binding_description = {};
    binding_description.binding = 0;
    binding_description.stride = 0;  // computed below
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    //VkPipelineVertexInputStateCreateInfo vertex_input_info = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    //std::vector<VkVertexInputAttributeDescription> attribute_descriptions(input_vars.size(), VkVertexInputAttributeDescription{});

    vertexInputs = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    attribute_descriptions.resize(input_vars.size());

    for (size_t i_var = 0; i_var < input_vars.size(); ++i_var) {
      const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);
      VkVertexInputAttributeDescription& attr_desc = attribute_descriptions[i_var];
      attr_desc.location = refl_var.location;
      attr_desc.binding = binding_description.binding;
      attr_desc.format = static_cast<VkFormat>(refl_var.format);
      attr_desc.offset = 0;  // final offset computed below after sorting.
    }

    // Compute final offsets of each attribute, and total vertex stride.
    for (auto& attribute : attribute_descriptions) {
      uint32_t format_size = FormatSize(attribute.format);
      attribute.offset = binding_description.stride;
      binding_description.stride += format_size;
    }

    vertexInputs.vertexBindingDescriptionCount = 1;
    vertexInputs.pVertexBindingDescriptions = &binding_description;
    vertexInputs.vertexAttributeDescriptionCount = (uint32_t)attribute_descriptions.size();
    vertexInputs.pVertexAttributeDescriptions    =           attribute_descriptions.data();


#ifdef ENABLE_LOGGING
    // Print input attributes
    printf("  Vertex Input attributes:\n");
    for(auto& var : input_vars) 
        printf("    %d : %s %s\n", var->location, ToStringGLSLType(*var->type_description).c_str(), var->name);
    printf("\n");
#endif
}

//----DescriptorSetLayout----
VkDescriptorSetLayout& CShaders::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutCreateInfo create_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    create_info.bindingCount = (uint32_t)bindings.size();
    create_info.pBindings    =           bindings.data();
    VKERRCHECK( vkCreateDescriptorSetLayout(device, &create_info, nullptr, &descriptorSetLayout) );
    return descriptorSetLayout;
}

//----DescriptorPool----
VkDescriptorPool& CShaders::CreateDescriptorPool() {
    uint32_t cnt = (uint32_t)bindings.size();
    std::vector<VkDescriptorPoolSize> poolSizes(cnt);
    for(uint32_t i = 0; i<cnt; ++i) { 
        poolSizes[i].type            = bindings[i].descriptorType;
        poolSizes[i].descriptorCount = bindings[i].descriptorCount;
    }

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = 1; //numSwapChainImages;
    poolInfo.poolSizeCount = cnt;
    poolInfo.pPoolSizes    = poolSizes.data();
    VKERRCHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool); );
    return descriptorPool;
}

//----DescriptorSet----
VkDescriptorSet& CShaders::CreateDescriptorSet() {
    CheckBindings();
    CreateDescriptorSetLayout();
    CreateDescriptorPool();

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    VKERRCHECK( vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet); );

    uint32_t cnt = (uint32_t)descriptorWrites.size();
    for(uint32_t i=0; i<cnt; ++i) {
        descriptorWrites[i].dstSet      = descriptorSet;
        descriptorWrites[i].pBufferInfo = &dsInfo[i].bufferInfo;
        descriptorWrites[i].pImageInfo  = &dsInfo[i].imageInfo;
    }

    vkUpdateDescriptorSets(device, 2, descriptorWrites.data(), 0, nullptr);
    return descriptorSet;
}


void CShaders::Bind(std::string name, CUBO& ubo) { 
    for(auto& item : dsInfo) {
        if(item.name == name) {
            LOGI("Bind UBO   to shader-in: \"%s\"\n", name.c_str());
            item.bufferInfo.buffer = ubo;
            item.bufferInfo.offset = 0;
            item.bufferInfo.range  = VK_WHOLE_SIZE;
            return;
        }
    } 
    LOGE("Failed to bind UBO to shader var: \"%s\" (Not found)\n", name.c_str());
}

void CShaders::Bind(std::string name, VkImageView imageView, VkSampler sampler) { 
    for(auto& item : dsInfo) {
        if(item.name == name) {
            LOGI("Bind Image to shader-in: \"%s\"\n", name.c_str());
            item.imageInfo.imageView   = imageView;
            item.imageInfo.sampler     = sampler;
            item.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            return;
        }
    } 
    LOGE("Failed to bind Image to shader var: \"%s\" (Not found)\n", name.c_str());
}

void CShaders::Bind(std::string name, const CvkImage& image) {
    Bind(name, image.view, image.sampler);
}

void CShaders::CheckBindings() { 
    for(auto& item : dsInfo) {
        if(item.bufferInfo.buffer == 0) {
            LOGE("Shader item: \"%s\" was not bound. Set a binding before creating the DescriptorSet.\n", item.name.c_str());
            PAUSE; 
            exit(0);
        }
    }
}


//---------------------------------------------PRINT---------------------------------------------
void CShaders::PrintModuleInfo(const SpvReflectShaderModule& module) {
    //printf("  Source language : %s\n", spvReflectSourceLanguage(module.source_language));
    printf("  Entry Point     : %s\n", module.entry_point_name);

    const char* stage ="";
    switch(module.shader_stage) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT                   : stage = "VERTEX"; break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT     : stage = "TESSELLATION_CONTROL"; break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT  : stage = "TESSELLATION_EVALUATION"; break;
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT                 : stage = "GEOMETRY"; break;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT                 : stage = "FRAGMENT"; break;
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT                  : stage = "COMPUTE"; break;
        default                                                    : stage = "UNKNOWN";
    }
    printf("  Shader stage    : %s\n", stage);
}

//----DescriptorSet----
void CShaders::PrintDescriptorSet(const SpvReflectDescriptorSet& set) {
    printf("  Descriptor set  : %d\n", set.set);
    for (uint32_t i = 0; i < set.binding_count; ++i) {
        const SpvReflectDescriptorBinding& binding = *set.bindings[i];
        printf("         binding  : %d\n", binding.binding);
        printf("         name     : %s\n", binding.name);
        printf("         type     : %s\n", ToStringDescriptorType(binding.descriptor_type).c_str());
    }
    printf("\n");
}

std::string CShaders::ToStringDescriptorType(SpvReflectDescriptorType value) {
    switch (value) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER                : return "VK_DESCRIPTOR_TYPE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE          : return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER         : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       : return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
    }
    // unhandled SpvReflectDescriptorType enum value
    return "VK_DESCRIPTOR_TYPE_???";
}

std::string CShaders::ToStringGLSLType(const SpvReflectTypeDescription& type) {
  switch (type.op) {
    case SpvOpTypeVector: {
      switch (type.traits.numeric.scalar.width) {
        case 32: {
          switch (type.traits.numeric.vector.component_count) {
          case 2: return "vec2";
          case 3: return "vec3";
          case 4: return "vec4";
          }
        }
        break;

        case 64: {
          switch (type.traits.numeric.vector.component_count) {
          case 2: return "dvec2";
          case 3: return "dvec3";
          case 4: return "dvec4";
          }
        }
        break;
      }
    }
    break;

    default:
      break;
  }
  return "";
}

//-----------------------------------------------------------------------------------------------