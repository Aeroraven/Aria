#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "./AnthemImageInfo.h"

namespace Anthem::Core{
    struct AnthemImageProp{
        VkImage image;
        VkDeviceMemory memory;
        VkImageCreateInfo imageInfo = {};
        VkImageView imageView;
        uint32_t mipmapLodLevels = 1;
        uint32_t layerCounts = 1;
        VkSampleCountFlags msaaCount = VK_SAMPLE_COUNT_1_BIT;
        VkImageLayout desiredLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkShaderStageFlags reqStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkImageCreateFlags extraFlags = 0;
        bool isCubic = false;
    };

    struct AnthemImagePipelineBarrier {
        VkImageLayout layout;
        VkAccessFlags access;
        VkPipelineStageFlags stage;
        uint32_t queueFamily;
    };

    class AnthemImageContainer:
    public Util::AnthemUtlLogicalDeviceReqBase,
    public Util::AnthemUtlPhyDeviceReqBase,
    public Util::AnthemUtlCommandBufferReqBase{
    protected:
        AnthemImageProp image = {};
        VkSampler sampler = nullptr;
        bool samplerCreated = false;

    protected:
        bool createImageInternal(VkImageUsageFlags usage, VkFormat format, uint32_t width, uint32_t height, uint32_t depth);
        bool createImageViewInternal(VkImageAspectFlags aspectFlags,bool use3d = false);
        bool createImageTransitionLayoutLegacy(VkImageLayout oldLayout,VkImageLayout newLayout);
        bool createImageTransitionLayoutInternal(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcFlag,
            VkPipelineStageFlags dstFlag,VkImageAspectFlags aspectFlag);
        bool createSampler();
        bool destroyImageInternal();
        bool destroyImageViewInternal();
        bool generateMipmap2D(uint32_t texWidth,uint32_t texHeight);

    
    public:
        bool recordPipelineBarrier(VkCommandBuffer* cmdBuf, AnthemImagePipelineBarrier* src, AnthemImagePipelineBarrier* dst, VkImageAspectFlags aspectFlag);
        
    public:
        uint32_t virtual getImageWidth();
        uint32_t virtual getImageHeight();
        uint32_t virtual getImageDepth();


        const VkImageView* getImageView() const;
        const VkShaderStageFlags getRequiredShaderStage() const {
            return this->image.reqStageFlags;
        }
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