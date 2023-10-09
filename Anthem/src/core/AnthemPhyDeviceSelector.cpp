#include "../../include/core/AnthemPhyDeviceSelector.h"

namespace Anthem::Core{
    ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) AnthemPhyDeviceSelector::findQueueFamilies(VkPhysicalDevice device){
        auto indices = ANTH_MAKE_SHARED(AnthemPhyQueueFamilyIdx)();
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        if(queueFamilyCount==0){
            ANTH_LOGI("Failed to find queue families");
            return indices;
        }
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for(const auto& queueFamily : queueFamilies){
            ANTH_LOGI("Queue Family ",i,":",queueFamily.queueFlags);
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indices->graphicsFamily = i;
            }
            if(indices->graphicsFamily.has_value()){
                break;
            }
            i++;
        }
        return indices;
    }
    VkPhysicalDevice AnthemPhyDeviceSelector::getPhyDevice(){
        return this->physicalDevice;
    }
    ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) AnthemPhyDeviceSelector::getPhyDeviceQueueFamilyInfo(){
        return this->physicalDeviceQueueFamily;
    }
    bool AnthemPhyDeviceSelector::selectPhyDevice(VkInstance* instance){
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);
        ANTH_ASSERT(deviceCount!=0,"Failed to find support");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

        const auto checkDeviceSuitable = [this](VkPhysicalDevice device){
            bool availFlag = false;
            int rating = 0;

            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);
            auto famQueueIdx = this->findQueueFamilies(device);

            ANTH_LOGI("Geometry:",deviceFeatures.geometryShader);
            for(auto i=0;i<deviceMemoryProperties.memoryHeapCount;i++){
                rating += 1;
                ANTH_LOGI("Heap ",i,":",deviceMemoryProperties.memoryHeaps[i].size,",",deviceMemoryProperties.memoryHeaps[i].flags);
            }
            ANTH_LOGI("Max Image D2D:",deviceProperties.limits.maxImageDimension2D);

            availFlag = (deviceProperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && 
                 famQueueIdx->graphicsFamily.has_value() &&
                 deviceFeatures.geometryShader;
            return std::make_tuple(availFlag, rating, famQueueIdx);
        };

        std::vector<std::tuple<int, VkPhysicalDevice, ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx)>> availableDevices;
        for(const auto& device : devices){
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            ANTH_LOGI("Found GPU: ",deviceProperties.deviceName," ApiVer:",deviceProperties.apiVersion);
            auto conds = checkDeviceSuitable(device);
            availableDevices.push_back(std::make_tuple(std::get<1>(conds), device, std::get<2>(conds)));
        }
        ANTH_ASSERT(availableDevices.size()!=0,"Failed to find suitable GPU");
        std::sort(availableDevices.begin(), availableDevices.end(), [](const auto& a, const auto& b){
            return std::get<0>(a)>std::get<0>(b);
        });
        auto chosenDevice = availableDevices.at(0);
        ANTH_LOGI("Chosen Score=",std::get<0>(chosenDevice));
        this->physicalDevice = std::get<1>(chosenDevice);
        this->physicalDeviceQueueFamily = std::get<2>(chosenDevice);
        return true;
    }
}