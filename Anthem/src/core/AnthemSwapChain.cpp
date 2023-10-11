#include "../../include/core/AnthemSwapChain.h"

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
}