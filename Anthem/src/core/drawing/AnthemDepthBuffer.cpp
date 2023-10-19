#include "../../../include/core/drawing/AnthemDepthBuffer.h"

namespace Anthem::Core{
    bool AnthemDepthBuffer::createImageInternal(){
        //Filling Image Info
        this->imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        this->imageInfo.imageType = VK_IMAGE_TYPE_2D;
        this->imageInfo.extent.width = this->swapChain->getSwapChainExtentWidth();
        this->imageInfo.extent.height = this->swapChain->getSwapChainExtentHeight();
        this->imageInfo.extent.depth = 1;
        this->imageInfo.mipLevels = 1;
        this->imageInfo.arrayLayers = 1;
        this->imageInfo.format = this->depthFormat;
        this->imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        this->imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        this->imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        this->imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        this->imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        this->imageInfo.flags = 0;

        auto creatImageRes = vkCreateImage(this->logicalDevice->getLogicalDevice(),&(this->imageInfo),nullptr,&(this->depthImage));
        ANTH_ASSERT(creatImageRes==VK_SUCCESS,"Failed to create image");

        //Create Image Memory
        VkMemoryRequirements memReq = {};
        vkGetImageMemoryRequirements(this->logicalDevice->getLogicalDevice(),this->depthImage,&memReq);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = this->phyDevice->findMemoryType(memReq.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        auto allocMemRes = vkAllocateMemory(this->logicalDevice->getLogicalDevice(),&allocInfo,nullptr,&(this->depthImageMemory));
        ANTH_ASSERT(allocMemRes==VK_SUCCESS,"Failed to allocate memory for image");

        vkBindImageMemory(this->logicalDevice->getLogicalDevice(),this->depthImage, this->depthImageMemory,0);
        ANTH_LOGI("Image created");
        return true;
    }
    bool AnthemDepthBuffer::createImageViewInternal(){
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->depthImage;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format =  this->imageInfo.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.levelCount = 1;
        auto result = vkCreateImageView(this->logicalDevice->getLogicalDevice(),&createInfo,nullptr,&(this->depthImageView));
        if(result != VK_SUCCESS){
            ANTH_LOGI("Failed to create image view",result);
            return false;
        }
        ANTH_LOGI("Image view created");
        return true;
    }

    bool AnthemDepthBuffer::createImageTransitionLayout(VkImageLayout oldLayout,VkImageLayout newLayout){
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
        barrier.image = this->depthImage;
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

    bool AnthemDepthBuffer::createDepthBuffer(){
        this->createImageInternal();
        this->createImageViewInternal();
        this->createImageTransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        return true;
    }
    bool AnthemDepthBuffer::destroyDepthBuffer(){
        vkDestroyImageView(this->logicalDevice->getLogicalDevice(),this->depthImageView,nullptr);
        vkDestroyImage(this->logicalDevice->getLogicalDevice(),this->depthImage,nullptr);
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->depthImageMemory,nullptr);
        return true;
    }
}