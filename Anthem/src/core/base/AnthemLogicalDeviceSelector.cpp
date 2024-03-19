#include "../../../include/core/base/AnthemLogicalDeviceSelector.h"

namespace Anthem::Core{
    AnthemLogicalDeviceSelector::AnthemLogicalDeviceSelector(AnthemPhyDevice* phyDevice){
        this->phyDevice = phyDevice;
    }
    bool AnthemLogicalDeviceSelector::createLogicalDevice(){
        //Create Queue
        registerQueueCreateInfo(phyDevice->getPhyQueueGraphicsFamilyIndice().value());
        registerQueueCreateInfo(phyDevice->getPhyQueuePresentFamilyIndice().value());
        

        //Get Device Extension
        const auto deviceSupportExtensions = phyDevice->getRequiredDevSupportedExts();

        //Get Device Features
        this->creatingFeats = phyDevice->getDeviceFeatures();
        this->creatingFeats.samplerAnisotropy = VK_TRUE;

        //Create Device
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = this->queueCreateInfo.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(this->queueCreateInfo.size());
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceSupportExtensions->size());
        createInfo.ppEnabledExtensionNames = deviceSupportExtensions->data();
        createInfo.enabledLayerCount = 0;
        createInfo.pEnabledFeatures = &(this->creatingFeats);

        VkPhysicalDeviceFeatures2 sPhyFeats = {};
        sPhyFeats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        sPhyFeats.features = this->creatingFeats;
        sPhyFeats.pNext = nullptr;

        void** lastCreateInfo = (void**)&createInfo.pNext;
        auto appendFeatureEnabler = [&](auto* ptr) {
            if (lastCreateInfo == (&createInfo.pNext)) {
                createInfo.pNext = &sPhyFeats;
                createInfo.pEnabledFeatures = nullptr;
                sPhyFeats.pNext = ptr;
            }
            else {
                *lastCreateInfo = ptr;
            }
            lastCreateInfo = &ptr->pNext;
            ptr->pNext = nullptr;
        };

#ifdef VK_EXT_mesh_shader
        VkPhysicalDeviceMeshShaderFeaturesEXT extMeshShader = {};
        extMeshShader.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
        extMeshShader.meshShader = VK_TRUE;
        extMeshShader.taskShader = VK_TRUE;
        appendFeatureEnabler(&extMeshShader);
#endif
        
#ifdef AT_FEATURE_RAYTRACING_ENABLED
#ifdef VK_KHR_acceleration_structure
        VkPhysicalDeviceAccelerationStructureFeaturesKHR extAccStruct = {};
        extAccStruct.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        extAccStruct.accelerationStructure = VK_TRUE;
        appendFeatureEnabler(&extAccStruct);
#endif

#ifdef VK_KHR_buffer_device_address
        VkPhysicalDeviceBufferDeviceAddressFeatures extBda = {};
        extBda.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        extBda.bufferDeviceAddress = VK_TRUE;
        appendFeatureEnabler(&extBda);
#endif 

#ifdef VK_KHR_ray_tracing_pipeline
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR extRtPipe = {};
        extRtPipe.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        extRtPipe.rayTracingPipeline = VK_TRUE;
        appendFeatureEnabler(&extRtPipe);
#endif
#ifdef VK_KHR_ray_query
        VkPhysicalDeviceRayQueryFeaturesKHR extRayQuery = {};
        extRayQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        extRayQuery.rayQuery = VK_TRUE;
        appendFeatureEnabler(&extRayQuery);
#endif
#endif

        auto res = vkCreateDevice(phyDevice->getPhysicalDevice(), &createInfo, nullptr, &logicalDevice);
        if(res!=VK_SUCCESS){
            ANTH_LOGI("Failed to create logical device", res);
            return false;
        }
        ANTH_LOGI("Logical device created");
        //Retrieving Queue Handlers
        this->retrieveQueues();
        return true;
    }
    bool AnthemLogicalDeviceSelector::destroyLogicalDevice(){
        ANTH_DEPRECATED_MSG;
        ANTH_LOGI("Destroying logical device");
        vkDestroyDevice(logicalDevice, nullptr);
        return true;
    }
    bool AnthemLogicalDeviceSelector::retrieveQueues(){
        vkGetDeviceQueue(logicalDevice, phyDevice->getPhyQueueGraphicsFamilyIndice().value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, phyDevice->getPhyQueuePresentFamilyIndice().value(), 0, &presentationQueue);
        vkGetDeviceQueue(logicalDevice, phyDevice->getPhyQueueComputeFamilyIndice().value(), 0, &computeQueue);
        return true;
    }
    bool AnthemLogicalDeviceSelector::registerQueueCreateInfo(std::optional<uint32_t> familyIdx,float priority){
        if(!familyIdx.has_value()){
            return false;
        }
        if(queueExistingId.count(familyIdx.value())!=0){
            return false;
        }
        VkDeviceQueueCreateInfo qci;
        qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex = familyIdx.value();
        qci.queueCount = 1;
        auto queuePriority = ANTH_MAKE_UNSAFE(float)(1.0f);
        qci.pQueuePriorities = queuePriority;
        qci.pNext = nullptr;
        qci.flags = 0;
        this->queueCreateInfo.push_back(qci);
        queueExistingId.insert(familyIdx.value());
        return true;
    }
    VkDevice AnthemLogicalDeviceSelector::getLogicalDevice(){
        return this->logicalDevice;
    }
    bool AnthemLogicalDeviceSelector::getLogicalDevice(AnthemLogicalDevice* logicalDevice){
        logicalDevice->specifyDevice(this->logicalDevice);
        logicalDevice->specifyGraphicsQueue(this->graphicsQueue);
        logicalDevice->specifyPresentQueue(this->presentationQueue);
        logicalDevice->specifyComputeQueue(this->computeQueue);
        return true;
    }
}   