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
* Spinning cube.
* Spin direction can be adjusted with arrow keys, mouse or touch screen.
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
#include "CShaders.h"

float dx = 0.1f;
float dy = 0.2f;

//-- EVENT HANDLERS --
class CWindow : public WSIWindow {
    float mx = 0;
    float my = 0;

  public:
    void OnResizeEvent(uint16_t width, uint16_t height) {
        printf("OnResizeEvent: %d x %d\n", width, height);
    }
    //  change spin with arrow keys
    void OnKeyEvent(eAction action, eKeycode keycode) {
        if(action==eDOWN) {
            if(keycode == KEY_Left ) dy-=0.1f;
            if(keycode == KEY_Right) dy+=0.1f;
            if(keycode == KEY_Up   ) dx+=0.1f;
            if(keycode == KEY_Down ) dx-=0.1f;
        }
    }

    //  change spin with mouse drag
    void OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {
        if(action==eMOVE && btn==1) { dy = x - mx;  dx = my - y;}
        mx = x; my = y;
    }
    //  change spin with touch screen
    void OnTouchEvent(eAction action, float x, float y, uint8_t id) {
        if(action==eMOVE) { dy = x - mx;  dx = my - y;}
        mx = x; my = y;
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

    //gpu->enabled_features.samplerAnisotropy = VK_TRUE;

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
    struct Vertex {vec3 pos; vec2 tc;};
    const std::vector<Vertex> vertices = {
        //front
        {{-0.5f,-0.5f, 0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
        //back
        {{ 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}},
        {{-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}},
        //left
        {{-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}},
        {{-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}},
        //right
        {{ 0.5f,-0.5f, 0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
        //top
        {{-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,-0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f,-0.5f, 0.5f}, {0.0f, 1.0f}},
        //bottom
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}},
    };
    const std::vector<uint16_t> index = { 
        0, 1, 2,  2, 3, 0, 
        4, 5, 6,  6, 7, 4,
        8, 9,10, 10,11, 8,
       12,13,14, 14,15,12,
       16,17,18, 18,19,16,
       20,21,22, 22,23,20
    };

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
    ubo.Allocate(sizeof(uniforms));
    printf("UBO created\n");
    //----------------------

    // Textures
    CImage img("vulkan.png");
    CvkImage vkImg(allocator);
    VkExtent3D extent = {(uint32_t)img.width, (uint32_t)img.height, 1 };
    vkImg.Data(img.buf, extent, VK_FORMAT_R8G8B8A8_UNORM, true);
    //vkImg.CreateSampler();
    // ---------

    //--
    CShaders shaders(device);
    shaders.LoadVertShader("shaders/vert.spv");
    shaders.LoadFragShader("shaders/frag.spv");
    shaders.Bind("ubo", ubo);
    shaders.Bind("texSampler", vkImg);
    VkDescriptorSet descriptorSet = shaders.CreateDescriptorSet();

    CPipeline pipeline(device, renderpass, shaders);
    pipeline.CreateGraphicsPipeline();
    printf("Pipeline created\n");
    //--


    //--- Main Loop ---
    while (Window.ProcessEvents()) {  // Main event loop, runs until window is closed.
        VkExtent2D ext  = swapchain.GetExtent();
        VkRect2D   scissor = {{0, 0}, ext};
        VkViewport viewport = {0, 0, (float)ext.width, (float)ext.height, 0, 1};

        float aspect = (float)ext.width/(float)ext.height;
        uniforms.proj.SetProjection(aspect, 40.f, 1, 1000);

        uniforms.model.RotateX(dx);
        uniforms.model.RotateY(dy);
        //uniforms.model.RotateZ(1);
        ubo.Update(&uniforms);

        VkCommandBuffer cmd_buf = swapchain.BeginFrame();
            vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdSetViewport(cmd_buf,0,1, &viewport);
            vkCmdSetScissor(cmd_buf,0,1, &scissor);

            VkBuffer vertexBuffers[] = {vbo};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd_buf, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cmd_buf, ibo, 0, VK_INDEX_TYPE_UINT16);
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

            vkCmdDrawIndexed(cmd_buf, ibo.Count(), 1, 0, 0, 0);

        swapchain.EndFrame();
    }
    //-----------------
    return 0;
}
