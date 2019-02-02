/*
*--------------------------------------------------------------------------
* Copyright (c) 2017 Rene Lindsay
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
* This sample project uses the CSwapchain class, to create a swapchain. (WIP)
*
*/

#include "WSIWindow.h"
#include "CDevices.h"
#include "CSwapchain.h"

#include "CRenderpass.h"

#include "triangle.h"

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

    CDevice device(*gpu);                                             // Create Logical device on selected gpu
    CQueue* queue = device.AddQueue(VK_QUEUE_GRAPHICS_BIT, surface);  // Create the present-queue
    CSwapchain swapchain(*queue);

    swapchain.renderpass.AddColorAttachment();
    //swapchain.renderpass.AddDepthAttachment();
    swapchain.renderpass.AddSubpass({0});
    swapchain.Apply();

    CTriangle triangle;
    triangle.device = device;
    //triangle.CreateRenderPass(swapchain.info.imageFormat);
    triangle.renderpass = swapchain.renderpass;
    triangle.CreateGraphicsPipeline(swapchain.GetExtent());
    printf("Pipeline created\n");

    swapchain.SetImageCount(3);
    //swapchain.SetRenderPass(triangle.renderpass);
    swapchain.Print();
//return 0;
    while (Window.ProcessEvents()) {  // Main event loop, runs until window is closed.
        CSwapchainBuffer& buffer = swapchain.AcquireNext();
        triangle.RecordCommandBuffer(buffer);
        swapchain.Present();
    }

    return 0;
}

