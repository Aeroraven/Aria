#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemLogicalDevice.h"
#include "AnthemShaderModule.h"
#include "AnthemViewport.h"

namespace Anthem::Core{
    class AnthemGraphicsPipeline{     
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        std::vector<VkDynamicState> reqiredDynamicStates={
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};

        const AnthemViewport* viewport = nullptr;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo = {};
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};

        bool prerequisiteInfoSpecified = false;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        VkPipelineLayout pipelineLayout = {};

    public:
        bool specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool specifyViewport(const AnthemViewport* viewport);
        bool preparePreqPipelineCreateInfo();
        bool createPipelineLayout();
        bool destroyPipelineLayout();
    };
}