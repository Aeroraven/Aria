#include "../../../../include/core/drawing/buffer/AnthemIndirectDrawBuffer.h"

namespace Anthem::Core {
	uint32_t AnthemIndirectDrawBuffer::calculateBufferSize() {
		return this->indirectCmds.size() * sizeof(VkDrawIndexedIndirectCommand);
	}
	bool AnthemIndirectDrawBuffer::addIndirectDrawCommand(uint32_t instanceNum, uint32_t firstInstance, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset) {
		VkDrawIndexedIndirectCommand drCmd;
		drCmd.firstIndex = firstIndex;
		drCmd.indexCount = indexCount;
		drCmd.vertexOffset = vertexOffset;
		drCmd.instanceCount = instanceNum;
		drCmd.firstInstance = firstInstance;
		this->indirectCmds.push_back(drCmd);
		return true;
	}
	bool AnthemIndirectDrawBuffer::createBuffer() {
		auto bufSize = calculateBufferSize();
		uint32_t cmdIdx;
		AnthemGeneralBufferProp staging;
		this->createBufferInternal(&staging,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		void* mapAddr;
		vkMapMemory(this->logicalDevice->getLogicalDevice(), staging.bufferMem, 0, bufSize, 0, &mapAddr);
		memcpy(mapAddr, this->indirectCmds.data(), bufSize);
		vkUnmapMemory(this->logicalDevice->getLogicalDevice(), staging.bufferMem);
		
		this->createBufferInternal(&indirect,
			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			
		VkBufferCopy cpInfo = {};
		cpInfo.size = bufSize;
		
		this->cmdBufs->createCommandBuffer(&cmdIdx);
		this->cmdBufs->startCommandRecording(cmdIdx);
		vkCmdCopyBuffer(*this->cmdBufs->getCommandBuffer(cmdIdx), staging.buffer, indirect.buffer, 1, &cpInfo);
		this->cmdBufs->endCommandRecording(cmdIdx);
		this->cmdBufs->submitTaskToGraphicsQueue(cmdIdx, true);

		vkFreeMemory(logicalDevice->getLogicalDevice(), staging.bufferMem, nullptr);
		vkDestroyBuffer(logicalDevice->getLogicalDevice(), staging.buffer, nullptr);
		created = true;
		return true;
	}

	bool  AnthemIndirectDrawBuffer::destroyBuffer() {
		if (created) {
			vkFreeMemory(logicalDevice->getLogicalDevice(), indirect.bufferMem, nullptr);
			vkDestroyBuffer(logicalDevice->getLogicalDevice(), indirect.buffer, nullptr);
			return true;
		}
		return false;
	}

	uint32_t AnthemIndirectDrawBuffer::getNumCommands() const {
		return this->indirectCmds.size();
	}
	const VkBuffer* AnthemIndirectDrawBuffer::getBuffer() const {
		ANTH_ASSERT(this->created, "Indirect buffer not created");
		auto p = &(this->indirect.buffer);
		return p;
	}

}