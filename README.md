*Note*: This repository is outdated and no longer maintained.  

A new and improved version can be found in the [GWindow](https://github.com/renelindsay/GWindow) repository.
GWindow adds many new features, such as gamepad, fullscreen, clipboard and mouse cursor support.
Also, there is now an stb-style single-header-file version for easier integration.
Vulkan code was separated out into the VkUtils library. (See the [vkSamples](https://github.com/renelindsay/vkSamples) repository.)

---

![Renegade Logo](./WSIWindow/docs/vulkan-wsiwindow.png "WSI-Window for Vulkan")

# WSI-Window

> WSI-Window provides a simple cross-platform interface for creating a Vulkan window in C++.
> It also handles keyboard, mouse and touch-screen input, using query or event handler functions.  Its goal is to take care of all the platform-specific complexities of setting up a Vulkan environment, so you can quickly get started on writing great Vulkan code. :)

## Supported platforms

![Renegade Logo](./WSIWindow/docs/platforms.png "Platforms")

| Platform         | Build Status                                                                                                                                                                   |
|:----------------:|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|
| Windows          | [![Windows Build status](https://ci.appveyor.com/api/projects/status/1kng5mnvn6ojvlh8?svg=true)](https://ci.appveyor.com/project/renelindsay/wsiwindow "Windows Build Status") |
| Linux (XLib-XCB) | [![Linux Build Status](https://travis-ci.org/renelindsay/Vulkan-WSIWindow.svg?branch=master)](https://travis-ci.org/renelindsay/Vulkan-WSIWindow "Linux Build Status")         |
| Android          | [![CircleCI](https://circleci.com/gh/renelindsay/Vulkan-WSIWindow.svg?style=shield)](https://circleci.com/gh/renelindsay/Vulkan-WSIWindow)                                     |

#### Todo (Contributions welcome)

- Apple OS X / iOS
- Linux Wayland

## Features

- Create a Vulkan window.
- Create a Vulkan instance.
- Load Layers and Extensions.
- Print Vulkan Validation Debug Reports.
- Create Logical device and queues.
- Mouse input.
- Keyboard input. (keycodes or localized text)
- Window management.
- Multi-touch input.

#### Todo (Contributions welcome)

- Full screen mode
- Sensors input? (Android)
- Joystick input?

Please see the [BUILD.md](BUILD.md) document for details on how to build WSI-Window for your target platform.

## Overview

WSI-Window has a very modular design. It consists of a few loosely coupled classes, which abstracts away much of the tedious, platform-specific boiler-plate code, when creating a Vulkan application.  For example, WSI-Window requires a valid VkInstance as input, when creating a VkSurfaceKHR, which you can get from the optional CInstance class.  Alternatively, you may create the VkInstance yourself, by calling Vulkan directly. However, if you do, it is up to you to set up Validation, and ensure the correct layers and extensions are loaded for your target platforms.

![WSI-Window diagram](./WSIWindow/docs/WSIWindow.png "WSI-Window")

## Classes

### WSIWindow class

The WSIWindow class creates a Vulkan window, and provides function calls to query keyboard and mouse state, as well as callbacks, to notify you of system events. (window / keyboard / mouse / touch-screen)
WSIWindow provides member functions for setting the window width, height, position and title.  These dimensions only apply to Linux and Windows, but are ignored on Android.
However, right after window creation, the OnResizeEvent callback will be triggered, to return the actual window dimensions. 

The "GetSurface()" member function takes a VkInstance as input (from CInstance), and returns a CSurface instance, which contains the VkSurfaceKHR of the window.  CSurface also provides the CanPresent() funtion, which wraps the `vkGetPhysicalDeviceSurfaceSupportKHR` function. When creating a Vulkan queue, use CanPresent() to check if the queue family can present to this surface.
Alternatively, WSIWindow also contains a similar CanPresent() member function, which wraps the set of `vkGetPhysicalDevice***PresentationSupportKHR` funcions, and can be used to check queue compatibility BEFORE creating the VkSurfaceKHR.  

#### The following query functions are provided:

- `GetWinPos . :` Get the window's current position, relative to the top-left corner of the display  
- `GetWinSize. :` Get the window's current width and height. (Client-area only, not including title-bar or borders.)
- `GetKeyState :` Get the current state of the specified keyboard key. (see "keycodes.h" for a list of key codes.)  
- `GetBtnState :` Get the state of the specified mouse button (1-3)  
- `GetMousePos :` Get the current mouse position (x,y) within this window.  
- `GetSurface. :` Returns CSurface, which contains the VkSurfaceKHR, and 'CanPresent()' function.

#### The following control functions are provided:

- `SetTitle . . . .:` Set window title.
- `SetWinPos. . . .:` Set window position.
- `SetWinSize . . .:` Set window size.
- `ShowKeyboard. . :` On Android, show the Soft-keyboard.
- `Close . . . . . :` Close the window.

#### Use one of the following functions for the main message-loop:

- `GetEvent . . .:` Returns one event from the message queue, for processing.
- `ProcessEvents :` Fetch all events from the message queue, and dispatch to event handlers.

#### 'ProcessEvents' may trigger the following event handler callbacks:

- `OnMouseEvent :` Mouse movement and button clicks
- `OnKeyEvent . :` Keyboard key-press and key-release events
- `OnTextEvent. :` Keyboard Text input, using OS keyboard layout and language settings.
- `OnMoveEvent. :` Window move events
- `OnResizeEvent:` Window resize events
- `OnFocusEvent :` Window gained / lost focus
- `OnTouchEvent :` Touch-screen events, tracking up to 10 fingers.

### CInstance class

The CInstance class creates a VkInstance, and loads appropriate layers and platform-specific WSI Surface extensions.  
CInstance may be passed to any vulkan function that expects a VkInstance.
If the "enable_validation" constructor parameter is set (default), Standard validation layers are loaded.

Also, the following extensions are loaded where available:  

> `VK_KHR_surface . . . . ` (On all platforms)  
> `VK_KHR_win32_surface . ` (On Windows)  
> `VK_KHR_xcb_surface . . ` (On Linux)  
> `VK_KHR_android_surface ` (On Android)  
> `VK_KHR_debug_report. . ` (When validation is enabled)   

If you need direct control over which layers and extensions to load, use the CLayers and CExtensions classes to enumerate, display and pick the items you want, and then pass them to the CInstance constructor.
The VkInstance is used in 2 places: Pass it to WSIWindow.GetSurface() to get the VkSurfaceKHR, and pass it to CPhysicalDevices,
to enumerate the available GPUs.

### CLayers class

The CLayers class wraps "vkEnumerateInstanceLayerProperties" to simplify enumerating, and picking instance layers to load.  On creation, it contains a list of available instance layers, and provides functions for picking which ones to load. Here are some of the useful functions:  

- ` Clear . :` Clear the picklist.  
- ` Pick . .:` Add one or more named items to the picklist. eg. layers.Pick({"layer1","layer2"});  
- ` PickAll :` Adds all available layers to the picklist.  
- ` PickList:` Returns the picklist as an array of layer names, which can be passed to CInstance.  
- ` Print . :` Prints the list of available layers, with a tick next to the ones what have been picked.

### CExtensions class

The CExtensions class wraps "vkEnumerateInstanceExtensionProperties" in much the same way as CLayers wraps the layers.
It provides the same functions as CLayers, for picking  extensions to load, and may also be passed to the CInstance constructor.

### Vulkan Validation Layers and Logging

WSIWindow makes use of Validation Layers, via the VK_KHR_debug_report extension, to display helpful, color-coded log messages, when Vulkan is used incorrectly. (Errors / Warnings / Info / etc.)  By default, WSIWindow enables standard validation layers, but they may be turned off for better runtime performance. There's also a CMAKE option to remove Validation code from the executable, for even faster execution, but it is highly recommended that you make use of Validation during development.  

The LOG functions may be used in the same way as "printf", but with some advantages:
On desktop, LOG messages are color-coded, for better readability, and on Android, they are forwarded to Android Studio's logcat facility.  Log messages can be easily stripped out, by turning off the "ENABLE_LOGGING" flag. This will reduce clutter, and keep the executable as small as possible.  
Here are some examples of LOG* message usage:

        LOGE("Error message\n");    // Errors are printed in red
        LOGW("Warning message\n");  // Warnings are printed in yellow
        LOGI("Info message\n");     // Info is printed in green

*(See Validation.h for more..)*  
On Desktop, Validation layers may be disabled by unselecting the "ENABLE_VALIDATION" option in cmake-gui, or QtCreator -> Projects.  On Android Studio, the option is under: Build -> Select Build Variant -> noValidateDebug.

### CPhysicalDevices class

The CPhysicalDevices class wraps an array of VkPhysicalDevice objects, and is used to enumerate the available GPUs, and their properties, features and available queues.  It requires a VkInstance as input, which you can acquire from the CInstance class.
Use the FindPresentable() member function, to find which CPU can present to a given window surface (VkSurfaceKHR), and use it to create a Logical device instance. (CDevice)

### CDevice class

The CDevice class takes the chosen GPU (CPhysicalDevice) from CPhysicalDevices, and allows you to create one or more queues of specified types, using the AddQueue() function. Optionally, you can pass in a VkSurfaceKHR to this function, if you want the queue to be presentable.  Available queue types is system specific, and AddQueue() returns 0 if the current system is unable to create a queue of the specified type, in which a case you may have to fall back to an alternative queue configuration.  
eg. If AddQueue() fails to create a Presentable Graphics queue, you may have to create separate queues for graphics and presentation.

## Examples

### Example 1: Create a Vulkan instance, with default layers and extensions:

        #include "WSIWindow.h"
    
        int main(){
            CInstance Inst(true);        // Create a Vulkan Instance, loading default layers and extensions
            VkInstance vkInst = Inst;    // Get the raw VkInstance
            return 0;                    // Exit
        }

### Example 2: List and pick specific layers and extensions to load:

        #include "WSIWindow.h"
    
        int main(){
            CLayers layers;                                       // Create layers pick-list
            layers.Pick({"VK_LAYER_LUNARG_parameter_validation",
                         "VK_LAYER_LUNARG_object_tracker",
                         "VK_LAYER_LUNARG_core_validation"});     // Pick three validation layers to load
            layers.Print();                                       // Display layer list...
                                                                  // (Picked items are ticked.)
            CExtensions extensions;                               // Create extensions pick-list
            extensions.PickAll();                                 // Pick all available extensions
            extensions.UnPick("VK_KHR_xlib_surface");             // ...except this one.
            extensions.Print();                                   // Display extension list
    
            CInstance Inst(layers, extensions);                   // Create VkInstance and load picked items
            return 0;                                             // Exit
        }

#### Output:

*(Notice the ticks next to picked items.  Available items may vary, depending on your setup.)*

    Layers picked: 3 of 11
        ✓ VK_LAYER_LUNARG_core_validation
          VK_LAYER_LUNARG_vktrace
        ✓ VK_LAYER_LUNARG_object_tracker
          VK_LAYER_LUNARG_screenshot
          VK_LAYER_GOOGLE_threading
          VK_LAYER_GOOGLE_unique_objects
          VK_LAYER_LUNARG_swapchain
        ✓ VK_LAYER_LUNARG_parameter_validation
          VK_LAYER_LUNARG_api_dump
          VK_LAYER_LUNARG_standard_validation
    Extensions picked: 3 of 4
        ✓ VK_KHR_surface
        ✓ VK_KHR_xcb_surface
          VK_KHR_xlib_surface
        ✓ VK_EXT_debug_report

### Example 3: Create a Vulkan window and surface.

        #include "WSIWindow.h"
    
        int main(){
            CInstance Inst;                                // Create a Vulkan Instance
            WSIWindow Window("Vulkan", 640, 480);          // Create a window, setting title and size.
            VkSurfaceKHR surface=Window.GetSurface(Inst);  // Get the Vulkan surface
            while(Window.ProcessEvents()){ }               // Run message-loop until window is closed
            return 0;
        }

### Example 4: Query the state of a keyboard key

        #include "WSIWindow.h"
    
        int main(){
            CInstance Inst;                                           // Create a Vulkan Instance
            WSIWindow Window("Vulkan",640, 480);                      // Create a Vulkan window
            VkSurfaceKHR surface=Window.GetSurface(Inst);             // Get the Vulkan surface
            while(Window.ProcessEvents()){                            // Run message-loop
                bool KeyPressed = Window.GetKeyState(KEY_LeftShift);  // Get state of a key. (see keycodes.h)
                if (KeyPressed) printf("LEFT-SHIFT is pressed\r");
            }
            return 0;
        }

### Example 5: Use event handlers to react to input events (mouse / keyboard / etc.)

> To get notified of system events in your Vulkan window, derive a new class from WSIWindow,  
> and override the virtual functions for the appropriate events. (see WSIWindow.h)  

        #include "WSIWindow.h"
    
        const char* type[]{"up  ", "down", "move"};
    
        class MyWindow : public WSIWindow{
            //--Mouse event handler--
            void OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn){
                printf("Mouse: %s %d x %d Btn:%d\n", type[action], x, y, btn);
            }
    
            //--Keyboard event handler--
            void OnKeyEvent(eAction action, uint8_t keycode){
                printf("Key: %s keycode:%d\n", type[action], keycode);
            }
    
            //--Text typed event handler--
            void OnTextEvent(const char* str){
                printf("Text: %s\n", str);
            }
    
            //--Window resize event handler--
            void OnResizeEvent(uint16_t width, uint16_t height){
                printf("Window Resize: width=%4d height=%4d\n", width, height);
            }
        };
    
        int main(){
            CInstance Inst;                                  // Create a Vulkan Instance
            MyWindow Window;                                 // Create a window
            Window.SetTitle("Vulkan");                       // Set window title
            Window.SetWinSize(640, 480);                     // Set window size
            VkSurfaceKHR surface = Window.GetSurface(Inst);  // Get the Vulkan surface
            while(Window.ProcessEvents()){ }                 // Run until window is closed
            return 0;
        }
