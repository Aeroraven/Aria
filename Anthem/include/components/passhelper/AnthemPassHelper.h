#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"
#include "../../core/utils/AnthemUtlTimeOps.h"
#include "../camera/AnthemCamera.h"

namespace Anthem::Components::PassHelper {
	using namespace Anthem::Core;

	class AnthemPassHelper {
	protected:
		AnthemSimpleToyRenderer* rd;
		AnthemRenderPass* pass;
		AnthemShaderModule* shader;

		AnthemDescriptorPool** descDepth;
		AnthemDepthBuffer** depth;

		std::vector<std::vector<const IAnthemImageViewContainer*>> renderTargets;
		
		AnthemSwapchainFramebuffer* swapchainFb;
		AnthemGraphicsPipeline* pipeline;

		std::vector<AnthemFramebuffer*> frameBuffers;
		std::vector<std::vector<AnthemDescriptorSetEntry>> descLayout;
		uint32_t* cmdIdx;
		uint32_t copies;

	public:
		bool enableDepthSampler = false;
		AnthemShaderFilePaths shaderPath;
		AnthemGraphicsPipelineCreateProps pipeOpt;
		AnthemRenderPassSetupOption passOpt;
		IAnthemVertexBufferAttrLayout* vxLayout = nullptr;


		
	public:
		AnthemPassHelper(AnthemSimpleToyRenderer* renderer, uint32_t copies);
		void setRenderTargets(const std::vector<const IAnthemImageViewContainer*>& targets, int destCopy = -1);
		void setDescriptorLayouts(const std::vector<AnthemDescriptorSetEntry>& layouts, int destCopy = -1);
		void buildGraphicsPipeline();
		void recordCommands(std::function<void(uint32_t)> injectedCommands);
		
		uint32_t getCommandIndex(uint32_t copy);
		AnthemDescriptorPool* getDepthDescriptor(uint32_t id);
	};
}