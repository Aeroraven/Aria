#include "../../../include/core/base/AnthemSwapChain.h"

namespace Anthem::Core{
    AnthemSwapChain::AnthemSwapChain(ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface){
        this->windowSurface = windowSurface;
    }
    bool AnthemSwapChain::prepareSwapChainInfo(VkPhysicalDevice phyDevice, AnthemSwapChainDetails& outDetails){

        //Get Device Surface Capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice,this->windowSurface->getWindowSurface(),&(outDetails.capabilities));
        
        //Get Surface Format
        uint32_t numSurfaceFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice,this->windowSurface->getWindowSurface(),&numSurfaceFormats,nullptr);
        outDetails.formats.resize(numSurfaceFormats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice,this->windowSurface->getWindowSurface(),&numSurfaceFormats,outDetails.formats.data());

        //Get Present Mode
        uint32_t numPresentMode = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice,this->windowSurface->getWindowSurface(),&numPresentMode,nullptr);
        outDetails.presentModes.resize(numPresentMode);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice,this->windowSurface->getWindowSurface(),&numPresentMode,outDetails.presentModes.data());
        
        return true;
    }
    bool AnthemSwapChain::specifySwapChainDetails(AnthemPhyDevice* phyDevice,GLFWwindow* window){
        //Retrieve Swap Chain Details
        this->specifiedSwapChainDetails = ANTH_MAKE_SHARED(AnthemSwapChainDetails)();
        this->prepareSwapChainInfo(phyDevice->getPhysicalDevice(),*(this->specifiedSwapChainDetails));

        //Specify Present Modes
        this->specifiedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        for(const auto& p:this->specifiedSwapChainDetails->presentModes){
            if(p == VK_PRESENT_MODE_MAILBOX_KHR){
                this->specifiedPresentMode = p;
                break;
            }
        }

        //Specify Surface Format
        this->specifiedSurfaceFormat = this->specifiedSwapChainDetails->formats[0];
        for(const auto& p:this->specifiedSwapChainDetails->formats){
            if(p.format == VK_FORMAT_B8G8R8A8_SRGB && p.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                this->specifiedSurfaceFormat = p;
                break;
            }
        }

        //Specify Extent 2D
        if(this->specifiedSwapChainDetails->capabilities.currentExtent.width != UINT32_MAX){
            this->scExtent = this->specifiedSwapChainDetails->capabilities.currentExtent;
        }else{
            int width,height;
            glfwGetFramebufferSize(window,&width,&height);
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            actualExtent.width = std::max(this->specifiedSwapChainDetails->capabilities.minImageExtent.width,std::min(this->specifiedSwapChainDetails->capabilities.maxImageExtent.width,actualExtent.width));
            actualExtent.height = std::max(this->specifiedSwapChainDetails->capabilities.minImageExtent.height,std::min(this->specifiedSwapChainDetails->capabilities.maxImageExtent.height,actualExtent.height));
            this->scExtent = actualExtent;
        }

        //Set Status Flag
        this->reqInfoSpecified = true;
        return true;
    }
    bool AnthemSwapChain::createSwapChain(AnthemLogicalDevice* device,AnthemPhyDevice* phyDevice){
        ANTH_ASSERT(this->reqInfoSpecified,"Swap Chain Details not specified");
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->windowSurface->getWindowSurface();
        createInfo.minImageCount = this->specifiedSwapChainDetails->capabilities.minImageCount;
        createInfo.imageFormat = this->specifiedSurfaceFormat.format;
        createInfo.imageColorSpace = this->specifiedSurfaceFormat.colorSpace;
        createInfo.imageExtent = this->scExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if(phyDevice->getPhyQueueGraphicsFamilyIndice() != phyDevice->getPhyQueuePresentFamilyIndice()){
            uint32_t queueFamilyIndices[] = {phyDevice->getPhyQueueGraphicsFamilyIndice().value(),phyDevice->getPhyQueuePresentFamilyIndice().value()};
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }else{
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.presentMode = this->specifiedPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        createInfo.preTransform = this->specifiedSwapChainDetails->capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        auto createResult = vkCreateSwapchainKHR(device->getLogicalDevice(),&createInfo,nullptr,&(this->swapChain));
        if(createResult != VK_SUCCESS){
            ANTH_LOGI("Failed to create swap chain",createResult);
            return false;
        }
        ANTH_LOGI("Swap chain created");
        return true;
    }
    bool AnthemSwapChain::destroySwapChain(AnthemLogicalDevice* device){
        ANTH_LOGI("Destroying swap chain");
        vkDestroySwapchainKHR(device->getLogicalDevice(),this->swapChain,nullptr);
        return true;
    }
    bool AnthemSwapChain::retrieveSwapChainImages(AnthemLogicalDevice* device){
        uint32_t numImages = 0;
        vkGetSwapchainImagesKHR(device->getLogicalDevice(),this->swapChain,&numImages,nullptr);
        this->swapChainImages.resize(numImages);
        vkGetSwapchainImagesKHR(device->getLogicalDevice(),this->swapChain,&numImages,this->swapChainImages.data());
        swapChainImageRetrieved = true;
        return true;
    }
    bool AnthemSwapChain::createSwapChainImageViews(AnthemLogicalDevice* device){
        ANTH_ASSERT(this->swapChainImageRetrieved,"Swap chain images not retrieved");
        if(!swapChainImageRetrieved){
            ANTH_LOGI("Swap chain images not retrieved");
            return false;
        }
        this->swapChainImageViews.resize(this->swapChainImages.size());
        for(size_t i=0;i<this->swapChainImages.size();i++){
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = this->swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = this->specifiedSurfaceFormat.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.layerCount = 1;
            createInfo.subresourceRange.levelCount = 1;
            auto result = vkCreateImageView(device->getLogicalDevice(),&createInfo,nullptr,&(this->swapChainImageViews[i]));
            if(result != VK_SUCCESS){
                ANTH_LOGI("Failed to create image view",result);
                return false;
            }
        }
        ANTH_LOGI("Swap chain image views created. Size=",this->swapChainImageViews.size());
        return true;
    }
    bool AnthemSwapChain::destroySwapChainImageViews(AnthemLogicalDevice* device){
        ANTH_LOGI("Destroying swap chain image views");
        for(auto& i:this->swapChainImageViews){
            vkDestroyImageView(device->getLogicalDevice(),i,nullptr);
        }
        return true;
    }
    uint32_t AnthemSwapChain::getSwapChainExtentHeight() const{
        return this->scExtent.height;
    }
    uint32_t AnthemSwapChain::getSwapChainExtentWidth() const{
        return this->scExtent.width;
    }
    const VkExtent2D* AnthemSwapChain::getSwapChainExtent() const{
        return &(this->scExtent);
    }
    const VkSurfaceFormatKHR* AnthemSwapChain::getSwapChainSurfaceFormat() const{
        return &(this->specifiedSurfaceFormat);
    }
    const std::vector<VkImageView>* AnthemSwapChain::getSwapChainImageViews() const{
        return &(this->swapChainImageViews);
    }
    const VkSwapchainKHR* AnthemSwapChain::getSwapChain() const{
        return &(this->swapChain);
    }
}