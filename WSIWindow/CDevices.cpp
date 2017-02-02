#include "CDevices.h"

//-------------------------CQueueFamily---------------------------

//CQueueFamily::CQueueFamily(){}

uint CQueueFamily::Pick(uint count){
    uint available = properties.queueCount - pick_count;
    uint pick_add = min(available, count);
    pick_count += pick_add;
    return pick_add;
}

//-------------------------CQueueFamilies-------------------------
void CQueueFamilies::Print(){
    printf("\t     Queue Families: %d\n", Count());
    for(uint i=0; i<Count(); ++i){
        VkQueueFamilyProperties props = family_list[i];
        printf("\t       %d:\t",i);
        printf("Queue count = %d\n", props.queueCount);
        // --Print queue flags--
        char buf[50]{};
        int flags = props.queueFlags << 1;
        for(auto flag : {"GRAPHICS", "COMPUTE", "TRANSFER", "SPARSE"})
            if ((flags >>= 1) & 1) sprintf(buf, "%s%s | ", buf, flag);
        printf("\t\tFlags ( %.*s) = %d\n", (int)strlen(buf) - 2, buf, props.queueFlags);
        // ---------------------
        VkExtent3D min=props.minImageTransferGranularity;
        printf("\t\tTimestampValidBits = %d\n", props.timestampValidBits);
        printf("\t\tminImageTransferGranularity(w,h,d) = (%d, %d, %d)\n", min.width, min.height, min.depth);
    }
}

int CQueueFamilies::Find(VkQueueFlags flags){
    forCount(Count()){
        const VkQueueFamilyProperties& family = family_list[i];
        if((family.queueFlags & flags) == flags) return i;
    }
    return -1;
}

int CQueueFamilies::FindPresentable(){
    forCount(Count()){
        if(family_list[i].IsPresentable()) return i;
    }
    return -1;
}

bool CQueueFamilies::Pick(uint presentable, uint graphics, uint compute, uint transfer){
    forCount(Count()){
        CQueueFamily& family = family_list[i];
        if (family.IsPresentable()) presentable -= family.Pick(presentable);
        if (family.Has(VK_QUEUE_GRAPHICS_BIT)) graphics -= family.Pick(graphics);
        if (family.Has(VK_QUEUE_COMPUTE_BIT )) compute  -= family.Pick(compute );
        if (family.Has(VK_QUEUE_TRANSFER_BIT)) transfer -= family.Pick(transfer);
    }
    return (presentable + graphics + compute + transfer == 0);
}

//----------------------------------------------------------------
//------------------------CPhysicalDevice-------------------------
CPhysicalDevice::CPhysicalDevice() : handle(0), properties(), features(), queue_families(), extensions(), presentable(){}

const char* CPhysicalDevice::VendorName() const {
    struct {const uint id; const char* name;} vendors[] =
    {{0x1002, "AMD"}, {0x10DE, "NVIDIA"}, {0x8086, "INTEL"}, {0x13B5, "ARM"}, {0x5143, "Qualcomm"}, {0x1010, "ImgTec"}};
    for(auto vendor : vendors) if(vendor.id == properties.vendorID) return vendor.name;
    return "";
}


CDevice CPhysicalDevice::Create(){
    LOGI("Creating logical device:\n");

    //--TEMP--
    queue_families.Pick(1,0,0,0);  // Pick one presentable queue only.
    //--------

    uint info_count = queue_families.Count();
    std::vector<VkDeviceQueueCreateInfo> info_list(info_count);
    forCount(info_count){
        uint queue_count = queue_families[i].pick_count;
        std::vector<float> priorities(queue_count, 0.0f);
        VkDeviceQueueCreateInfo& info = info_list[i];
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = i;
        info.queueCount       = queue_count;
        info.pQueuePriorities = priorities.data();
        LOGI("\t%d x queue_family_%d\n", queue_count, i);
    }

    VkPhysicalDeviceFeatures features = {};  // disable all features
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount    = info_count;
    device_create_info.pQueueCreateInfos       = info_list.data();
    device_create_info.enabledExtensionCount   = extensions.PickCount();
    device_create_info.ppEnabledExtensionNames = extensions.PickList();
    device_create_info.pEnabledFeatures        = &features;

    CDevice device;
    VKERRCHECK(vkCreateDevice(handle, &device_create_info, nullptr, &device.handle));

    //--Create device queues--
    forCount(queue_families.Count()){
        uint f_inx = i;
        CQueueFamily& family = queue_families[i];
        uint pick_count = queue_families[i].pick_count;
        forCount(pick_count){
            CQueue q;
            uint q_inx = i;
            q.flags       = family.properties.queueFlags;
            q.presentable = family.IsPresentable();
            q.family      = f_inx;
            q.index       = q_inx;
            vkGetDeviceQueue(device.handle, f_inx, q_inx, &q.handle);
            device.queues.push_back(q);
        }
    }
    //-----------------------
    device.Print();
    return device;
}

