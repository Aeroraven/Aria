#include "../../../include/core/drawing/AnthemImage.h"

namespace Anthem::Core{
    bool AnthemImage::createDescriptorPool(uint32_t maxSets){
        return this->createDescriptorPoolInternal(this->logicalDevice,maxSets,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }
    bool AnthemImage::loadImageData(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels){
        this->rawImageData = new char[width*height*channels];
        ANTH_LOGV("Before Memcpy Sz=", width*height*channels, " PTR=",(long long)(data));
        memcpy(this->rawImageData,data,width*height*channels);
        ANTH_LOGV("After Memcpy");
        this->width = width;
        this->height = height;
        this->channels = channels;
        ANTH_TODO("Check if image is 4 channel");
        ANTH_ASSERT(this->channels==4,"Image channels must be 4");
        return true;
    }
    bool AnthemImage::prepareImage(){
        this->createStagingBuffer();
        this->createImageInternal();
        ANTH_LOGI("Image created");
        this->createImageTransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        this->copyBufferToImage();
        ANTH_LOGI("Buffer copied");
        this->createImageTransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        this->createImageView();
        this->createSampler();
        this->destroyStagingBuffer();
        return true;
    }
    bool AnthemImage::createStagingBuffer(){
        VkDeviceSize dvSize = this->width*this->height*this->channels;
        createBufferInternal(&stagingBuffer,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(this->logicalDevice->getLogicalDevice(),stagingBuffer.bufferMem,0,dvSize,0,&stagingBuffer.mappedMem);
        memcpy(stagingBuffer.mappedMem,this->rawImageData,dvSize);
        vkUnmapMemory(this->logicalDevice->getLogicalDevice(),stagingBuffer.bufferMem);
        return true;
    }
    bool AnthemImage::createImageInternal(){
        //Filling Image Info
        this->image.imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        this->image.imageInfo.imageType = VK_IMAGE_TYPE_2D;
        this->image.imageInfo.extent.width = this->width;
        this->image.imageInfo.extent.height = this->height;
        this->image.imageInfo.extent.depth = 1;
        this->image.imageInfo.mipLevels = 1;
        this->image.imageInfo.arrayLayers = 1;
        this->image.imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        this->image.imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        this->image.imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        this->image.imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        this->image.imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        this->image.imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        this->image.imageInfo.flags = 0;

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
    bool AnthemImage::createImageTransitionLayout(VkImageLayout oldLayout,VkImageLayout newLayout){
        uint32_t cmdBufIdx;
        this->cmdBufs->createCommandBuffer(&cmdBufIdx);
        this->cmdBufs->startCommandRecording(cmdBufIdx);
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = this->image.image;
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(*(this->cmdBufs->getCommandBuffer(cmdBufIdx)),sourceStage, destinationStage,
            0,0, nullptr,0, nullptr,1, &barrier);  

        this->cmdBufs->endCommandRecording(cmdBufIdx);  
        this->cmdBufs->submitTaskToGraphicsQueue(cmdBufIdx,true);
        this->cmdBufs->freeCommandBuffer(cmdBufIdx);
        ANTH_LOGI("Image pipeline barrier created, transition done");
        return true;
    }
    bool AnthemImage::copyBufferToImage(){
        uint32_t cmdBufIdx;
        this->cmdBufs->createCommandBuffer(&cmdBufIdx);
        this->cmdBufs->startCommandRecording(cmdBufIdx);
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0,0,0};
        region.imageExtent = {this->width,this->height,1};
        vkCmdCopyBufferToImage(*(this->cmdBufs->getCommandBuffer(cmdBufIdx)),this->stagingBuffer.buffer,this->image.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
        
        this->cmdBufs->endCommandRecording(cmdBufIdx);
        this->cmdBufs->submitTaskToGraphicsQueue(cmdBufIdx,true);
        this->cmdBufs->freeCommandBuffer(cmdBufIdx);
        ANTH_LOGI("Image copied from buffer");
        return true;
    }
    bool AnthemImage::destroyStagingBuffer(){
        vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.buffer,nullptr);
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.bufferMem,nullptr);
        return true;
    }
    bool AnthemImage::destroyImage(){
        vkDestroySampler(this->logicalDevice->getLogicalDevice(),this->sampler,nullptr);
        vkDestroyImageView(this->logicalDevice->getLogicalDevice(),this->imageView,nullptr);
        vkDestroyImage(this->logicalDevice->getLogicalDevice(),this->image.image,nullptr);
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->image.memory,nullptr);
        return true;
    }
    bool AnthemImage::createImageView(){
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->image.image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format =  this->image.imageInfo.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.levelCount = 1;
        auto result = vkCreateImageView(this->logicalDevice->getLogicalDevice(),&createInfo,nullptr,&(this->imageView));
        if(result != VK_SUCCESS){
            ANTH_LOGI("Failed to create image view",result);
            return false;
        }
        ANTH_LOGI("Image view created");
        return true;
    }
    bool AnthemImage::createSampler(){
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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
        samplerInfo.maxLod = 0.0f;

        auto samplerResult = vkCreateSampler(this->logicalDevice->getLogicalDevice(),&samplerInfo,nullptr,&(this->sampler));
        ANTH_ASSERT(samplerResult==VK_SUCCESS,"Failed to create sampler");
        return true;
    }
    uint32_t AnthemImage::calculateBufferSize(){
        return this->height*this->width*this->channels;
    }
    const VkImageView* AnthemImage::getImageView() const{
        return &(this->imageView);
    }
    const VkSampler* AnthemImage::getSampler() const{
        return &(this->sampler);
    }

}