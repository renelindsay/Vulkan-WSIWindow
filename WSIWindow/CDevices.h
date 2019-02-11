// Copyright (c) 2017 Rene Lindsay

/*
*  This unit wraps Physical devices, Logical devices and Queues.
*  Use CPhysicalDevice.CreateDevice(), to create a logical device, with queues.
*
*  └CPhysicalDevices[]
*   └CPhysicalDevice ----------------------------------> : CDevice
*     ├VkPhysicalDeviceProperties                          └CQueue[]
*     ├VkPhysicalDeviceFeatures
*     ├CDeviceExtensions[]        (Picklist)
*     └CQueueFamily[]             (array)
*
*  WARNING: This unit is a work in progress.
*  Interfaces are experimental and likely to change.
*
* CPhysicalDevices:
* -----------------
* Create an instance of CPhysicalDevices, to enumerate the available GPUs, and their properties.
* Use the FindPresentable() function to find which GPU can present to the given window surface.

* CPhysicalDevice:
* ----------------
* Use FindSurfaceFormat() function to find a supported color format for the given window surface.
* Use FindDepthFormat() function to find a supported depth format.
*
* CDevice:
* --------
* Create an instance of CDevice, using the picked CPhysicalDevice as input.
* Then call the AddQueue() function, to add queues to the logical device.
*
*/

#ifndef CDEVICES_H
#define CDEVICES_H

#include "CInstance.h"
#include "WindowImpl.h"

//------------------------CPhysicalDevice-------------------------
class CPhysicalDevice {
  public:
    CPhysicalDevice();
    const char* VendorName() const;
    // -- Read-only properties --
    VkPhysicalDevice                handle;
    VkPhysicalDeviceProperties      properties;      // properties and limits
    VkPhysicalDeviceFeatures        features;        // list of available features
    vector<VkQueueFamilyProperties> queue_families;  // array of queue families
    // VkSurfaceCapabilitiesKHR   surface_caps;
    // -- Configurable properties --
    CDeviceExtensions        extensions;             // picklist: select extensions to load (Defaults to "VK_KHR_swapchain" only.)
    VkPhysicalDeviceFeatures enabled_features = {};  // Set required features.   TODO: finish this.

    operator VkPhysicalDevice() const { return handle; }
    int FindQueueFamily(VkQueueFlags flags, VkSurfaceKHR surface = 0);  // Returns a QueueFamlyIndex, or -1 if none found.

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(VkSurfaceKHR surface);     // Returns list of supported surface formats.
    VkFormat FindSurfaceFormat(VkSurfaceKHR surface,                          // Returns first supported format from given list,
        std::vector<VkFormat> preferred_formats = {VK_FORMAT_B8G8R8A8_UNORM,  // or VK_FORMAT_UNDEFINED if no match was found.
                                                   VK_FORMAT_R8G8B8A8_UNORM});
    VkFormat FindDepthFormat(
        std::vector<VkFormat> preferred_formats = {VK_FORMAT_D32_SFLOAT,          // Returns first supported depth format from list,
                                                   VK_FORMAT_D32_SFLOAT_S8_UINT,  // or VK_FORMAT_UNDEFINED if no match was found.
                                                   VK_FORMAT_D24_UNORM_S8_UINT,
                                                   VK_FORMAT_D16_UNORM_S8_UINT,
                                                   VK_FORMAT_D16_UNORM});
};
//----------------------------------------------------------------
//------------------------CPhysicalDevices------------------------
class CPhysicalDevices {
    vector<CPhysicalDevice> gpu_list;

   public:
    CPhysicalDevices(const VkInstance instance);
    uint32_t Count() { return (uint32_t)gpu_list.size(); }
    CPhysicalDevice* FindPresentable(VkSurfaceKHR surface);  // Returns first device able to present to surface, or null if none.
    CPhysicalDevice& operator[](const int i) { return gpu_list[i]; }
    void Print(bool show_queues = false);
};
//----------------------------------------------------------------
//-----------------------------CQueue-----------------------------
struct CQueue{
    VkQueue         handle;
    uint            family;   // queue family
    uint            index;    // queue index
    VkQueueFlags    flags;    // Graphics / Compute / Transfer / Sparse / Protected
    VkSurfaceKHR    surface;  // 0 if queue can not present
    VkDevice        device;   // (used by CSwapchain)
    CPhysicalDevice gpu;      // (used by CSwapchain)

    operator VkQueue() const { return handle; }
};
//----------------------------------------------------------------
//-----------------------------CDevice----------------------------
class CDevice {
    //friend class CSwapchain;
    VkDevice        handle;
    CPhysicalDevice gpu;
    vector<CQueue>  queues;
    uint FamilyQueueCount(uint family);
    void Create();
    void Destroy();

   public:
    CDevice(CPhysicalDevice gpu);
    ~CDevice();
    CQueue* AddQueue(VkQueueFlags flags, VkSurfaceKHR surface = 0);  // returns 0 if failed
    operator VkDevice() const { return handle; }
};
//----------------------------------------------------------------


#endif
