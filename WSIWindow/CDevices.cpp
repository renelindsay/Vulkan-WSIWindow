#include "CDevices.h"

//-------------------------CQueueFamily---------------------------

//CQueueFamily::CQueueFamily(){}

//-------------------------CQueueFamilies-------------------------
/*
void CQueueFamilies::Init(VkPhysicalDevice gpu){
    uint count=0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, NULL);
    assert(count>0 && "No queue families found.");
    list.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, list.data());
}
*/
void CQueueFamilies::Print(){
    printf("\t   Queue Families: %d\n",Count());
    for(uint i=0; i<Count();++i){
        VkQueueFamilyProperties props = family_list[i];
        const uint32_t     count = props.queueCount;
        const VkQueueFlags flags = props.queueFlags;
        printf("\t        %d:\t",i);
        printf("Queue count = %d\n",count);
        printf("\t\tFlags (");
        const char* sep="";
        if(flags&1){ printf("%s GRAPHICS ",sep); sep="|";}
        if(flags&2){ printf("%s COMPUTE " ,sep); sep="|";}
        if(flags&4){ printf("%s TRANSFER ",sep); sep="|";}
        if(flags&8){ printf("%s SPARSE "  ,sep);         }
        printf(" ) = %d\n", flags);

        VkExtent3D min=props.minImageTransferGranularity;
        printf("\t\tTimestampValidBits = %d\n", props.timestampValidBits);
        printf("\t\tminImageTransferGranularity(w,h,d) = (%d, %d, %d)\n", min.width, min.height, min.depth);
    }
}

//----------------------------------------------------------------
//------------------------CPhysicalDevice-------------------------

const char* CPhysicalDevice::VendorName(){
    //static char vendor_id[8]{};
    uint32_t vendorID=properties.vendorID;
    switch(vendorID){
        case 0x1002 : return "AMD";
        case 0x10DE : return "NVIDIA";
        case 0x8086 : return "INTEL";
        case 0x13B5 : return "ARM";
        case 0x5143 : return "Qualcomm";
        case 0x1010 : return "ImgTec";
        default : return "";
        //default: sprintf(vendor_id, "0x%X",vendorID); return vendor_id;
    }
}

VkDevice CPhysicalDevice::CreateDevice(){
    uint info_count = 1;
    std::vector<VkDeviceQueueCreateInfo> info_list(info_count);
    forCount(info_count){
        uint queue_count = 1;
        std::vector<float> priorities(queue_count, 0.0f);
        VkDeviceQueueCreateInfo& info = info_list[i];
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = 0;
        info.queueCount       = queue_count;
        info.pQueuePriorities = priorities.data();
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

/*
    float priorities[] = {0.0f};
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = 0;
    queue_create_info.queueCount       = 1;
    queue_create_info.pQueuePriorities = priorities;

    VkPhysicalDeviceFeatures features = {};  // disable all features

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount    = 1;
    device_create_info.pQueueCreateInfos       = &queue_create_info;
    device_create_info.enabledExtensionCount   = extensions.PickCount();
    device_create_info.ppEnabledExtensionNames = extensions.PickList();
    device_create_info.pEnabledFeatures        = &features;

    VkDevice device = 0;
    VKERRCHECK(vkCreateDevice(handle, &device_create_info, nullptr, &device));
    return device;
*/
}

//----------------------------------------------------------------
//----------------------------CDevices----------------------------
CDevices::CDevices(VkInstance instance){
    VkResult result;
    uint count=0;
    vector<VkPhysicalDevice> gpus;
    do{
        result = vkEnumeratePhysicalDevices(instance, &count, NULL);             // Get number of gpu's
        if(result == VK_SUCCESS && count>0){
            gpus.resize(count);
            result = vkEnumeratePhysicalDevices(instance, &count, gpus.data());  // Fetch gpu list
        }
    }while (result==VK_INCOMPLETE);                                              // If list is incomplete, try again.
    VKERRCHECK(result);
    if(!count) LOGW("No GPU devices found.");                                    // Vulkan driver missing?

    device_list.resize(count);
    for(uint i=0; i<count; ++i) { // for each device
        CPhysicalDevice& device = device_list[i];
        device.handle = gpus[i];
        vkGetPhysicalDeviceProperties(device.handle, &device.properties);
        vkGetPhysicalDeviceFeatures  (device.handle, &device.features);
        device.extensions.Init(device.handle);
        device.extensions.Pick(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        // device.queue_families.Init(device.handle);

        //--Get Queue Families--
        uint family_count=0;
        vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &family_count, NULL);
        vector<VkQueueFamilyProperties> family_list(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &family_count, family_list.data());

        device.queue_families.family_list.resize(family_count);
        for(uint i=0; i<family_count; ++i){
            device.queue_families.family_list[i].properties=family_list[i];
            device.queue_families.family_list[i].properties.queueCount=99;
        }

        //----------------------


/*
        //--Get Queue Families--
        uint dev_count=0;
        auto& q_families = device.queue_families.family_list;
        vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &dev_count, NULL);
        assert(dev_count > 0 && "No queue families found.");

        //vector<VkQueueFamilyProperties> family_list();


        q_families.resize(dev_count);
        //vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &dev_count, q_families.data());
        vkGetPhysicalDeviceQueueFamilyProperties(device.handle, &dev_count, &q_families[0].properties);
        //----------------------
*/
    }
}

void CDevices::Print(bool show_queues){
    printf("Physical Devices: %d\n",Count());
    for(uint i=0; i<Count(); ++i) { // each gpu
        CPhysicalDevice& device = device_list[i];
        VkPhysicalDeviceProperties& props = device.properties;
        const char* devType[]{"OTHER","INTEGRATED","DISCRETE","VIRTUAL","CPU"};
        const char* vendor = device.VendorName();
        const char* type   = (props.deviceType<5) ? devType[props.deviceType] : 0;
        printf("\t%d: %-10s %-8s %s\n", i, type, vendor, props.deviceName);
        if(show_queues) device.queue_families.Print();
    }
}


//----------------------------------------------------------------
