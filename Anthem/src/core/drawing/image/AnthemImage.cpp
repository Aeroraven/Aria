#include "../../../../include/core/drawing/image/AnthemImage.h"

namespace Anthem::Core{
    bool AnthemImage::loadImageData(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels){
        this->rawImageData = new char[width*height*channels];
        if (data == nullptr) {
            ANTH_LOGW("Data is nullptr, skip loading image data.");
        }
        else {
            memcpy(this->rawImageData, data, width * height * channels);
        }
        this->width = width;
        this->height = height;
        this->channels = channels;
        return true;
    }
    bool AnthemImage::loadImageData3(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels,uint32_t depth) {
        this->rawImageData = new char[width * height * depth * channels];
        if (data == nullptr) {
            ANTH_LOGW("Data is nullptr, skip loading image data.");
        }
        else {
            memcpy(this->rawImageData, data, width * height * depth * channels);
        }
        this->width = width;
        this->height = height;
        this->depth = depth;
        this->channels = channels;
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
    bool AnthemImage::setImageSize3(uint32_t width, uint32_t height, uint32_t depth) {
        this->width = width;
        this->height = height;
        this->depth = depth;
        return true;
    }
    bool AnthemImage::prepareImage(){
        ANTH_ASSERT(this->definedUsage != AT_IU_UNDEFINED,"Image usage not specified");
        ANTH_ASSERT(this->desiredFormat != AT_IF_UNDEFINED, "Image format should not be empty");

        VkFormat pendingFormat = AnthemImageInfoProcessing::getPendingFormat(this->desiredFormat, this->swapchain);
        if (this->definedUsage == AT_IU_TEXTURE){
            
            this->createStagingBuffer();
            this->createImageInternal( VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pendingFormat, this->width, this->height, this->depth);
            this->createImageTransitionLayoutLegacy(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            this->copyBufferToImage();
            if(this->image.mipmapLodLevels > 1){
                this->generateMipmap2D(this->width,this->height);
            }else{
                this->createImageTransitionLayoutLegacy(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT,this->depth>1);
            this->createSampler();
            this->destroyStagingBuffer();
        }else if(this->definedUsage == AT_IU_COLOR_ATTACHMENT){
            if(this->msaaOn){
                this->createImageInternal(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT , pendingFormat, this->width, this->height,this->depth);
            }else{
                this->createImageInternal(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT , pendingFormat, this->width, this->height, this->depth);
            }
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
            this->createSampler();
        }
        else if (this->definedUsage == AT_IU_COMPUTE_OUTPUT) {
            this->createImageInternal(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pendingFormat, this->width, this->height, this->depth);
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
            this->createSampler();
            this->image.reqStageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        else if (this->definedUsage == AT_IU_RAYTRACING_DEST) {
            this->createImageInternal(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pendingFormat, this->width, this->height, this->depth);
            this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT);
        }
        return true;
    }
    bool AnthemImage::createStagingBuffer(){
        VkDeviceSize dvSize = this->width*this->height*this->channels*this->depth;
        createBufferInternalUt(this->logicalDevice,this->phyDevice,
            &stagingBuffer,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(this->logicalDevice->getLogicalDevice(),stagingBuffer.bufferMem,0,dvSize,0,&stagingBuffer.mappedMem);
        memcpy(stagingBuffer.mappedMem,this->rawImageData,dvSize);
        vkUnmapMemory(this->logicalDevice->getLogicalDevice(),stagingBuffer.bufferMem);
        return true;
    }

    bool AnthemImage::addAccessStage(uint32_t stageFlag) {
        this->image.reqStageFlags |= stageFlag;
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
        region.imageExtent = {this->width,this->height,this->depth};
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
        }else{
            ANTH_LOGI("Sampler not created, skipping");
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
        return this->height*this->width*this->channels*this->depth;
    }
    const VkImageView* AnthemImage::getImageView() const{
        return AnthemImageContainer::getImageView();
    }
    const VkImage* AnthemImage::getImage() const {
        return &this->image.image;
    }
    bool AnthemImage::specifyUsage(AnthemImageUsage usage){
        this->definedUsage = usage;
        return true;
    }
    bool AnthemImage::specifySwapchain(AnthemSwapChain* sc) {
        this->swapchain = sc;
        return true;
    }
    uint32_t AnthemImage::getWidth() const{
        return this->width;
    }
    uint32_t AnthemImage::getHeight() const{
        return this->height;
    }
    uint32_t AnthemImage::getDepth() const {
        return this->depth;
    }
    uint32_t AnthemImage::getLayers() const {
        return this->image.layerCounts;
    }
    uint32_t AnthemImage::getMipLevels() const {
        return this->image.mipmapLodLevels;
    }
}