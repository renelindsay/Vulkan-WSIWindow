// Copyright (C) 2017 by Rene Lindsay

#include "CRenderpass.h"

// -----------------------------------Subpass------------------------------------

VkAttachmentDescription Attachment(VkFormat format, VkImageLayout finalLayout) {
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

CRenderpass::CSubpass::operator VkSubpassDescription() {
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = (uint32_t)input_refs.size();
    subpass.pInputAttachments       = input_refs.data();
    subpass.colorAttachmentCount    = (uint32_t)color_refs.size();
    subpass.pColorAttachments       = color_refs.data();
    subpass.pResolveAttachments     = NULL;
    subpass.pDepthStencilAttachment = pdepth_ref;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = NULL;
    return subpass;
}

void CRenderpass::CSubpass::UseAttachment(uint32_t attachment_index) {
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

void CRenderpass::CSubpass::UseAttachments(vector<uint32_t> attachment_indexes) {
    for(auto i : attachment_indexes) UseAttachment(i);
}

void CRenderpass::CSubpass::InputAttachment(uint32_t attachment_index) {
    VkAttachmentReference ref = {};
    ref.attachment = attachment_index;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    input_refs.push_back(ref);
}

void CRenderpass::CSubpass::InputAttachments(vector<uint32_t> attachment_indexes) {
    for(auto i : attachment_indexes) InputAttachment(i);
}
// -----------------------------------------------------------------------------


// ----------------------------------Renderpass---------------------------------
CRenderpass::CRenderpass(VkDevice device) : device(device), renderpass() {}
CRenderpass::~CRenderpass() {Destroy();}

uint32_t CRenderpass::AddColorAttachment(VkFormat format, VkClearColorValue clearVal, VkImageLayout final_layout) {
    clearValues.push_back({});
    clearValues.back().color = clearVal;

    if (final_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) surface_format = format;
    attachments.push_back(Attachment(format, final_layout));
    return (uint32_t)attachments.size()-1;
}

uint32_t CRenderpass::AddDepthAttachment(VkFormat format, VkClearDepthStencilValue clearVal) {
//    ASSERT(depth_format == VK_FORMAT_UNDEFINED, "Renderpass can't have more than one depth buffer. ");
    depth_format = format;

    clearValues.push_back({});
    clearValues.back().depthStencil = clearVal;

    attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
    return (uint32_t)attachments.size()-1;
}

CRenderpass::CSubpass& CRenderpass::AddSubpass(vector<uint32_t> attachment_indexes) {
    subpasses.push_back(CSubpass(*this));
    CSubpass& subpass = subpasses.back();
    for(uint32_t i : attachment_indexes) subpass.UseAttachment(i);
    return subpass;
}

void CRenderpass::AddSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = srcSubpass; //VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies.push_back(dependency);
}

void CRenderpass::Create() {
    ASSERT(!renderpass, "Renderpass cannot be modified after its been linked to swapchain or pipeline.\n");
    if(renderpass) return;

    // Build subpass array
    std::vector<VkSubpassDescription> subs(subpasses.size());
    repeat(subpasses.size()) subs[i] = subpasses[i];

    VkRenderPassCreateInfo rp_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rp_info.pNext = NULL;
    rp_info.flags = 0;
    rp_info.attachmentCount = (uint32_t)attachments.size();
    rp_info.pAttachments    =           attachments.data();
    rp_info.subpassCount    = (uint32_t)subs.size();
    rp_info.pSubpasses      =           subs.data();
    rp_info.dependencyCount = (uint32_t)dependencies.size();
    rp_info.pDependencies   =           dependencies.data();
    VKERRCHECK(vkCreateRenderPass(device, &rp_info, nullptr, &renderpass));
    LOGI("Renderpass created\n");
}

void CRenderpass::Destroy() {
    if(!renderpass) return;
    vkDestroyRenderPass(device, renderpass, nullptr);
    renderpass = 0;
    LOGI("Renderpass destroyed\n");
}



/*  // Minimal RenderPass example

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
