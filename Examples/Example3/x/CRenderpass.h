// * Copyright (C) 2017 by Rene Lindsay
//
//
//             +-------------------------+
//             | VkAttachmentDescription |
//             +-----+--------------+----+
//                   | 1            V m
//                   ^ m            |
//    +--------------+--------+     |
//    | VkAttachmentReference |     |
//    +--------------+--------+     |
//                   V m            |
//                   | 1            |
//    +--------------+-------+      |
//    | VkSubpassDescription |      |
//    +--------------+-------+      |
//                   V m            |
//                   | 1            | 1
//            +------+--------------+--+
//            | VkRenderPassCreateInfo |
//            +------------------------+
//

#ifndef CRENDERPASS_H
#define CRENDERPASS_H

#include "WSIWindow.h"

class CRenderpass;

class CSubpass{
    friend class CRenderpass;
    CRenderpass& renderpass;
    std::vector<VkAttachmentReference> color_refs;
    VkAttachmentReference depth_ref;
  public:
    CSubpass(CRenderpass& renderpass);
//    uint32_t AddColorAttachment(VkFormat format);
//    uint32_t AddDepthAttachment(VkFormat format);
    //void UseColorAttachment(uint32_t attachment_index);
    //void UseDepthAttachment(uint32_t attachment_index);
    void UseAttachment(uint32_t attachment_index);
    operator VkSubpassDescription();
};


class CRenderpass{
    //friend class CSubpass;
    VkDevice device;
    VkRenderPass renderpass;
  public:
    std::vector<CSubpass>                subpasses;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency>     dependencies;

    CRenderpass(VkDevice device);
    ~CRenderpass();
/*
    // count: number of subpasses to create.
    void AddSubpasses(uint32_t count);
   //CSubpass& AddSubpass();

    // format : The surface format. (Get it from swapchain.info.imageFormat)
    // subpass_indexes: A list of subpass indexes that use this attachment.
    uint32_t AddColorAttachment(VkFormat format, vector<uint32_t> subpass_indexes = {});
    uint32_t AddDepthAttachment(VkFormat format, vector<uint32_t> subpass_indexes = {});
*/
    // ---- B ----
    uint32_t AddColorAttachment(VkFormat format);
    uint32_t AddDepthAttachment(VkFormat format);
    CSubpass& AddSubpass(vector<uint32_t> attachment_indexes = {});


    void Create();
    void Destroy();
    operator VkRenderPass () const { return renderpass; }
};













/*
class CAttachment{
  public:
    CAttachment(VkFormat format);
    VkAttachmentDescription description;
    operator VkAttachmentDescription() { return description; }
};
*/

/*
class CSubpass{
    VkSubpassDescription subpass;
};

class CRenderpass{
    std::vector<VkAttachmentDescription> attach_desc;
    std::vector<VkAttachmentReference>   attach_ref;



    //std::vector<CSubpass>
  public:
    CRenderpass(){}
    ~CRenderpass(){}
    void Create();
    void Destroy();
    void AddAttachment(VkAttachmentDescription attachment, VkImageLayout layout);
    uint32_t AddColorAttachment(VkFormat format);  // color
    uint32_t AddDepthAttachment(VkFormat format);  // depth + stencil

    void AddSubpass(VkSubpassDescription subpass);

};
*/
#endif
