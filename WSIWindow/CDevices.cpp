// Copyright (c) 2017 Rene Lindsay

#include "CDevices.h"

//------------------------CPhysicalDevice-------------------------
CPhysicalDevice::CPhysicalDevice() : handle(0), properties(), features(), extensions() {}

const char* CPhysicalDevice::VendorName() const {
    struct {const uint id; const char* name;} vendors[] =
    {{0x1002, "AMD"}, {0x10DE, "NVIDIA"}, {0x8086, "INTEL"}, {0x13B5, "ARM"}, {0x5143, "Qualcomm"}, {0x1010, "ImgTec"}};
    for (auto vendor : vendors) if (vendor.id == properties.vendorID) return vendor.name;
    return "";
}

// Find queue-family with requred flags, and can present to given surface. (if provided)
// Returns the QueueFamily index, or -1 if not found.
int CPhysicalDevice::FindQueueFamily(VkQueueFlags flags, VkSurfaceKHR surface){
    repeat (queue_families.size()) {
        VkBool32 can_present = false;
        if ((queue_families[i].queueFlags & flags) != flags) continue;
        if (surface) VKERRCHECK(vkGetPhysicalDeviceSurfaceSupportKHR(handle, i, surface, &can_present));
        if (!!surface == !!can_present) return i;
    }
    return -1;
}

std::vector<VkSurfaceFormatKHR> CPhysicalDevice::SurfaceFormats(VkSurfaceKHR surface) {  // Get Surface format list
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, formats.data());
    ASSERT(!!count, "No supported surface formats found.");
    return formats;
}

//--Returns the first supported surface color format from the preferred_formats list, or VK_FORMAT_UNDEFINED if no match found.
VkFormat CPhysicalDevice::FindSurfaceFormat(VkSurfaceKHR surface, std::vector<VkFormat> preferred_formats) {
    auto formats = SurfaceFormats(surface);  // get list of supported surface formats
    for (auto& pf : preferred_formats) 
        for (auto& f : formats) 
            if(f.format == pf) return f.format;
    //return formats[0].format;  //first supported format
    return VK_FORMAT_UNDEFINED;
}

VkFormat CPhysicalDevice::FindDepthFormat(std::vector<VkFormat> preferred_formats) {
    for (auto& format : preferred_formats) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(handle, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED; // 0
}
//----------------------------------------------------------------

//------------------------CPhysicalDevices------------------------
CPhysicalDevices::CPhysicalDevices(const VkInstance instance) {
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

        // Get Queue Family properties
        uint family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, NULL);
        gpu.queue_families.resize(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, gpu.queue_families.data());
    }
}

CPhysicalDevice* CPhysicalDevices::FindPresentable(VkSurfaceKHR surface) {
    for (auto& gpu : gpu_list)
        if (gpu.FindQueueFamily(0, surface) >= 0 ) return &gpu;
    LOGW("No devices can present to this surface. (Is DRI3 enabled?)\n");
    return 0;
}

void CPhysicalDevices::Print(bool show_queues) {
    printf("Physical Devices: %d\n", Count());
    for (uint i = 0; i < Count(); ++i) {  // each gpu
        CPhysicalDevice& gpu = gpu_list[i];
        VkPhysicalDeviceProperties& props = gpu.properties;
        const char* devType[]{"OTHER", "INTEGRATED", "DISCRETE", "VIRTUAL", "CPU"};
        const char* vendor = gpu.VendorName();
        printf("\t%d: %s %s %s\n", i, devType[props.deviceType], vendor, props.deviceName);
        if(show_queues){
            repeat(gpu.queue_families.size()){
                VkQueueFamilyProperties& props = gpu.queue_families[i];
                uint flags = props.queueFlags;
                printf("\t\tQueue-family:%d  count:%2d  flags:[ %s%s%s%s]\n", i, props.queueCount,
                    (flags & 1) ? "GRAPHICS " : "", (flags & 2) ? "COMPUTE " : "",
                    (flags & 4) ? "TRANSFER " : "", (flags & 8) ? "SPARSE "  : "");
            }
        }
    }
}
//----------------------------------------------------------------

//-----------------------------CDevice----------------------------
CQueue* CDevice::AddQueue(VkQueueFlags flags, VkSurfaceKHR surface) {
    uint f_inx = gpu.FindQueueFamily(flags, surface);                                        // Find correct queue family
    if (f_inx < 0) { LOGW("Could not create queue with requested properties."); return 0; }  // exit if not found
    uint max = gpu.queue_families[f_inx].queueCount;                                         // max number of queues
    uint q_inx = FamilyQueueCount(f_inx);                                                    // count queues from this family
    if (q_inx == max) { LOGW("No more queues available from this family."); return 0; }      // exit if too many queues
    CQueue queue = {0, f_inx, q_inx, flags, surface, handle, gpu};                           // create queue
    queues.push_back(queue);                                                                 // add to queue list
    Create();                                                                                // create logical device
    LOGI("Queue: %d  flags: [ %s%s%s%s]%s\n", q_inx,
         (flags & 1) ? "GRAPHICS " : "", (flags & 2) ? "COMPUTE " : "",
         (flags & 4) ? "TRANSFER " : "", (flags & 8) ? "SPARSE "  : "",
         surface ? " (can present)" : "");
    return &queues.back();
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
    device_create_info.queueCreateInfoCount    = (uint32_t)info_list.size();
    device_create_info.pQueueCreateInfos       = info_list.data();
    device_create_info.enabledExtensionCount   = extensions.PickCount();
    device_create_info.ppEnabledExtensionNames = extensions.PickList();
    device_create_info.pEnabledFeatures        = &gpu.enabled_features;
    VKERRCHECK(vkCreateDevice(gpu, &device_create_info, nullptr, &handle));         // create device
    //for (auto& q : queues) vkGetDeviceQueue(handle, q.family, q.index, &q.handle);  // get queue handles
    for (auto& q : queues) {
        q.device = handle;
        vkGetDeviceQueue(handle, q.family, q.index, &q.handle);  // get queue handles
    }

}

void CDevice::Destroy(){
    if (!handle) return;
    vkDeviceWaitIdle(handle);
    vkDestroyDevice(handle, nullptr);
    handle = 0;
}

CDevice::CDevice(CPhysicalDevice gpu) : handle() {
    this->gpu = gpu;
    LOGI("Logical Device using GPU: %s\n",gpu.properties.deviceName);
#ifdef ENABLE_VALIDATION
    gpu.extensions.Print();
#endif
}

CDevice::~CDevice() {
    Destroy();
    LOGI("Logical device destroyed\n");
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
