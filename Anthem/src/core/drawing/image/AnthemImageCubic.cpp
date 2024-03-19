#include "../../../../include/core/drawing/image/AnthemImageCubic.h"

namespace Anthem::Core {
	uint32_t AnthemImageCubic::calculateBufferSize() {
		return this->height * this->width * this->channels * this->depth * 6;
	}
	bool AnthemImageCubic::destroyStagingBuffer() {
		vkDestroyBuffer(this->logicalDevice->getLogicalDevice(), this->stagingBuffer.buffer, nullptr);
		vkFreeMemory(this->logicalDevice->getLogicalDevice(), this->stagingBuffer.bufferMem, nullptr);
		return true;
	}
	bool AnthemImageCubic::createStagingBuffer() {
		VkDeviceSize dvSize = this->calculateBufferSize();
		createBufferInternalUt(this->logicalDevice, this->phyDevice,
			&stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vkMapMemory(this->logicalDevice->getLogicalDevice(), stagingBuffer.bufferMem, 0, dvSize, 0, &stagingBuffer.mappedMem);
		memcpy(stagingBuffer.mappedMem, this->rawImageData, dvSize);
		vkUnmapMemory(this->logicalDevice->getLogicalDevice(), stagingBuffer.bufferMem);
		return true;
	}
	bool AnthemImageCubic::copyBufferToImage() {
		std::vector<VkBufferImageCopy> copyRegions;
		uint32_t cmdIdx;
		cmdBufs->createCommandBuffer(&cmdIdx);
		for (auto i : std::views::iota(0, 6)) {
			VkBufferImageCopy region = {};
			region.imageExtent = { this->width,this->height,this->depth };
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = i;
			region.imageSubresource.layerCount = 1;
			region.bufferOffset = this->height * this->width * this->channels * this->depth * i;
			copyRegions.push_back(region);
		}
		cmdBufs->startCommandRecording(cmdIdx);
		vkCmdCopyBufferToImage(*this->cmdBufs->getCommandBuffer(cmdIdx),
			this->stagingBuffer.buffer, this->image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, copyRegions.data());
		cmdBufs->endCommandRecording(cmdIdx);
		cmdBufs->submitTaskToGraphicsQueue(cmdIdx, true);
		cmdBufs->freeCommandBuffer(cmdIdx);
		return true;
	}

	AnthemImageCubic::AnthemImageCubic() {
		this->image.desiredLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		this->image.extraFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		this->image.isCubic = true;
		this->image.layerCounts = 6;
	}
	bool AnthemImageCubic::loadImageData(std::array<uint8_t*, 6>data, uint32_t width, uint32_t height, uint32_t channels) {
		this->rawImageData = new uint8_t[width * height * channels * 6];
		uint32_t offsets = 0;
		uint32_t blockSize = width * height * channels;
		for (int i = 0; i < 6; i++) {
			memcpy(this->rawImageData + offsets, data[i], blockSize);
			offsets += blockSize;
		}
		this->width = width;
		this->height = height;
		this->channels = channels;
		return true;
	}
	bool AnthemImageCubic::setImageSize(uint32_t width, uint32_t height) {
		ANTH_LOGE("Not supported");
		return false;
	}
	bool AnthemImageCubic::setImageFormat(AnthemImageFormat format) {
		this->desiredFormat = format;
		return true;
	}
	bool AnthemImageCubic::specifyUsage(AnthemImageUsage usage) {
		this->definedUsage = usage;
		return true;
	}
	bool AnthemImageCubic::prepareImage() {
		ANTH_ASSERT(this->definedUsage == AT_IU_TEXTURE, "Not supported currently");
		auto pendingFormat = AnthemImageInfoProcessing::getPendingFormat(this->desiredFormat);
		this->createStagingBuffer();
		this->createImageInternal(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pendingFormat, this->width, this->height, this->depth);
		this->createImageTransitionLayoutLegacy(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		this->copyBufferToImage();
		this->createImageTransitionLayoutLegacy(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		this->createImageViewInternal(VK_IMAGE_ASPECT_COLOR_BIT, this->depth > 1);
		this->createSampler();
		this->destroyStagingBuffer();
		return true;
	}
	bool AnthemImageCubic::destroyImage() {
		if (this->samplerCreated) {
			vkDestroySampler(this->logicalDevice->getLogicalDevice(), this->sampler, nullptr);
		}
		else {
			ANTH_LOGI("Sampler not created, skipping");
		}
		this->destroyImageViewInternal();
		this->destroyImageInternal();
		return true;
	}
	bool AnthemImageCubic::addAccessStage(uint32_t stageFlag) {
		this->image.reqStageFlags |= stageFlag;
		return true;
	}
	const VkImageView* AnthemImageCubic::getImageView() const {
		return AnthemImageContainer::getImageView();
	}
	uint32_t AnthemImageCubic::getWidth() const {
		return this->width;
	}
	uint32_t AnthemImageCubic::getHeight() const {
		return this->height;
	}
	uint32_t AnthemImageCubic::getDepth() const {
		return this->depth;
	}
	uint32_t AnthemImageCubic::getLayers() const {
		return this->image.layerCounts;
	}
}
