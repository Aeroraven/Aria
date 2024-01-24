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
        uint32_t mipmapLodLevels = 1;
        VkSampleCountFlags msaaCount = VK_SAMPLE_COUNT_1_BIT;
        VkImageLayout desiredLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    class AnthemImageContainer:
    public Util::AnthemUtlLogicalDeviceReqBase,
    public Util::AnthemUtlPhyDeviceReqBase,
    public Util::AnthemUtlCommandBufferReqBase{
    protected:
        AnthemImageProp image = {};
        VkSampler sampler = nullptr;
    protected:
        bool createImageInternal(VkImageUsageFlags usage, VkFormat format, uint32_t width, uint32_t height, uint32_t depth);
        bool createImageViewInternal(VkImageAspectFlags aspectFlags,bool use3d = false);
        bool createImageTransitionLayout(VkImageLayout oldLayout,VkImageLayout newLayout);
        bool createSampler();
        bool destroyImageInternal();
        bool destroyImageViewInternal();
        bool generateMipmap2D(uint32_t texWidth,uint32_t texHeight);
        
        bool samplerCreated=false;
        
    public:
        uint32_t virtual getImageWidth();
        uint32_t virtual getImageHeight();
        uint32_t virtual getImageDepth();

        const VkImageView* getImageView() const;
        const VkSampleCountFlags getSampleCount() const{
            return this->image.msaaCount;
        }
        const VkSampler* getSampler() const{
            ANTH_ASSERT(this->samplerCreated,"Sampler not created");
            return &(this->sampler);
        }
        const VkImageLayout getDesiredLayout() const{
            return this->image.desiredLayout;
        }
    };
}