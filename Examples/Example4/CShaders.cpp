#define _CRT_SECURE_NO_WARNINGS
#include "CShaders.h"
//#include "spirv_reflect.c"

#include <iostream>
#include <algorithm>



// Returns the size in bytes of the provided VkFormat.
// As this is only intended for vertex attribute formats, not all VkFormats are supported.
static uint32_t FormatSize(VkFormat format) {
  uint32_t result = 0;
  switch (format) {
  case VK_FORMAT_UNDEFINED: result = 0; break;
  case VK_FORMAT_R4G4_UNORM_PACK8: result = 1; break;
  case VK_FORMAT_R4G4B4A4_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_B4G4R4A4_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_R5G6B5_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_B5G6R5_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_R5G5B5A1_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_B5G5R5A1_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_A1R5G5B5_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_R8_UNORM: result = 1; break;
  case VK_FORMAT_R8_SNORM: result = 1; break;
  case VK_FORMAT_R8_USCALED: result = 1; break;
  case VK_FORMAT_R8_SSCALED: result = 1; break;
  case VK_FORMAT_R8_UINT: result = 1; break;
  case VK_FORMAT_R8_SINT: result = 1; break;
  case VK_FORMAT_R8_SRGB: result = 1; break;
  case VK_FORMAT_R8G8_UNORM: result = 2; break;
  case VK_FORMAT_R8G8_SNORM: result = 2; break;
  case VK_FORMAT_R8G8_USCALED: result = 2; break;
  case VK_FORMAT_R8G8_SSCALED: result = 2; break;
  case VK_FORMAT_R8G8_UINT: result = 2; break;
  case VK_FORMAT_R8G8_SINT: result = 2; break;
  case VK_FORMAT_R8G8_SRGB: result = 2; break;
  case VK_FORMAT_R8G8B8_UNORM: result = 3; break;
  case VK_FORMAT_R8G8B8_SNORM: result = 3; break;
  case VK_FORMAT_R8G8B8_USCALED: result = 3; break;
  case VK_FORMAT_R8G8B8_SSCALED: result = 3; break;
  case VK_FORMAT_R8G8B8_UINT: result = 3; break;
  case VK_FORMAT_R8G8B8_SINT: result = 3; break;
  case VK_FORMAT_R8G8B8_SRGB: result = 3; break;
  case VK_FORMAT_B8G8R8_UNORM: result = 3; break;
  case VK_FORMAT_B8G8R8_SNORM: result = 3; break;
  case VK_FORMAT_B8G8R8_USCALED: result = 3; break;
  case VK_FORMAT_B8G8R8_SSCALED: result = 3; break;
  case VK_FORMAT_B8G8R8_UINT: result = 3; break;
  case VK_FORMAT_B8G8R8_SINT: result = 3; break;
  case VK_FORMAT_B8G8R8_SRGB: result = 3; break;
  case VK_FORMAT_R8G8B8A8_UNORM: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SNORM: result = 4; break;
  case VK_FORMAT_R8G8B8A8_USCALED: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SSCALED: result = 4; break;
  case VK_FORMAT_R8G8B8A8_UINT: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SINT: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SRGB: result = 4; break;
  case VK_FORMAT_B8G8R8A8_UNORM: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SNORM: result = 4; break;
  case VK_FORMAT_B8G8R8A8_USCALED: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SSCALED: result = 4; break;
  case VK_FORMAT_B8G8R8A8_UINT: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SINT: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SRGB: result = 4; break;
  case VK_FORMAT_A8B8G8R8_UNORM_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SNORM_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_USCALED_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_UINT_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SINT_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SRGB_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_UNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_SNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_USCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_UINT_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_SINT_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_UNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_SNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_USCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_UINT_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_SINT_PACK32: result = 4; break;
  case VK_FORMAT_R16_UNORM: result = 2; break;
  case VK_FORMAT_R16_SNORM: result = 2; break;
  case VK_FORMAT_R16_USCALED: result = 2; break;
  case VK_FORMAT_R16_SSCALED: result = 2; break;
  case VK_FORMAT_R16_UINT: result = 2; break;
  case VK_FORMAT_R16_SINT: result = 2; break;
  case VK_FORMAT_R16_SFLOAT: result = 2; break;
  case VK_FORMAT_R16G16_UNORM: result = 4; break;
  case VK_FORMAT_R16G16_SNORM: result = 4; break;
  case VK_FORMAT_R16G16_USCALED: result = 4; break;
  case VK_FORMAT_R16G16_SSCALED: result = 4; break;
  case VK_FORMAT_R16G16_UINT: result = 4; break;
  case VK_FORMAT_R16G16_SINT: result = 4; break;
  case VK_FORMAT_R16G16_SFLOAT: result = 4; break;
  case VK_FORMAT_R16G16B16_UNORM: result = 6; break;
  case VK_FORMAT_R16G16B16_SNORM: result = 6; break;
  case VK_FORMAT_R16G16B16_USCALED: result = 6; break;
  case VK_FORMAT_R16G16B16_SSCALED: result = 6; break;
  case VK_FORMAT_R16G16B16_UINT: result = 6; break;
  case VK_FORMAT_R16G16B16_SINT: result = 6; break;
  case VK_FORMAT_R16G16B16_SFLOAT: result = 6; break;
  case VK_FORMAT_R16G16B16A16_UNORM: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SNORM: result = 8; break;
  case VK_FORMAT_R16G16B16A16_USCALED: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SSCALED: result = 8; break;
  case VK_FORMAT_R16G16B16A16_UINT: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SINT: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SFLOAT: result = 8; break;
  case VK_FORMAT_R32_UINT: result = 4; break;
  case VK_FORMAT_R32_SINT: result = 4; break;
  case VK_FORMAT_R32_SFLOAT: result = 4; break;
  case VK_FORMAT_R32G32_UINT: result = 8; break;
  case VK_FORMAT_R32G32_SINT: result = 8; break;
  case VK_FORMAT_R32G32_SFLOAT: result = 8; break;
  case VK_FORMAT_R32G32B32_UINT: result = 12; break;
  case VK_FORMAT_R32G32B32_SINT: result = 12; break;
  case VK_FORMAT_R32G32B32_SFLOAT: result = 12; break;
  case VK_FORMAT_R32G32B32A32_UINT: result = 16; break;
  case VK_FORMAT_R32G32B32A32_SINT: result = 16; break;
  case VK_FORMAT_R32G32B32A32_SFLOAT: result = 16; break;
  case VK_FORMAT_R64_UINT: result = 8; break;
  case VK_FORMAT_R64_SINT: result = 8; break;
  case VK_FORMAT_R64_SFLOAT: result = 8; break;
  case VK_FORMAT_R64G64_UINT: result = 16; break;
  case VK_FORMAT_R64G64_SINT: result = 16; break;
  case VK_FORMAT_R64G64_SFLOAT: result = 16; break;
  case VK_FORMAT_R64G64B64_UINT: result = 24; break;
  case VK_FORMAT_R64G64B64_SINT: result = 24; break;
  case VK_FORMAT_R64G64B64_SFLOAT: result = 24; break;
  case VK_FORMAT_R64G64B64A64_UINT: result = 32; break;
  case VK_FORMAT_R64G64B64A64_SINT: result = 32; break;
  case VK_FORMAT_R64G64B64A64_SFLOAT: result = 32; break;
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32: result = 4; break;
  case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: result = 4; break;

  default:
    break;
  }
  return result;
}

