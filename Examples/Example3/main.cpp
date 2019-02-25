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
* This example project renders a triangle using 3 Vulkan wrapper classes:
*   CRenderpass contains the subpasses and is used to set up the color and depth buffers.
*   CPipeline loads the shaders and configures the graphics pipeline.
*   CSwapchain manages the frame/command buffers and presents the result to the window surface.
*
*/

#include "WSIWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"
#include "CSwapchain.h"
#include "CPipeline.h"
#include "Buffers.h"

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
    Window.SetTitle("WSI-Window Example3");                // Set the window title
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

    //--- Pipeline ---
    CPipeline pipeline(device, renderpass);
    pipeline.LoadVertShader("shaders/vert.spv");
    pipeline.LoadFragShader("shaders/frag.spv");
    pipeline.CreateGraphicsPipeline(swapchain.GetExtent());
    printf("Pipeline created\n");
    //----------------

    //--- Buffers ---
    // vertex/index arrays
    struct vec2{float x, y;};
    struct vec3{float x, y, z;};
    struct Vertex {vec2 pos; vec3 color;};
    const std::vector<Vertex> vertices = {
        {{ 0.0f,-0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    const std::vector<uint16_t> index = { 0,1,2 };

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

    //--- Main Loop ---
    while (Window.ProcessEvents()) {  // Main event loop, runs until window is closed.
        VkCommandBuffer cmd_buf = swapchain.BeginFrame();
            vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
//            vkCmdDraw(cmd_buf, 3, 1, 0, 0);

            VkBuffer vertexBuffers[] = {vbo};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd_buf, 0, 1, vertexBuffers, offsets);
            //vkCmdDraw(cmd_buf, vbo.Count(), 1, 0, 0);

            vkCmdBindIndexBuffer(cmd_buf, ibo, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(cmd_buf, ibo.Count(), 1, 0, 0, 0);

        swapchain.EndFrame();
    }
    //-----------------
    return 0;
}
