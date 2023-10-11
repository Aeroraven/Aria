#include "../../../include/core/base/AnthemPhyDeviceSelector.h"

namespace Anthem::Core{
    bool AnthemPhyDeviceSelector::checkDeviceExtensionSupport(VkPhysicalDevice device){
        uint32_t extensionCounts;
        vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCounts,nullptr);

        std::vector<VkExtensionProperties> extensionProps(extensionCounts);
        vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCounts,extensionProps.data());

        auto supportedReqExts = 0;
        for(const auto& p:extensionProps){
            for(const auto& q:this->requiredDeviceSupportedExtension){
                if(std::string(p.extensionName) == std::string(q)){
                    supportedReqExts += 1;
                }
            }
        }
        return supportedReqExts == this->requiredDeviceSupportedExtension.size();
    }
    ANTH_UNSAFE_PTR(AnthemPhyQueueFamilyIdx) AnthemPhyDeviceSelector::findQueueFamilies(VkPhysicalDevice device,ANTH_SHARED_PTR(AnthemWindowSurface) surface){
        auto indices = ANTH_MAKE_UNSAFE(AnthemPhyQueueFamilyIdx)();
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
            
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface->getWindowSurface(), &presentSupport);

            if(presentSupport){
                indices->presentFamily = i;
            }
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indices->graphicsFamily = i;
            }
            if(indices->graphicsFamily.has_value() && presentSupport){
                break;
            }
            i++;
        }
        return indices;
    }
    bool AnthemPhyDeviceSelector::getPhyDevice(AnthemPhyDevice* phyDevice){
        phyDevice->specifyDevice(this->physicalDevice,this->physicalDeviceQueueFamily);
        return true;
    }
    bool AnthemPhyDeviceSelector::selectPhyDevice(VkInstance* instance, ANTH_SHARED_PTR(AnthemWindowSurface) surface){
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);
        ANTH_ASSERT(deviceCount!=0,"Failed to find support");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

        const auto checkDeviceSuitable = [this,&surface](VkPhysicalDevice device){
            bool availFlag = false;
            int rating = 0;

            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
            AnthemSwapChainDetails scDetails;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);
            this->swapChain->prepareSwapChainInfo(device,scDetails);

            auto extSupportAssert = checkDeviceExtensionSupport(device);
            auto famQueueIdx = this->findQueueFamilies(device,surface);
            auto swapChainSupportAssert = !scDetails.formats.empty() && !scDetails.presentModes.empty();

            for(auto i=0;i<deviceMemoryProperties.memoryHeapCount;i++){
                rating += 1;
                ANTH_LOGI("Heap ",i,":",deviceMemoryProperties.memoryHeaps[i].size,",",deviceMemoryProperties.memoryHeaps[i].flags);
            }
            ANTH_LOGI("Extension Support:",extSupportAssert);
 
            availFlag = (deviceProperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && 
                 famQueueIdx->graphicsFamily.has_value() &&
                 deviceFeatures.geometryShader &&
                 extSupportAssert &&
                 swapChainSupportAssert;
            return std::make_tuple(availFlag, rating, famQueueIdx);
        };

        std::vector<std::tuple<int, VkPhysicalDevice, ANTH_UNSAFE_PTR(AnthemPhyQueueFamilyIdx)>> availableDevices;
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
    const std::vector<const char*>* const AnthemPhyDeviceSelector::getRequiredDevSupportedExts() const{
        return &(this->requiredDeviceSupportedExtension);
    }
    AnthemPhyDeviceSelector::AnthemPhyDeviceSelector(ANTH_SHARED_PTR(AnthemWindowSurface) surface,ANTH_SHARED_PTR(AnthemSwapChain) swapChain){
        this->surface = surface;
        this->swapChain = swapChain;
    }
    void AnthemPhyDeviceSelector::getDeviceFeature(VkPhysicalDeviceFeatures& outFeatures){
        vkGetPhysicalDeviceFeatures(this->physicalDevice,&outFeatures);
    }
}