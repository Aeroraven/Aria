#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"

namespace Anthem::Core{
    class AnthemDepthBuffer:
    public Util::AnthemUtlLogicalDeviceReqBase,
    public Util::AnthemUtlPhyDeviceReqBase,
    public Util::AnthemUtlSwapChainReqBase,
    public Util::AnthemUtlCommandBufferReqBase{
    private:
        VkImageCreateInfo imageInfo = {};
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        
    protected:
        bool createImageInternal();
        bool createImageViewInternal();
        bool createImageTransitionLayout(VkImageLayout oldLayout,VkImageLayout newLayout);
    public:
        bool createDepthBuffer();
        bool destroyDepthBuffer();
        VkFormat getDepthFormat() const{
            return depthFormat;
        }
        const VkImageView* getImageView() const{
            return &depthImageView;
        }
    };
}