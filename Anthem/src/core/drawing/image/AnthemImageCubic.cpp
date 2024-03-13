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
	}
}
