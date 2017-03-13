/*
*--------------------------------------------------------------
* Author: Rene Lindsay <rjklindsay@gmail.com>
*--------------------------------------------------------------
*
* This example project uses WSI-Window to create a Vulkan window,
* and then calls a modified version fo LunarG's cube.c, to render a cube.
* The original cube.c was converted to c++, and windowing code was removed. (~1500 lines)
*
* The CDevices.cpp/h unit enumerates your vulkan-capable GPUs (VkPhysicalDevice),
* to find which one can present to the given Vulkan surface (VkSurfaceKHR),
* rather than just picking the first one.
* eg. A PC may have both an integrated and discreet GPU, either of which
* may be used to run the desktop. Typically, only the active GPU is able
* to present to the window surface.
*
* This Example project runs on Windows, Linux and Android.
*
*/

#include "WSIWindow.h"
#include "CDevices.h"
#include "cube.h"

CCube cube;

//-- EVENT HANDLERS --
class CWindow : public WSIWindow {
    void OnResizeEvent(uint16_t width, uint16_t height) { cube.Resize(width, height); }
};

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);                      // Prevent printf buffering in QtCreator
    CInstance instance(true);                              // Create a Vulkan Instance
    instance.DebugReport.SetFlags(14);                     // Error+Perf+Warning flags
    CWindow Window;                                        // Create a Vulkan window
    Window.SetTitle("WSI-Window Example2: cube.c");        // Set the window title
    Window.SetWinSize(500, 500);                           // Set the window size (Desktop)
    Window.SetWinPos(0, 0);                                // Set the window position to top-left
    VkSurfaceKHR surface = Window.GetSurface(instance);    // Create the Vulkan surface
    CPhysicalDevices gpus(instance);                       // Enumerate GPUs, and their properties
    CPhysicalDevice *gpu = gpus.FindPresentable(surface);  // Find which GPU, can present to the given surface.

    gpus.Print();        // List the available GPUs.
    if (!gpu) return 0;  // Exit if no devices can present to the given surface.

    cube.Init(argc, argv);
    cube.InitDevice(*gpu);            // Run cube on given GPU
    cube.InitSwapchain(surface);      // Attach cube demo to wsi-window's surface
    while (Window.ProcessEvents()) {  // Main event loop, runs until window is closed.
        cube.Draw();
    }
    cube.Cleanup();
    return 0;
}
