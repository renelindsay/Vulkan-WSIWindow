/*
*  This unit wraps Physical devices, Logical devices and Queues.
*
*  WARNING: This unit is a work in progress.
*  Interfaces are highly experimental and very likely to change.
*/

#ifndef CDEVICES_H
#define CDEVICES_H

#include "CInstance.h"
#include "WindowImpl.h"


//-------------------------CQueueFamily---------------------------
class CQueueFamily{
    friend class CDevices;
    friend class CPhysicalDevice;
    VkQueueFamilyProperties properties;
    bool                    presentable = false;
    uint                    pick_count = 0;
  public:
    bool IsPresentable(){ return presentable; }
    operator VkQueueFamilyProperties() const { return properties; }
    uint Pick(uint count);
    bool Has(VkQueueFlags flags){ return ((properties.queueFlags&flags) == flags); }
};
//----------------------------------------------------------------
//-------------------------CQueueFamilies-------------------------
class CQueueFamilies{
    friend class CDevices;
    friend class CPhysicalDevice;
    vector<CQueueFamily> family_list;
public:
    uint32_t Count(){return (uint32_t) family_list.size();}
    CQueueFamily& operator [](const int i) { return family_list[i]; }
    void Print();

    int Find(VkQueueFlags flags);
    int FindPresentable();
    bool Pick(uint presentable=1, uint graphics=1, uint compute=0, uint transfer=0);
};
//----------------------------------------------------------------
//-----------------------------CQueue-----------------------------
struct CQueue{
    VkQueue      handle;
    uint         family;
    uint         index;
    VkQueueFlags flags;
    bool presentable;
    operator VkQueue () const { return handle; }
};
//----------------------------------------------------------------
//----------------------------CDevice-----------------------------

struct CDevice{  //Logical device
    VkDevice handle = 0;
    operator VkDevice () const { return handle; }

    //CQueue present_queue;
    //vector<VkQueue> graphics_queues;
    //vector<VkQueue> compute_queues;
    //vector<VkQueue> transfer_queues;

    vector<CQueue> queues;

    ~CDevice(){
        LOGI("Logical device destroyed\n");
        vkDeviceWaitIdle(handle);
        vkDestroyDevice(handle, nullptr);
    }

    void Print();
};
//----------------------------------------------------------------
//------------------------CPhysicalDevice-------------------------

struct CPhysicalDevice{
    CPhysicalDevice();
    const char* VendorName() const;
    VkPhysicalDevice           handle;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures   features;
    CQueueFamilies             queue_families; // array
    CDeviceExtensions          extensions;     // picklist
    bool                       presentable;    // has presentable queues

    operator VkPhysicalDevice () const { return handle; }
    //VkDevice Create();  // Create logical device
    CDevice Create();  // Create logical device

    //CDevice Create(uint presentable=1, uint graphics=1, uint Compute=0, uint transfer=0);  // Create logical device
};
//----------------------------------------------------------------
//----------------------------------------------------------------
class CDevices{
    vector<CPhysicalDevice> device_list;
public:
    CDevices(const CSurface& surface);
    uint32_t Count(){return (uint32_t)device_list.size();}
    CPhysicalDevice& operator [](const int i) { return device_list[i]; }
    void Print(bool show_queues = false);
};
//----------------------------------------------------------------



#endif
