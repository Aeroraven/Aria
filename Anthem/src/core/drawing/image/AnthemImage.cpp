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
    bool AnthemImage::setImageFormat(AnthemImageFormat format){
        this->desiredFormat = format;
        return true;
    }
    bool AnthemImage::setImageSize(uint32_t width, uint32_t height){
        this->width = width;
        this->height = height;
        return true;
    }
    bool AnthemImage::prepareImage(){
        ANTH_ASSERT(this->definedUsage != AT_IU_UNDEFINED,"Image usage not specified");
        ANTH_ASSERT(this->desiredFormat != AT_IF_UNDEFINED, "Image format should not be empty");

        VkFormat pendingFormat;
        if(this->desiredFormat == AT_IF_SRGB_UINT8){
            pendingFormat = VK_FORMAT_R8G8B8A8_SRGB;
        }else if(this->desiredFormat == AT_IF_SRGB_FLOAT32){
            pendingFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        }else if(this->desiredFormat == AT_IF_SBGR_UINT8){
            pendingFormat = VK_FORMAT_B8G8R8A8_SRGB;
        }
        if (this->definedUsage == AT_IU_TEXTURE2D){
            this->createStagingBuffer();
            this->createImageInternal( VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pendingFormat, this->width, this->height);
            ANTH_LOGI("Image created");
            this->createImageTransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            this->copyBufferToImage();
            ANTH_LOGI("Buffer copied");
            if(this->image.mipmapLodLevels > 1){
                this->generateMipmap(this->width,this->height);
                ANTH_LOGI("Generated Mipmap");
            }else{
                this->createImageTransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                ANTH_LOGI("Skip generating Mipmap");
            }
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
            this->createSampler();
            this->destroyStagingBuffer();
        }else if(this->definedUsage == AT_IU_COLOR_ATTACHMENT){
            if(this->msaaOn){
                this->createImageInternal(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT , pendingFormat, this->width, this->height);
            }else{
                this->createImageInternal(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT , pendingFormat, this->width, this->height);
            }
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
            this->createSampler();
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
        ANTH_LOGI("Mipmap Level",mipLevels);
        return true;
    }
    bool AnthemImage::enableMsaa(){
        this->msaaOn = true;
        this->image.msaaCount = this->phyDevice->getMaxSampleCount();
        return true;
    }

    uint32_t AnthemImage::calculateBufferSize(){
        return this->height*this->width*this->channels;
    }
    const VkImageView* AnthemImage::getImageView() const{
        return AnthemImageContainer::getImageView();
    }

    bool AnthemImage::specifyUsage(AnthemImageUsage usage){
        this->definedUsage = usage;
        return true;
    }
    uint32_t AnthemImage::getWidth() const{
        return this->width;
    }
    uint32_t AnthemImage::getHeight() const{
        return this->height;
    }
}