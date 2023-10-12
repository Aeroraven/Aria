#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemLogicalDevice.h"
#include "AnthemShaderModule.h"
#include "AnthemViewport.h"
#include "AnthemRenderPass.h"

namespace Anthem::Core{
    class AnthemGraphicsPipeline{     
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        const AnthemShaderModule* shaderModule = nullptr;
        const AnthemViewport* viewport = nullptr;
        const AnthemRenderPass* renderPass = nullptr;

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        std::vector<VkDynamicState> reqiredDynamicStates={
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};

        
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo = {};
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo = {};

        bool prerequisiteInfoSpecified = false;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        VkPipelineLayout pipelineLayout = {};

        bool pipelineCreated = false;
        VkPipeline pipeline;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

    public:
        bool specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool specifyViewport(const AnthemViewport* viewport);
        bool specifyRenderPass(const AnthemRenderPass* renderPass);
        bool specifyShaderModule(const AnthemShaderModule* shaderModule);

        bool preparePreqPipelineCreateInfo();
        bool createPipelineLayout();
        bool destroyPipelineLayout();
        
        bool createPipeline();
        bool destroyPipeline();

        const VkPipeline* getPipeline() const;
    };
}