#ifndef CDEVICES_H
#define CDEVICES_H

#include "CInstance.h"

//-------------------------CQueueFamilies-------------------------

//struct CQueueFamily{
//    VkQueueFamilyProperties properties;
//    void Print();
//};


class CQueueFamilies{
    friend class CDevices;
    //void Init(VkPhysicalDevice gpu);
    vector<VkQueueFamilyProperties> family_list;
public:
    uint32_t Count(){return (uint32_t) family_list.size();}
    VkQueueFamilyProperties operator [](const int i) const { return family_list[i]; }
    void Print();
};


//------------------------CPhysicalDevice-------------------------

struct CPhysicalDevice{
    const char* VendorName();
    VkPhysicalDevice           handle;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures   features;
    CQueueFamilies             queue_families; // array
    CDeviceExtensions          extensions;     // picklist

    VkDevice CreateDevice();  // Create logical device
};
//----------------------------------------------------------------

class CDevices{
    vector<CPhysicalDevice> device_list;
public:
    CDevices(VkInstance instance);
    uint32_t Count(){return (uint32_t)device_list.size();}
    CPhysicalDevice operator [](const int i) const { return device_list[i]; }
    void Print(bool show_queues=false);
};




#endif
