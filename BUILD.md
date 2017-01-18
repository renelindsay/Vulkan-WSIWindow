# Build Instructions

This document contains instructions for building WSI-Window on Linux and Windows.

## Install a Vulkan Driver

Before proceeding, it is strongly recommended that you obtain a Vulkan driver from your graphics hardware vendor and install it.  However, the Intel graphics drivers included with Ubuntu 16.04, do not include the Vulkan ICD. If you have a 4th-gen (Haswell) or newer CPU, you can either compile the [Mesa drivers](https://github.com/mesa3d/mesa) yourself, or install the pre-compiled Mesa Vulkan ICD from the included  [MesaVulkan.zip](MesaVulkan.zip "Intel-Mesa Vulkan ICD for Ubuntu 16.04") file.  

It is also recommended that you install LunarG's Vulkan SDK.  Although WSI-Window and its samples may build and run stand-alone, it does not include the SDK's Validation layers, or SPIR-V compiler.  Installing these, will allow you to modify and re-compile the GLSL shaders, as well as ensure that any code changes are properly validated.

## Platform Setup

The Qt Creator IDE can be used to directly open and configure the CMakeLists.txt project file.  
Alternatively, CMake provides a GUI, to generate project files for other IDE's, such as Visual Studio.

### CMake settings:
 - `ENABLE_VALIDATION :` Enable Vulkan Validation. (Turn this off for Release builds.)
 - `ENABLE_LOGGING . .:` Allow WSIWindow to print log messages to the Terminal, or Android LogCat.
 - `ENABLE_MULTITOUCH :` Enables Multi-touch input, tracking up to 10 finders. Disable, to emulate mouse instead.
 - `USE_VULKAN_WRAPPER:` Builds a dispatch-table, to skip the Loader trampoline-code. (Required for Android)
 - `VULKAN_LOADER . . :` Full path (including filename) of the vulkan loader. (libvulkan.so or vulkan-1.lib).
 - `VULKAN_INCLUDE . .:` Set this to the path of the vulkan.h file.

### Windows
Install the Vulkan SDK, CMake and Visual Studio.  
Use cmake-gui to load the CMakeLists.txt file.  
Configure CMake settings if needed, and generate the Visual Studio project.  
Use Visual Studio to open the generated solution.  
(Alternatively, you may use QtCreator to load and configure the CMakeLists.txt file directly.)
Set Sample1 as the Startup project.  
Compile and run the sample project.

### Linux
A Few headers are required, to compile WSI-Window: XCB for Windowing, XKB for Keyboard input, and XInput2 for Multi-touch(optional).

    sudo apt-get install libx11-xcb-dev libxkbcommon-dev libxi-dev

Use Qt-Creator to load the CMakeLists.txt project file, and tweak CMake settings under "Projects" if needed.  Then compile and run the sample project.  Alternatively, you may use cmake-gui to load CMakeLists.txt, configure settings and generate a project file for your favourite IDE.

CMake configuration may be simplified by setting the VULKAN_SDK environment variable to point to the Vulkan SDK.  
On Ubuntu, this may be done globally by adding the following line (or similar) to your ~/.profile file, and then reboot:  
  
  `export VULKAN_SDK="$HOME/VulkanSDK/1.0.xx.0/x86_64"`
  
### Android (using Ubuntu as host)

Install Android Studio 2.2 or later.
Use the Android SDK Manager to add the NDK and CMake modules. (under SDK Tools)
Use Android Studio -> File -> New -> Import Project... to import the included Android Studio project.
If you see Gradle errors, run the clear.sh script, to delete auto-generated files, and try again.
Connect your device via USB, compile and run the sample project.  

For debugging purposes, "printf" output is routed to Android Studio's Android Monitor -> logcat tab.  

Resource files can be added to your APK, by creating an "Assets" folder in the project's root directory.  
"fopen" will see this Assets folder as its current working directory, but will be in read-only mode.

