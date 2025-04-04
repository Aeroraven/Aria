#include "../../../../include/core/drawing/image/AnthemImageContainer.h"

namespace Anthem::Core{
    bool AnthemImageContainer::generateMipmap2D(uint32_t texWidth,uint32_t texHeight){
        //Referenced from https://vulkan-tutorial.com/Generating_Mipmaps

        uint32_t cmdBufIdx;
        this->cmdBufs->createCommandBuffer(&cmdBufIdx);
        this->cmdBufs->startCommandRecording(cmdBufIdx);
        this->generateMipmap2DCommandBufferInternal(cmdBufIdx, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        
        this->cmdBufs->endCommandRecording(cmdBufIdx);  
        this->cmdBufs->submitTaskToGraphicsQueue(cmdBufIdx,true);
        this->cmdBufs->freeCommandBuffer(cmdBufIdx);
        ANTH_LOGV("Mipmap generated");

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(phyDevice->getPhysicalDevice(), VK_FORMAT_R8G8B8A8_SRGB, &formatProperties);
        ANTH_ASSERT((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),"Unsupported Linear Filtering");
        return true;
    }
    bool AnthemImageContainer::generateMipmap2DCommandBufferInternal(uint32_t cmdIdx, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcFlag,
        VkPipelineStageFlags dstFlag, VkImageAspectFlags aspectFlag) {

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = this->image.image;
        barrier.subresourceRange.aspectMask = aspectFlag;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;

        int32_t mipWidth = this->getImageWidth();
        int32_t mipHeight = this->getImageHeight();

        for (uint32_t i = 1u; i < this->image.mipmapLodLevels; i++) {
            ANTH_LOGI("MipLvl:", i, " Wid=", (signed)mipWidth, " Hei=", (signed)mipHeight);
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(*this->cmdBufs->getCommandBuffer(cmdIdx), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr, 0, nullptr, 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = aspectFlag;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = aspectFlag;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(*this->cmdBufs->getCommandBuffer(cmdIdx), this->image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->image.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(*this->cmdBufs->getCommandBuffer(cmdIdx), VK_PIPELINE_STAGE_TRANSFER_BIT, dstFlag, 0,
                0, nullptr, 0, nullptr, 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }
        barrier.subresourceRange.baseMipLevel = this->image.mipmapLodLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(*(this->cmdBufs->getCommandBuffer(cmdIdx)), VK_PIPELINE_STAGE_TRANSFER_BIT, dstFlag,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
        return true;
    }
    bool AnthemImageContainer::destroyImageViewInternal(){
        vkDestroyImageView(this->logicalDevice->getLogicalDevice(),this->image.imageView,nullptr);
        if (image.imageViewFb != nullptr) {
            vkDestroyImageView(this->logicalDevice->getLogicalDevice(), this->image.imageViewFb, nullptr);
        }

        return true;
    }
    bool AnthemImageContainer::destroyImageInternal(){
        vkDestroyImage(this->logicalDevice->getLogicalDevice(),this->image.image,nullptr);
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->image.memory,nullptr);
        return true;
    }
    bool AnthemImageContainer::createImageViewInternal(VkImageAspectFlags aspectFlags, bool use3d){
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->image.image;
        if (this->image.isCubic) {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }else if (!use3d) {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        }
        else {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        }
        createInfo.format =  this->image.imageInfo.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectFlags;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = this->image.layerCounts;
        createInfo.subresourceRange.levelCount = this->image.mipmapLodLevels;
        auto result = vkCreateImageView(this->logicalDevice->getLogicalDevice(),&createInfo,nullptr,&(this->image.imageView));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create image view",result);
            return false;
        }
        return true;
    }
    bool AnthemImageContainer::createImageViewFbInternal(VkImageAspectFlags aspectFlags, bool use3d) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->image.image;
        if (this->image.isCubic) {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }
        else if (!use3d) {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        }
        else {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        }
        createInfo.format = this->image.imageInfo.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectFlags;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = this->image.layerCounts;
        createInfo.subresourceRange.levelCount = 1;
        auto result = vkCreateImageView(this->logicalDevice->getLogicalDevice(), &createInfo, nullptr, &(this->image.imageViewFb));
        if (result != VK_SUCCESS) {
            ANTH_LOGE("Failed to create image view", result);
            return false;
        }
        return true;
    }
    bool  AnthemImageContainer::createImageTransitionLayoutInternal(VkImageLayout oldLayout, VkImageLayout newLayout,
        VkPipelineStageFlags srcFlag, VkPipelineStageFlags dstFlag, VkImageAspectFlags aspectFlag) {
        uint32_t cmdBufIdx;
        this->cmdBufs->createCommandBuffer(&cmdBufIdx);
        this->cmdBufs->startCommandRecording(cmdBufIdx);
        
        AnthemImageInfoProcessing::setImageLayout(this->image.image,
            *(this->cmdBufs->getCommandBuffer(cmdBufIdx)), oldLayout, newLayout, srcFlag, dstFlag,
            this->image.layerCounts, this->image.mipmapLodLevels,aspectFlag);

        this->cmdBufs->endCommandRecording(cmdBufIdx);
        this->cmdBufs->submitTaskToGraphicsQueue(cmdBufIdx, true);
        this->cmdBufs->freeCommandBuffer(cmdBufIdx);
        return true;
    }
    bool AnthemImageContainer::createImageTransitionLayoutLegacy(VkImageLayout oldLayout,VkImageLayout newLayout){
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            return this->createImageTransitionLayoutInternal(oldLayout, newLayout, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,VK_IMAGE_ASPECT_COLOR_BIT);
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
            return  this->createImageTransitionLayoutInternal(oldLayout, newLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
            return  this->createImageTransitionLayoutInternal(oldLayout, newLayout, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) 
            return  this->createImageTransitionLayoutInternal(oldLayout, newLayout, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        throw std::invalid_argument("unsupported layout transition!");
    }
    bool AnthemImageContainer::createSampler(){
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        auto maxAnisotropy =  (this->phyDevice->getDeviceProperties()).limits.maxSamplerAnisotropy;
        samplerInfo.maxAnisotropy = maxAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        if(this->image.mipmapLodLevels > 1){
            samplerInfo.maxLod = 1.0f*this->image.mipmapLodLevels;
        }else{
            samplerInfo.maxLod = 1.0f;
        }
        this->samplerCreated = true;
        auto samplerResult = vkCreateSampler(this->logicalDevice->getLogicalDevice(),&samplerInfo,nullptr,&(this->sampler));
        ANTH_ASSERT(samplerResult==VK_SUCCESS,"Failed to create sampler");
        return true;
    }
    bool AnthemImageContainer::createImageInternal(VkImageUsageFlags usage,  VkFormat format, uint32_t width, uint32_t height,uint32_t depth){
        //Filling Image Info
        this->image.imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        if (depth > 1) {
            this->image.imageInfo.imageType = VK_IMAGE_TYPE_3D;
        }
        else {
            this->image.imageInfo.imageType = VK_IMAGE_TYPE_2D;
        }
        this->image.imageInfo.extent.width = width;
        this->image.imageInfo.extent.height = height;
        this->image.imageInfo.extent.depth = depth;
        this->image.imageInfo.mipLevels = this->image.mipmapLodLevels;
        this->image.imageInfo.arrayLayers = this->image.layerCounts;;
        this->image.imageInfo.format = format;
        this->image.imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        this->image.imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        this->image.imageInfo.usage = usage;
        this->image.imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        this->image.imageInfo.samples = (decltype(this->image.imageInfo.samples)) this->image.msaaCount;
        this->image.imageInfo.flags = this->image.extraFlags;

        auto creatImageRes = vkCreateImage(this->logicalDevice->getLogicalDevice(),&(this->image.imageInfo),nullptr,&(this->image.image));
        ANTH_ASSERT(creatImageRes==VK_SUCCESS,"Failed to create image");

        //Create Image Memory
        VkMemoryRequirements memReq = {};
        vkGetImageMemoryRequirements(this->logicalDevice->getLogicalDevice(),this->image.image,&memReq);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = this->phyDevice->findMemoryType(memReq.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        auto allocMemRes = vkAllocateMemory(this->logicalDevice->getLogicalDevice(),&allocInfo,nullptr,&(this->image.memory));
        ANTH_ASSERT(allocMemRes==VK_SUCCESS,"Failed to allocate memory for image");

        vkBindImageMemory(this->logicalDevice->getLogicalDevice(),this->image.image, this->image.memory,0);
        ANTH_LOGI("Image created");
        return true;
    }

    bool AnthemImageContainer::recordPipelineBarrier(VkCommandBuffer* cmdBuf, AnthemImagePipelineBarrier* src, AnthemImagePipelineBarrier* dst, VkImageAspectFlags aspectFlag) {
        VkImageMemoryBarrier mb = {};
        mb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        mb.image = this->image.image;
        mb.subresourceRange.aspectMask = aspectFlag;
        mb.subresourceRange.baseArrayLayer = 0;
        mb.subresourceRange.layerCount = 1;
        mb.subresourceRange.baseMipLevel = 0;
        mb.subresourceRange.levelCount = this->image.mipmapLodLevels;

        mb.srcAccessMask = src->access;
        mb.srcQueueFamilyIndex = src->queueFamily;
        mb.oldLayout = src->layout;
        mb.dstAccessMask = dst->access;
        mb.dstQueueFamilyIndex = dst->queueFamily;
        mb.newLayout = dst->layout;

        mb.pNext = nullptr;
        vkCmdPipelineBarrier(*cmdBuf, src->stage, dst->stage, 0, 0, nullptr, 0, nullptr, 1, &mb);
        return true;
    }

    uint32_t AnthemImageContainer::getImageWidth(){
        return this->image.imageInfo.extent.width;
    }
    uint32_t AnthemImageContainer::getImageHeight(){
        return this->image.imageInfo.extent.height;
    }
    uint32_t AnthemImageContainer::getImageDepth() {
        return this->image.imageInfo.extent.depth;
    }
    const VkImageView* AnthemImageContainer::getImageView() const{
        return &(this->image.imageView);
    }
    const VkImageView* AnthemImageContainer::getImageViewFb() const {
        if (this->image.imageViewFb == nullptr) {
            ANTH_LOGE("Invalid image view");
        }
        return &(this->image.imageViewFb);
    }
}