/*
*--------------------------------------------------------------------------
* Copyright (c) 2019 Rene Lindsay
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Author: Rene Lindsay <rjklindsay@gmail.com>
*
*--------------------------------------------------------------------------
*
*
*/

#include "WSIWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"
#include "CSwapchain.h"
#include "CPipeline.h"
#include "Buffers.h"
#include "CImage.h"
#include "matrix.h"
#include "CDescriptor.h"
#include "CShaders.h"


//-- EVENT HANDLERS --
class CWindow : public WSIWindow {
  public:
    void OnResizeEvent(uint16_t width, uint16_t height) {
        //printf("OnResizeEvent: %d x %d\n", width, height);
    }
};

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);                      // Prevent printf buffering in QtCreator
    CInstance instance(true);                              // Create a Vulkan Instance
    instance.DebugReport.SetFlags(14);                     // Error+Perf+Warning flags
    CWindow Window;                                        // Create a Vulkan window
    Window.SetTitle("WSI-Window Example4");                // Set the window title
    Window.SetWinSize(640, 480);                           // Set the window size (Desktop)
    Window.SetWinPos(0, 0);                                // Set the window position to top-left
    VkSurfaceKHR surface = Window.GetSurface(instance);    // Create the Vulkan surface
    CPhysicalDevices gpus(instance);                       // Enumerate GPUs, and their properties
    CPhysicalDevice *gpu = gpus.FindPresentable(surface);  // Find which GPU, can present to the given surface.

    gpus.Print();        // List the available GPUs.
    if (!gpu) return 0;  // Exit if no devices can present to the given surface.

    //--- Device and Queues ---
    CDevice device(*gpu);                                                      // Create Logical device on selected gpu

    CQueue* present_queue  = device.AddQueue(VK_QUEUE_GRAPHICS_BIT, surface);  // Create a graphics + present-queue
    CQueue* graphics_queue = present_queue;                                    // If possible use same queue for both
    if(!present_queue) {                                                       // If not, create separate queues
        present_queue  = device.AddQueue(0, surface);                          // Create present-queue
        graphics_queue = device.AddQueue(VK_QUEUE_GRAPHICS_BIT);               // Create graphics queue
    }
    //-------------------------

    //--- Renderpass ---
    VkFormat color_fmt = gpu->FindSurfaceFormat(surface);
    VkFormat depth_fmt = gpu->FindDepthFormat();
    CRenderpass renderpass(device);
    renderpass.AddColorAttachment(color_fmt, {0.0f, 0.0f, 0.3f, 1.0f});  // color buffer, clear to blue
    renderpass.AddDepthAttachment(depth_fmt);
    renderpass.AddSubpass({0,1});
    //-------------------

    //--- Swapchain ---
    CSwapchain swapchain(renderpass, present_queue, graphics_queue);
    swapchain.SetImageCount(3);  // use tripple-buffering
    swapchain.Print();
    //-----------------

    //--- Buffers ---
    struct Vertex {vec3 pos; vec3 color; vec2 tc;};
    const std::vector<Vertex> vertices = {
        {{-0.5f,-0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,-0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };
    const std::vector<uint16_t> index = { 0, 1, 2,  2, 3, 0 };

    struct Uniforms {
        mat4 model;
        mat4 view;
        mat4 proj;
    } uniforms;

    uniforms.view.Translate(0,0,-4);

    CAllocator allocator(*graphics_queue);                                        // Create "Vulkan Memory Aloocator"
    printf("Allocator created\n");

    // Vertex Buffer Object
    CVBO vbo(allocator);                                                          // Create vertex buffer
    vbo.Data((void*)vertices.data(), (uint32_t)vertices.size(), sizeof(Vertex));  // load vertex data
    printf("VBO created\n");

    // Index Buffer Object
    CIBO ibo(allocator);
    ibo.Data(index.data(), (uint32_t)index.size());
    printf("IBO created\n");
    //---------------

    // Uniform Buffer Object
    CUBO ubo(allocator);
    //ubo.Data(&uniforms, sizeof(uniforms));
    ubo.Allocate(sizeof(uniforms));
    //ubo.Update(&uniforms);
    printf("UBO created\n");
    //----------------------

    // Textures
    CImage img("vulkan.png");
    CvkImage vkImg(allocator);
    VkExtent3D extent = {(uint32_t)img.width, (uint32_t)img.height, 1 };
    vkImg.Data(img.buf, extent);
    vkImg.CreateSampler();
    // ---------


/*
    //--- Descriptor ---
    CDescriptors descriptor(device);
    descriptor.CreateDescriptorSetLayout();
    descriptor.CreateDescriptorPool();
    descriptor.CreateDescriptorSet(ubo, ubo.size(), vkImg.view, vkImg.sampler);

    VkDescriptorSet* set = descriptor.getDescriptorSet();
    //------------------

    //--- Pipeline ---
    CPipeline pipeline(device, renderpass);
    pipeline.LoadVertShader("shaders/vert.spv");
    pipeline.LoadFragShader("shaders/frag.spv");
    //pipeline.CreateDescriptorSetLayout();
    pipeline.DescriptorSetLayout(descriptor);
    pipeline.CreateGraphicsPipeline(swapchain.GetExtent());
    printf("Pipeline created\n");
    //----------------
*/



    //--
    CShaders shaders(device);
    shaders.LoadVertShader("shaders/vert.spv");
    shaders.LoadFragShader("shaders/frag.spv");
    shaders.CreateDescriptorSetLayout();
    shaders.CreateDescriptorPool();
    shaders.Bind("ubo", ubo);
    shaders.Bind("texSampler", vkImg.view, vkImg.sampler);
    VkDescriptorSet descriptorSet = shaders.CreateDescriptorSet();
    //VkDescriptorSet* set = &descriptorSet;

    CPipeline2 pipeline2(device, renderpass, shaders);
    pipeline2.CreateGraphicsPipeline();
    //--


    //--- Main Loop ---
    while (Window.ProcessEvents()) {  // Main event loop, runs until window is closed.

        VkExtent2D ext  = swapchain.GetExtent();
        VkRect2D   scissor = {{0, 0}, ext};
        VkViewport viewport = {0, 0, (float)ext.width, (float)ext.height, 0, 1};

        float aspect = (float)ext.width/(float)ext.height;
        uniforms.proj.SetProjection(aspect, 40.f, 1, 1000);

        uniforms.model.RotateZ(1);
        ubo.Update(&uniforms);         // memcpy(ubo.mapped, &uniforms, ubo.size());

        VkCommandBuffer cmd_buf = swapchain.BeginFrame();
            vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline2);
            vkCmdSetViewport(cmd_buf,0,1, &viewport);
            vkCmdSetScissor(cmd_buf,0,1, &scissor);

            VkBuffer vertexBuffers[] = {vbo};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd_buf, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cmd_buf, ibo, 0, VK_INDEX_TYPE_UINT16);

            //vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, set, 0, nullptr);
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline2.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

            vkCmdDrawIndexed(cmd_buf, ibo.Count(), 1, 0, 0, 0);

        swapchain.EndFrame();
    }
    //-----------------
    return 0;
}
