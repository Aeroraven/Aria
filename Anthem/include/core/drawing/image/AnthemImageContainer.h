#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"


namespace Anthem::Core{
    struct AnthemImageProp{
        VkImage image;
        VkDeviceMemory memory;
        VkImageCreateInfo imageInfo = {};
        VkImageView imageView;
        int mipmapLodLevels = 1;
        VkSampleCountFlags msaaCount = VK_SAMPLE_COUNT_1_BIT;
    };

    class AnthemImageContainer:
    public Util::AnthemUtlLogicalDeviceReqBase,
    public Util::AnthemUtlPhyDeviceReqBase,
    public Util::AnthemUtlCommandBufferReqBase{
    protected:
        AnthemImageProp image = {};
    protected:
        bool createImageInternal(VkImageUsageFlags usage, VkFormat format, uint32_t width, uint32_t height);
        bool createImageViewInternal(VkImageAspectFlags aspectFlags);
        bool createImageTransitionLayout(VkImageLayout oldLayout,VkImageLayout newLayout);
        bool destroyImageInternal();
        bool destroyImageViewInternal();
        bool generateMipmap(uint32_t texWidth,uint32_t texHeight);
        
    public:
        uint32_t virtual getImageWidth();
        uint32_t virtual getImageHeight();
        const VkImageView* getImageView() const;
        const VkSampleCountFlags getSampleCount() const{
            return this->image.msaaCount;
        }
    };
}