/*
std::string ToStringSpvBuiltIn(SpvBuiltIn built_in) {
  switch (built_in) {
    case SpvBuiltInPosition                    : return "Position";
    case SpvBuiltInPointSize                   : return "PointSize";
    case SpvBuiltInClipDistance                : return "ClipDistance";
    case SpvBuiltInCullDistance                : return "CullDistance";
    case SpvBuiltInVertexId                    : return "VertexId";
    case SpvBuiltInInstanceId                  : return "InstanceId";
    case SpvBuiltInPrimitiveId                 : return "PrimitiveId";
    case SpvBuiltInInvocationId                : return "InvocationId";
    case SpvBuiltInLayer                       : return "Layer";
    case SpvBuiltInViewportIndex               : return "ViewportIndex";
    case SpvBuiltInTessLevelOuter              : return "TessLevelOuter";
    case SpvBuiltInTessLevelInner              : return "TessLevelInner";
    case SpvBuiltInTessCoord                   : return "TessCoord";
    case SpvBuiltInPatchVertices               : return "PatchVertices";
    case SpvBuiltInFragCoord                   : return "FragCoord";
    case SpvBuiltInPointCoord                  : return "PointCoord";
    case SpvBuiltInFrontFacing                 : return "FrontFacing";
    case SpvBuiltInSampleId                    : return "SampleId";
    case SpvBuiltInSamplePosition              : return "SamplePosition";
    case SpvBuiltInSampleMask                  : return "SampleMask";
    case SpvBuiltInFragDepth                   : return "FragDepth";
    case SpvBuiltInHelperInvocation            : return "HelperInvocation";
    case SpvBuiltInNumWorkgroups               : return "NumWorkgroups";
    case SpvBuiltInWorkgroupSize               : return "WorkgroupSize";
    case SpvBuiltInWorkgroupId                 : return "WorkgroupId";
    case SpvBuiltInLocalInvocationId           : return "LocalInvocationId";
    case SpvBuiltInGlobalInvocationId          : return "GlobalInvocationId";
    case SpvBuiltInLocalInvocationIndex        : return "LocalInvocationIndex";
    case SpvBuiltInWorkDim                     : return "WorkDim";
    case SpvBuiltInGlobalSize                  : return "GlobalSize";
    case SpvBuiltInEnqueuedWorkgroupSize       : return "EnqueuedWorkgroupSize";
    case SpvBuiltInGlobalOffset                : return "GlobalOffset";
    case SpvBuiltInGlobalLinearId              : return "GlobalLinearId";
    case SpvBuiltInSubgroupSize                : return "SubgroupSize";
    case SpvBuiltInSubgroupMaxSize             : return "SubgroupMaxSize";
    case SpvBuiltInNumSubgroups                : return "NumSubgroups";
    case SpvBuiltInNumEnqueuedSubgroups        : return "NumEnqueuedSubgroups";
    case SpvBuiltInSubgroupId                  : return "SubgroupId";
    case SpvBuiltInSubgroupLocalInvocationId   : return "SubgroupLocalInvocationId";
    case SpvBuiltInVertexIndex                 : return "VertexIndex";
    case SpvBuiltInInstanceIndex               : return "InstanceIndex";
    case SpvBuiltInSubgroupEqMaskKHR           : return "SubgroupEqMaskKHR";
    case SpvBuiltInSubgroupGeMaskKHR           : return "SubgroupGeMaskKHR";
    case SpvBuiltInSubgroupGtMaskKHR           : return "SubgroupGtMaskKHR";
    case SpvBuiltInSubgroupLeMaskKHR           : return "SubgroupLeMaskKHR";
    case SpvBuiltInSubgroupLtMaskKHR           : return "SubgroupLtMaskKHR";
    case SpvBuiltInBaseVertex                  : return "BaseVertex";
    case SpvBuiltInBaseInstance                : return "BaseInstance";
    case SpvBuiltInDrawIndex                   : return "DrawIndex";
    case SpvBuiltInDeviceIndex                 : return "DeviceIndex";
    case SpvBuiltInViewIndex                   : return "ViewIndex";
    case SpvBuiltInBaryCoordNoPerspAMD         : return "BaryCoordNoPerspAMD";
    case SpvBuiltInBaryCoordNoPerspCentroidAMD : return "BaryCoordNoPerspCentroidAMD";
    case SpvBuiltInBaryCoordNoPerspSampleAMD   : return "BaryCoordNoPerspSampleAMD";
    case SpvBuiltInBaryCoordSmoothAMD          : return "BaryCoordSmoothAMD";
    case SpvBuiltInBaryCoordSmoothCentroidAMD  : return "BaryCoordSmoothCentroidAMD";
    case SpvBuiltInBaryCoordSmoothSampleAMD    : return "BaryCoordSmoothSampleAMD";
    case SpvBuiltInBaryCoordPullModelAMD       : return "BaryCoordPullModelAMD";
    case SpvBuiltInFragStencilRefEXT           : return "FragStencilRefEXT";
    case SpvBuiltInViewportMaskNV              : return "ViewportMaskNV";
    case SpvBuiltInSecondaryPositionNV         : return "SecondaryPositionNV";
    case SpvBuiltInSecondaryViewportMaskNV     : return "SecondaryViewportMaskNV";
    case SpvBuiltInPositionPerViewNV           : return "PositionPerViewNV";
    case SpvBuiltInViewportMaskPerViewNV       : return "ViewportMaskPerViewNV";

    case SpvBuiltInMax:
    default:
      break;
  }
  // unhandled SpvBuiltIn enum value
  return "???";
}
*/

static std::string ToStringGlslType(const SpvReflectTypeDescription& type) {
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
//==================================================================================================


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
        printf("    %d : %s %s\n", var->location, ToStringGlslType(*var->type_description).c_str(), var->name);
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
//-----------------------------------------------------------------------------------------------