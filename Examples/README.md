# WSI-Window Examples: #

## Example 1:  
Demonstrates the WSI-Window event system. (Windows/Linux/Android)  
Creates a blank window with a Vulkan surface, vkPhysicalDevice, vkDevice and graphics queue.
Sets up event event handlers to react to keyboard, mouse and touch-screen events. 
Also shows how to show/hide the on-screen keyboard when on Android.


## Example 2:  
Renders a spinning cube.   
(This demo is deprecated and will soon be removed.)

## Example 3:
Renders a colored triangle.
Uses the new CRenderpass class, to create a frame buffer with color and depth attachments.
Uses then new CSwapchain class to create a triple buffered  swapchain, with command buffers.  
Uses AMD's Vulkan Memory Allocator, to create vertex and index arrays, using simplified wrapper classes.
Demonstrates how to acquire render and present a frame to the graphics queue inside the main rendering loop.

Note: The CPipeline class (WIP) loads the vertex and fragment shaders and configures the pipeline. 


## Teapots: 
Renders flying teapots.  
(This demo is deprecated and will soon be removed.) 