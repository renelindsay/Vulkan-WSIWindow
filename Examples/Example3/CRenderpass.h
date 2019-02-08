// * Copyright (C) 2017 by Rene Lindsay
//
//      -- Renderpasss structure --
//
//                    +-------------------------+
//                    | VkAttachmentDescription |
//                    +-----+--------------+----+
//                          | 1            V m
//                          ^ m            |
//           +--------------+--------+     |
//           | VkAttachmentReference |     |
//           +--------------+--------+     |
//                          V m            |
//                          | 1            |
//           +--------------+-------+      |
//           | VkSubpassDescription |      |
//           +---------+----+-------+      |
//                     V 2  V m            |
//                     | 1  |              |
//  +------------------+--+ |              |
//  | VkSubpassDependency | |              |
//  +------------------+--+ |              |
//                     V m  |              |
//                     |    |              |
//                     | 1  | 1            | 1
//                   +------+--------------+--+
//                   | VkRenderPassCreateInfo |
//                   +------------------------+
//
// How to use:
//   1: Find which device can present to given surface, and what color/depth formats it supports.
//   2: Create an instance of renderpass. eg: CRenderpass renderpass(device);
//   3: Add Color and Depth-stencil attachments.
//   4: Add subpasses, with an array of attachment indexes, used by that renderpass.
//
// eg:
//    CRenderpass renderpass(device);                              // Create a new renderpass structure.
//    renderpass.AddColorAttachment(VK_FORMAT_B8G8R8A8_UNORM);     // Add a color attachment
//    renderpass.AddDepthAttachment(VK_FORMAT_D24_UNORM_S8_UINT);  // Add a depth-stencil attachment
//    renderpass.AddSubpass({0,1});                                // Create subpass, and link to attachment 0 and 1. (color and depth)
//    renderpass.Create();                                         // Create the VkRenderPass instance. (optional)
//
//    WARNING: Don't make changes to renderpass after passing it to CSwapchain or CPipeline
//
// TODO:
//   Subpass.pResolveAttachments
//   Subpass.pPreserveAttachments


#ifndef CRENDERPASS_H
#define CRENDERPASS_H

#include "WSIWindow.h"

VkFormat GetSupportedDepthFormat(VkPhysicalDevice gpu,
    std::vector<VkFormat> preferred_formats = {VK_FORMAT_D32_SFLOAT_S8_UINT, 
                                               VK_FORMAT_D32_SFLOAT,
                                               VK_FORMAT_D24_UNORM_S8_UINT,
                                               VK_FORMAT_D16_UNORM_S8_UINT,
                                               VK_FORMAT_D16_UNORM});

VkFormat GetSupportedColorFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface, 
    std::vector<VkFormat> preferred_formats = {VK_FORMAT_B8G8R8A8_UNORM, 
                                               VK_FORMAT_R8G8B8A8_UNORM});

class CRenderpass {
    class CSubpass {
        friend class CRenderpass;
        CRenderpass& renderpass;
        std::vector<VkAttachmentReference> input_refs;
        std::vector<VkAttachmentReference> color_refs;
        VkAttachmentReference              depth_ref;
        VkAttachmentReference*             pdepth_ref;  // points to depth_ref, or 0 if none.
        operator VkSubpassDescription();
        CSubpass(CRenderpass& renderpass);
      public:
        void UseAttachment(uint32_t attachment_index);  // for write
        void UseAttachments(vector<uint32_t> attachment_indexes = {});
        void InputAttachment(uint32_t attachment_index);  //for read
        void InputAttachments(vector<uint32_t> attachment_indexes = {});
    };

    VkDevice     device;
    VkRenderPass renderpass;

  public:
    VkFormat     surface_format = VK_FORMAT_UNDEFINED;
    //VkFormat     depth_format;

    std::vector<CSubpass>                subpasses;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency>     dependencies;

    CRenderpass(VkDevice device);
    ~CRenderpass();
    //void Init(VkDevice device, VkFormat surface_format, VkFormat depth_format);

    uint32_t AddColorAttachment(VkFormat format = VK_FORMAT_UNDEFINED, VkImageLayout final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    uint32_t AddDepthAttachment(VkFormat format = VK_FORMAT_UNDEFINED);
    CSubpass& AddSubpass(vector<uint32_t> attachment_indexes = {});
    void AddSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass);

    void Create();
    void Destroy();
    operator VkRenderPass () {
        if(!renderpass) Create();
        //ASSERT(!!renderpass, "Swapchain.renderpass was not initialized. Call: CSwapchain.Apply(); before use.\n");
        return renderpass;
    }
};

#endif
