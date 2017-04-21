// Copyright (C) 2017 by Rene Lindsay

#include "CRenderpass.h"
#include <array>

// Returns best available depth-stencil format which supports optimal tiling, or 0 if none found.
VkFormat GetSupportedDepthFormat(VkPhysicalDevice physicalDevice) {
    std::array<VkFormat, 5> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depthFormats) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED; // 0
}

// ----------------------------------Subpass------------------------------------

VkAttachmentDescription Attachment(VkFormat format, VkImageLayout finalLayout){
    VkAttachmentDescription attachment = {};
    attachment.format         = format;
    attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout    = finalLayout; //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return attachment;
}

CRenderpass::CSubpass::CSubpass(CRenderpass& renderpass):renderpass(renderpass), pdepth_ref(){}

CRenderpass::CSubpass::operator VkSubpassDescription(){
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = input_refs.size();
    subpass.pInputAttachments       = input_refs.data();
    subpass.colorAttachmentCount    = color_refs.size();
    subpass.pColorAttachments       = color_refs.data();
    subpass.pResolveAttachments     = NULL;
    subpass.pDepthStencilAttachment = pdepth_ref;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = NULL;
    return subpass;
}

void CRenderpass::CSubpass::UseAttachment(uint32_t attachment_index){
    if(renderpass.attachments.at(attachment_index).finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){  // depth-stencil attachment
        depth_ref.attachment = attachment_index;
        depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        pdepth_ref = &depth_ref;
    }else{  // color attachment
        VkAttachmentReference ref = {};
        ref.attachment = attachment_index;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(ref);
    }
}

void CRenderpass::CSubpass::UseAttachments(vector<uint32_t> attachment_indexes){
    for(auto i : attachment_indexes) UseAttachment(i);
}

void CRenderpass::CSubpass::InputAttachment(uint32_t attachment_index){
    VkAttachmentReference ref = {};
    ref.attachment = attachment_index;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    input_refs.push_back(ref);
}

void CRenderpass::CSubpass::InputAttachments(vector<uint32_t> attachment_indexes){
    for(auto i : attachment_indexes) InputAttachment(i);
}

// ----------------------------------Renderpass---------------------------------
CRenderpass::CRenderpass(VkDevice device) : device(device), renderpass() {}
CRenderpass::~CRenderpass() {Destroy();}

uint32_t CRenderpass::AddColorAttachment(VkFormat format){
    attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
    return attachments.size()-1;
}

uint32_t CRenderpass::AddDepthAttachment(VkFormat format){
    attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
    return attachments.size()-1;
}

CRenderpass::CSubpass& CRenderpass::AddSubpass(vector<uint32_t> attachment_indexes){
    subpasses.push_back(CSubpass(*this));
    CSubpass& subpass = subpasses.back();
    for(uint32_t i : attachment_indexes) subpass.UseAttachment(i);
    return subpass;
}

void CRenderpass::AddSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass){
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = srcSubpass; //VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies.push_back(dependency);
}

void CRenderpass::Create(){
    std::vector<VkSubpassDescription> subs;
    for(auto& sub : subpasses) subs.push_back(sub);  // build list of subpasses

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = NULL;
    rp_info.flags = 0;
    rp_info.attachmentCount = attachments.size();
    rp_info.pAttachments    = attachments.data();
    rp_info.subpassCount    = subs.size();
    rp_info.pSubpasses      = subs.data();
    rp_info.dependencyCount = dependencies.size();
    rp_info.pDependencies   = dependencies.data();
    VKERRCHECK(vkCreateRenderPass(device, &rp_info, nullptr, &renderpass));
}

void CRenderpass::Destroy(){
    if(renderpass) vkDestroyRenderPass(device, renderpass, nullptr);
    LOGI("Renderpass destroyed\n");
}


/*  // Minimal example of end-result
void CreateRenderPass(VkFormat swapchainImageFormat) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format         = swapchainImageFormat;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VKERRCHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpass));
}
*/