void CDevice::Print(){  // List created queues
    printf("Logical Device queues:\n");
    uint qcount = queues.size();
    forCount(qcount){
       CQueue& q=queues[i];
       char flags[5]{"GCST"};
       forCount(4){ if (!(q.flags & 1<<i))  flags[i]='.'; }
       printf("\t%d: family=%d index=%d flags=%s presentable=%s\n", i, q.family,q.index, flags, q.presentable ? "True" : "False");
    }
}


/*
VkDevice CPhysicalDevice::Create(){
    LOGI("Creating logical device:\n");
    uint info_count = queue_families.Count();
    std::vector<VkDeviceQueueCreateInfo> info_list(info_count);
    forCount(info_count){
        uint queue_count = queue_families[i].pick_count;
        std::vector<float> priorities(queue_count, 0.0f);
        VkDeviceQueueCreateInfo& info = info_list[i];
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = i;
        info.queueCount       = queue_count;
        info.pQueuePriorities = priorities.data();
        //LOGI("\tqueue-family:%d  count:%d\n", i, queue_count);
        LOGI("\t%d x queue-family:%d\n", queue_count, i);
    }

    VkPhysicalDeviceFeatures features = {};  // disable all features
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount    = info_count;
    device_create_info.pQueueCreateInfos       = info_list.data();
    device_create_info.enabledExtensionCount   = extensions.PickCount();
    device_create_info.ppEnabledExtensionNames = extensions.PickList();
    device_create_info.pEnabledFeatures        = &features;

    VkDevice device = 0;
    VKERRCHECK(vkCreateDevice(handle, &device_create_info, nullptr, &device));
    return device;
}
*/
//----------------------------------------------------------------
//----------------------------CDevices----------------------------
CDevices::CDevices(const CSurface& surface){
    VkInstance instance = surface.instance;
    VkResult result;
    uint gpu_count=0;
    vector<VkPhysicalDevice> gpus;
    do{
        result = vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);             // Get number of gpu's
        if(result == VK_SUCCESS && gpu_count>0){
            gpus.resize(gpu_count);
            result = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());  // Fetch gpu list
        }
    }while (result == VK_INCOMPLETE);                                                // If list is incomplete, try again.
    VKERRCHECK(result);
    if(!gpu_count) LOGW("No GPU devices found.");                                        // Vulkan driver missing?

    device_list.resize(gpu_count);
    for(uint i=0; i<gpu_count; ++i) { // for each device
        CPhysicalDevice& device = device_list[i];
        device.handle = gpus[i];
        vkGetPhysicalDeviceProperties(device.handle, &device.properties);
        vkGetPhysicalDeviceFeatures  (device.handle, &device.features);
        device.extensions.Init(device.handle);
        device.extensions.Pick(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        //--Get Queue Families--
        uint family_count=0;
        vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &family_count, NULL);
        vector<VkQueueFamilyProperties> family_list(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &family_count, family_list.data());

        device.queue_families.family_list.resize(family_count);
        for(uint i=0; i<family_count; ++i){
            CQueueFamily& family = device.queue_families.family_list[i];
            family.properties = family_list[i];
            family.presentable = surface.CanPresent(device.handle, i);
            if(family.presentable) device.presentable = true;
        }
        //----------------------
    }
}

void CDevices::Print(bool show_queues){
    printf("Physical Devices: %d\n",Count());
    for(uint i=0; i<Count(); ++i) { // each gpu
        CPhysicalDevice& device = device_list[i];
        VkPhysicalDeviceProperties& props = device.properties;
        const char* devType[]{"OTHER", "INTEGRATED", "DISCRETE", "VIRTUAL", "CPU"};
        const char* vendor = device.VendorName();

        color(device.presentable ? eRESET : eFAINT );
        printf("\t%s", device.presentable ? cTICK : " ");
        printf(" %d: %-10s %-8s %s\n", i, devType[props.deviceType], vendor, props.deviceName);
        if(show_queues) device.queue_families.Print();
        color(eRESET);
    }
}


//----------------------------------------------------------------
