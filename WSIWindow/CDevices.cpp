﻿#include "CDevices.h"

//-------------------------CQueueFamily---------------------------
void CPhysicalDevice::CQueueFamily::Print(uint index) {
    uint flags = properties.queueFlags;
    printf("\t\tQueue-family:%d  count:%2d  present:%s  flags:[ %s%s%s%s]\n", index, properties.queueCount, presentable ? "Y" : "N",
        (flags & 1) ? "GRAPHICS " : "", (flags & 2) ? "COMPUTE " : "",
        (flags & 4) ? "TRANSFER " : "", (flags & 8) ? "SPARSE "  : "");
}
//----------------------------------------------------------------
//------------------------CPhysicalDevice-------------------------
CPhysicalDevice::CPhysicalDevice() : handle(0), properties(), features(), extensions(), presentable() {}

const char* CPhysicalDevice::VendorName() const {
    struct {const uint id; const char* name;} vendors[] =
    {{0x1002, "AMD"}, {0x10DE, "NVIDIA"}, {0x8086, "INTEL"}, {0x13B5, "ARM"}, {0x5143, "Qualcomm"}, {0x1010, "ImgTec"}};
    for (auto vendor : vendors) if (vendor.id == properties.vendorID) return vendor.name;
    return "";
}

int CPhysicalDevice::FindQueueFamily(VkQueueFlags flags, bool presentable){
    repeat (queue_families.size()) {
        const VkQueueFamilyProperties& family_props = queue_families[i].properties;
        if(presentable && !queue_families[i].presentable) continue;
        if((family_props.queueFlags & flags) == flags) return i;
    }
    return -1;
}
//----------------------------------------------------------------
//------------------------CPhysicalDevices------------------------
CPhysicalDevices::CPhysicalDevices(const CSurface& surface) {
    VkInstance instance = surface.instance;
    VkResult result;
    uint gpu_count = 0;
    vector<VkPhysicalDevice> gpus;
    do {
        result = vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);  // Get number of gpu's
        if (result == VK_SUCCESS && gpu_count > 0) {
            gpus.resize(gpu_count);
            result = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());  // Fetch gpu list
        }
    } while (result == VK_INCOMPLETE);  // If list is incomplete, try again.
    VKERRCHECK(result);
    if (!gpu_count) LOGW("No GPU devices found.");  // Vulkan driver missing?

    gpu_list.resize(gpu_count);
    for (uint i = 0; i < gpu_count; ++i) {  // for each device
        CPhysicalDevice& gpu = gpu_list[i];
        gpu.handle = gpus[i];
        vkGetPhysicalDeviceProperties(gpu, &gpu.properties);
        vkGetPhysicalDeviceFeatures  (gpu, &gpu.features);
        //--Surface caps--
        // VkSurfaceCapabilitiesKHR surface_caps;
        // VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
        //----------------
        gpu.extensions.Init(gpu);
        gpu.extensions.Pick(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        //--Get Queue Families--
        uint family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, NULL);
        vector<VkQueueFamilyProperties> qf_props(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, qf_props.data());

        gpu.queue_families.resize(family_count);
        repeat(family_count){
            auto& family = gpu.queue_families[i];
            family.properties = qf_props[i];
            family.presentable = surface.CanPresent(gpu.handle, i);
            if (family.presentable) gpu.presentable = true;
        }
        //----------------------
    }
}

CPhysicalDevice* CPhysicalDevices::FindPresentable() {
    for (auto& gpu : gpu_list)
        if (gpu.presentable) return &gpu;
    return 0;
}

void CPhysicalDevices::Print(bool show_queues) {
    printf("Physical Devices: %d\n", Count());
    for (uint i = 0; i < Count(); ++i) {  // each gpu
        CPhysicalDevice& gpu = gpu_list[i];
        VkPhysicalDeviceProperties& props = gpu.properties;
        const char* devType[]{"OTHER", "INTEGRATED", "DISCRETE", "VIRTUAL", "CPU"};
        const char* vendor = gpu.VendorName();

        color(gpu.presentable ? eRESET : eFAINT);
        printf("\t%s", gpu.presentable ? cTICK : " ");
        printf(" %d: %s %s %s\n", i, devType[props.deviceType], vendor, props.deviceName);
        //if (show_queues) gpu.queue_families.Print();
        if(show_queues){
            repeat(gpu.queue_families.size()){
                gpu.queue_families[i].Print(i);
            }
        }
        color(eRESET);
    }
}
//----------------------------------------------------------------

//-----------------------------CDevice----------------------------
CQueue* CDevice::AddQueue(VkQueueFlags flags, bool presentable) {
    uint f_inx = gpu.FindQueueFamily(flags, presentable);        // Find correct queue family
    if (f_inx < 0) return 0;                                     // exit if not found
    uint max = gpu.queue_families[f_inx].properties.queueCount;  // max number of queues
    uint q_inx = FamilyQueueCount(f_inx);                        // count queues from this family
    if (q_inx == max) return 0;                                  // exit if too many queues
    CQueue queue = {0, f_inx, q_inx, flags, presentable};        // create queue
    queues.push_back(queue);                                     // add to queue list
    Create();                                                    // create logical device

    LOGI("Queue: %d  flags: [ %s%s%s%s]\n", q_inx, (flags & 1) ? "GRAPHICS " : "", (flags & 2) ? "COMPUTE " : "",
                                                   (flags & 4) ? "TRANSFER " : "", (flags & 8) ? "SPARSE "  : "");
}

uint CDevice::FamilyQueueCount(uint family) {
    uint count = 0;
    for (auto& q : queues) if (q.family == family) count++;
    return count;
}

void CDevice::Create() {
    if (handle) Destroy();  // destroy old handle
    std::vector<float> priorities(queues.size(), 0.0f);
    std::vector<VkDeviceQueueCreateInfo> info_list;
    repeat (gpu.queue_families.size()) {
        uint queue_count = FamilyQueueCount(i);
        if (queue_count > 0) {
            VkDeviceQueueCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.queueFamilyIndex = i;
            info.queueCount       = queue_count;
            info.pQueuePriorities = priorities.data();
            info_list.push_back(info);
            // LOGI("\t%d x queue_family_%d\n", queue_count, i);
        }
    }

    CDeviceExtensions& extensions = gpu.extensions;
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount    = info_list.size();
    device_create_info.pQueueCreateInfos       = info_list.data();
    device_create_info.enabledExtensionCount   = extensions.PickCount();
    device_create_info.ppEnabledExtensionNames = extensions.PickList();
    device_create_info.pEnabledFeatures        = &gpu.enabled_features;
    VKERRCHECK(vkCreateDevice(gpu, &device_create_info, nullptr, &handle));         // create device
    for (auto& q : queues) vkGetDeviceQueue(handle, q.family, q.index, &q.handle);  // get queue handles
}

void CDevice::Destroy(){
    if (!handle) return;
    vkDeviceWaitIdle(handle);
    vkDestroyDevice(handle, nullptr);
    handle = 0;
    //LOGI("Logical device destroyed\n");
}

/*
void CDevice::Print() {  // List created queues
    printf("Logical Device queues:\n");
    uint qcount = queues.size();
    repeat (qcount){
        CQueue& q = queues[i];
        printf("\t%d: family=%d index=%d presentable=%s flags=", i, q.family, q.index, q.presentable ? "True" : "False");
        const char* fnames[]{"GRAPHICS", "COMPUTE", "TRANSFER", "SPARSE"};
        repeat(4) if ((q.flags & 1 << i)) printf("%s ", fnames[i]);
        printf("\n");
    }
}
*/
