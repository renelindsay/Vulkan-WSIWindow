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
*  Interfaces are highly experimental and very likely to change.
*
* CPhysicalDevices:
* -----------------
* Create an instance of CPhysicalDevices, to enumerate the available GPUs, and their properties.
* Use the FindPresentable() function to find which CPU can present to a given window surface.
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
    struct CQueueFamily {
        bool presentable = false;
        VkQueueFamilyProperties properties;
        void Print(uint index);
    };

  public:
    CPhysicalDevice();
    const char* VendorName() const;
    VkPhysicalDevice           handle;          // (RO)
    VkPhysicalDeviceProperties properties;      // properties and limits (RO)
    VkPhysicalDeviceFeatures   features;        // list of available features (RO)
    bool                       presentable;     // has presentable queues (RO)
    vector<CQueueFamily>       queue_families;  // array of queue families (RO)

    CDeviceExtensions        extensions;             // picklist
    VkPhysicalDeviceFeatures enabled_features = {};  // Set required features.   TODO: finish this.
    // VkSurfaceCapabilitiesKHR   surface_caps;

    operator VkPhysicalDevice() const { return handle; }
    int FindQueueFamily(VkQueueFlags flags, bool presentable = false);  // Returns a QueueFamlyIndex, or -1 if none found.
};
//----------------------------------------------------------------
//------------------------CPhysicalDevices------------------------
class CPhysicalDevices {
    vector<CPhysicalDevice> gpu_list;

   public:
    CPhysicalDevices(const CSurface& surface);
    uint32_t Count() { return (uint32_t)gpu_list.size(); }
    CPhysicalDevice* FindPresentable();  // Returns first device which is able to present to the given surface, or null if none.
    CPhysicalDevice& operator[](const int i) { return gpu_list[i]; }
    void Print(bool show_queues = false);
};
//----------------------------------------------------------------
//-----------------------------CQueue-----------------------------
struct CQueue{
    VkQueue      handle;
    uint         family;
    uint         index;
    VkQueueFlags flags;
    bool         presentable;
    operator VkQueue() const { return handle; }
};
//----------------------------------------------------------------
//-----------------------------CDevice----------------------------
class CDevice {
    VkDevice        handle;
    CPhysicalDevice gpu;
    vector<CQueue>  queues;
    uint FamilyQueueCount(uint family);
    void Create();
    void Destroy();

   public:
    CDevice(CPhysicalDevice gpu) : handle() { this->gpu = gpu; }
    ~CDevice() { Destroy(); }
    CQueue* AddQueue(VkQueueFlags flags, bool presentable = false);  // returns 0 if failed
};
//----------------------------------------------------------------


#endif
