#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"
#include "AnthemWindowSurface.h"

namespace Anthem::Core{
    struct AnthemSwapChainDetails{
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkPresentModeKHR> presentModes;
        std::vector<VkSurfaceFormatKHR> formats;
    };
    class AnthemSwapChain{
    private:
        ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface;

    public:
        AnthemSwapChain(ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface);
        
        bool virtual prepareSwapChainInfo(VkPhysicalDevice phyDevice,AnthemSwapChainDetails& outDetails);
    };
}
