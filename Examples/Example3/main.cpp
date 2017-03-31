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

//-- EVENT HANDLERS --
class CWindow : public WSIWindow {

};

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);                      // Prevent printf buffering in QtCreator
    CInstance instance(true);                              // Create a Vulkan Instance
    instance.DebugReport.SetFlags(14);                     // Error+Perf+Warning flags
    CWindow Window;                                        // Create a Vulkan window
    Window.SetTitle("WSI-Window Example3");                // Set the window title
    Window.SetWinSize(500, 500);                           // Set the window size (Desktop)
    Window.SetWinPos(0, 0);                                // Set the window position to top-left
    VkSurfaceKHR surface = Window.GetSurface(instance);    // Create the Vulkan surface
    CPhysicalDevices gpus(instance);                       // Enumerate GPUs, and their properties
    CPhysicalDevice *gpu = gpus.FindPresentable(surface);  // Find which GPU, can present to the given surface.

    gpus.Print();        // List the available GPUs.
    if (!gpu) return 0;  // Exit if no devices can present to the given surface.

    CDevice device(*gpu);                                             // Create Logical device on selected gpu
    CQueue* queue = device.AddQueue(VK_QUEUE_GRAPHICS_BIT, surface);  // Create the present-queue

    CSwapchain swapchain(*gpu, device, surface);
    //swapchain.Apply();
    swapchain.Print();

    while (Window.ProcessEvents()) {  // Main event loop, runs until window is closed.
    }
    return 0;
}
