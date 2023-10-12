#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"
#include "AnthemWindowSurface.h"
#include "AnthemPhyDevice.h"
#include "AnthemLogicalDevice.h"

namespace Anthem::Core{
    struct AnthemSwapChainDetails{
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkPresentModeKHR> presentModes;
        std::vector<VkSurfaceFormatKHR> formats;
    };
    class AnthemSwapChain{
    private:
        ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface;
        ANTH_SHARED_PTR(AnthemSwapChainDetails) specifiedSwapChainDetails;

        VkPresentModeKHR specifiedPresentMode;
        VkSurfaceFormatKHR specifiedSurfaceFormat;
        VkExtent2D scExtent;
        
        VkSwapchainKHR swapChain;

        bool reqInfoSpecified = false;
        bool swapChainImageRetrieved = false;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

    public:
        AnthemSwapChain(ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface);
        bool virtual prepareSwapChainInfo(VkPhysicalDevice phyDevice,AnthemSwapChainDetails& outDetails);
        bool virtual specifySwapChainDetails(AnthemPhyDevice* phyDevice,const GLFWwindow* window);
        bool virtual createSwapChain(AnthemLogicalDevice* device,AnthemPhyDevice* phyDevice);
        bool virtual destroySwapChain(AnthemLogicalDevice* device);
        bool virtual retrieveSwapChainImages(AnthemLogicalDevice* device);
        bool virtual createSwapChainImageViews(AnthemLogicalDevice* device);
        bool virtual destroySwapChainImageViews(AnthemLogicalDevice* device);

        uint32_t virtual getSwapChainExtentWidth() const;
        uint32_t virtual getSwapChainExtentHeight() const;
        const VkExtent2D* getSwapChainExtent() const;
        const VkSurfaceFormatKHR* getSwapChainSurfaceFormat() const;

        const std::vector<VkImageView>* getSwapChainImageViews() const;
        const VkSwapchainKHR* getSwapChain() const;
    };
}
