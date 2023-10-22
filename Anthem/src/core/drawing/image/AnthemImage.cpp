#include "../../../../include/core/drawing/image/AnthemImage.h"

namespace Anthem::Core{
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
        ANTH_ASSERT(this->definedUsage != AT_IU_UNDEFINED,"Image usage not specified");
        if (this->definedUsage == AT_IU_TEXTURE2D){
            this->createStagingBuffer();
            this->createImageInternal(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_SRGB, this->width, this->height);
            ANTH_LOGI("Image created");
            this->createImageTransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            this->copyBufferToImage();
            ANTH_LOGI("Buffer copied");
            if(this->image.mipmapLodLevels > 1){
                this->generateMipmap(this->width,this->height);
            }else{
                this->createImageTransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
            this->createSampler();
            this->destroyStagingBuffer();
        }else if(this->definedUsage == AT_IU_COLOR_ATTACHMENT){
            this->createImageInternal(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_FORMAT_R8G8B8A8_SRGB, this->width, this->height);
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
        }
        return true;
    }
    bool AnthemImage::createStagingBuffer(){
        VkDeviceSize dvSize = this->width*this->height*this->channels;
        createBufferInternalUt(this->logicalDevice,this->phyDevice,
            &stagingBuffer,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(this->logicalDevice->getLogicalDevice(),stagingBuffer.bufferMem,0,dvSize,0,&stagingBuffer.mappedMem);
        memcpy(stagingBuffer.mappedMem,this->rawImageData,dvSize);
        vkUnmapMemory(this->logicalDevice->getLogicalDevice(),stagingBuffer.bufferMem);
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
        if(this->samplerCreated){
            vkDestroySampler(this->logicalDevice->getLogicalDevice(),this->sampler,nullptr);
        }
        this->destroyImageViewInternal();
        this->destroyImageInternal();
        return true;
    }
    bool AnthemImage::enableMipMapping(){
        int32_t mipLevels = static_cast<int32_t>(std::floor(std::log2(std::max(this->width,this->height)))) + 1;
        this->image.mipmapLodLevels = mipLevels;
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
        if(this->image.mipmapLodLevels > 1){
            samplerInfo.maxLod = this->image.mipmapLodLevels;
        }else{
            samplerInfo.maxLod = 1.0f;
        }
        this->samplerCreated = true;
        auto samplerResult = vkCreateSampler(this->logicalDevice->getLogicalDevice(),&samplerInfo,nullptr,&(this->sampler));
        ANTH_ASSERT(samplerResult==VK_SUCCESS,"Failed to create sampler");
        return true;
    }
    uint32_t AnthemImage::calculateBufferSize(){
        return this->height*this->width*this->channels;
    }
    const VkImageView* AnthemImage::getImageView() const{
        return AnthemImageContainer::getImageView();
    }
    const VkSampler* AnthemImage::getSampler() const{
        ANTH_ASSERT(this->samplerCreated,"Sampler not created");
        return &(this->sampler);
    }
    bool AnthemImage::specifyUsage(AnthemImageUsage usage){
        this->definedUsage = usage;
        return true;
    }
}