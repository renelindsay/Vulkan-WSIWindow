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

CSubpass::CSubpass(CRenderpass& renderpass):renderpass(renderpass), depth_ref(){}

CSubpass::operator VkSubpassDescription(){
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = NULL;
    subpass.colorAttachmentCount    = color_refs.size();
    subpass.pColorAttachments       = color_refs.data();
    subpass.pResolveAttachments     = NULL;
    subpass.pDepthStencilAttachment = &depth_ref;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = NULL;
    return subpass;
}
/*
uint32_t CSubpass::AddColorAttachment(VkFormat format){
    renderpass.attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
    VkAttachmentReference ref = {};
    ref.attachment = renderpass.attachments.size()-1;  //index
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_refs.push_back(ref);
}

uint32_t CSubpass::AddDepthAttachment(VkFormat format){
    assert(depth_ref.attachment == 0);
    renderpass.attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
    depth_ref.attachment = renderpass.attachments.size()-1;  //index
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}
*/
/*
void CSubpass::UseColorAttachment(uint32_t attachment_index){
    VkAttachmentReference ref = {};
    ref.attachment = attachment_index;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_refs.push_back(ref);
}

void CSubpass::UseDepthAttachment(uint32_t attachment_index){
    depth_ref.attachment = attachment_index;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}
*/

void CSubpass::UseAttachment(uint32_t attachment_index){
    if(renderpass.attachments.at(attachment_index).finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){  // depth-stencil attachment
        depth_ref.attachment = attachment_index;
        depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }else{  // color attachment
        VkAttachmentReference ref = {};
        ref.attachment = attachment_index;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(ref);
    }
}
// ----------------------------------Renderpass---------------------------------
CRenderpass::CRenderpass(VkDevice device) : device(device), renderpass() {}
CRenderpass::~CRenderpass() {Destroy();}
/*
//------------------scheme A------------------
void CRenderpass::AddSubpasses(uint32_t count){
    repeat(count) subpasses.push_back(CSubpass(*this));
};

uint32_t CRenderpass::AddColorAttachment(VkFormat format, vector<uint32_t> subpass_indexes){
  attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
  uint32_t index = attachments.size()-1;
  for(uint32_t i : subpass_indexes)  subpasses.at(i).UseColorAttachment(index);
  return index;
}

uint32_t CRenderpass::AddDepthAttachment(VkFormat format, vector<uint32_t> subpass_indexes){
  attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
  uint32_t index = attachments.size()-1;
  for(uint32_t i : subpass_indexes)  subpasses.at(i).UseDepthAttachment(index);
  return index;
}
//-------------------------------------------
*/
//------------------scheme B-----------------
uint32_t CRenderpass::AddColorAttachment(VkFormat format){
    attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
    return attachments.size()-1;
}

uint32_t CRenderpass::AddDepthAttachment(VkFormat format){
    attachments.push_back(Attachment(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
    return attachments.size()-1;
}

//CSubpass& CRenderpass::AddSubpass(){
//    subpasses.push_back(CSubpass(*this));
//    return subpasses.back();
//}

CSubpass& CRenderpass::AddSubpass(vector<uint32_t> attachment_indexes){
    subpasses.push_back(CSubpass(*this));
    CSubpass& subpass = subpasses.back();
    for(uint32_t i : attachment_indexes) subpass.UseAttachment(i);
    return subpass;
}
//-------------------------------------------

void CRenderpass::Create(){
    std::vector<VkSubpassDescription> subs;
    for(auto sub : subpasses) subs.push_back(sub);  // build list of subpasses

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = NULL;
    rp_info.flags = 0;
    rp_info.attachmentCount = attachments.size();
    rp_info.pAttachments    = attachments.data();
    rp_info.subpassCount    = subs.size();
    rp_info.pSubpasses      = subs.data();
    rp_info.dependencyCount = 0;
    rp_info.pDependencies   = NULL;
    VKERRCHECK(vkCreateRenderPass(device, &rp_info, NULL, &renderpass));
}

void CRenderpass::Destroy(){
    if(renderpass) vkDestroyRenderPass(device, renderpass, nullptr);
}

// -----------------------------------------------------------------------------










/*

void demo_prepare_render_pass(struct demo *demo) {
    // The initial layout for the color and depth attachments will be LAYOUT_UNDEFINED
    // because at the start of the renderpass, we don't care about their contents.
    // At the start of the subpass, the color attachment's layout will be transitioned
    // to LAYOUT_COLOR_ATTACHMENT_OPTIMAL and the depth stencil attachment's layout
    // will be transitioned to LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of
    // the renderpass, the color attachment's layout will be transitioned to
    // LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part of
    // the renderpass, no barriers are necessary.

    VkAttachmentDescription attachments[2] = {};
    attachments[0].format = demo->format;
    attachments[0].flags = 0;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = demo->depth.format;
    attachments[1].flags = 0;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depth_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = NULL;
    rp_info.flags = 0;
    rp_info.attachmentCount = 2;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 0;
    rp_info.pDependencies = NULL;

    VkResult err = vkCreateRenderPass(demo->device, &rp_info, NULL, &demo->render_pass);
    assert(!err);
}
*/


/*
void CTriangle::CreateRenderPass(VkFormat swapchainImageFormat) {
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
