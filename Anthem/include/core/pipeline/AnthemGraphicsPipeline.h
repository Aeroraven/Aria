#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemLogicalDevice.h"
#include "AnthemShaderModule.h"
#include "AnthemViewport.h"
#include "AnthemRenderPass.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "../drawing/buffer/AnthemVertexBuffer.h"
#include "../drawing/buffer/AnthemUniformBuffer.h"

namespace Anthem::Core{
    enum AnthemDescriptorSetEntrySourceType{
        AT_ACDS_UNDEFINED = 0,
        AT_ACDS_UNIFORM_BUFFER = 1,
        AT_ACDS_SAMPLER = 2,
    };
    struct AnthemDescriptorSetEntry{
        AnthemDescriptorPool* descPool = nullptr;
        AnthemDescriptorSetEntrySourceType descSetType = AT_ACDS_UNDEFINED;
        uint32_t inTypeIndex = 0;
    };
    class AnthemGraphicsPipeline{     
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        const AnthemShaderModule* shaderModule = nullptr;
        const AnthemViewport* viewport = nullptr;
        const AnthemRenderPass* renderPass = nullptr;
    
        AnthemVertexBuffer* vertexBuffer = nullptr;
        AnthemUniformBuffer* uniformBuffer = nullptr;
        AnthemDescriptorPool* descriptorPool = nullptr;

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
        
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentState = {};
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo = {};

        bool prerequisiteInfoSpecified = false;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        VkPipelineLayout pipelineLayout = {};

        bool pipelineCreated = false;
        VkPipeline pipeline;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        std::vector<VkDescriptorSetLayout> layouts;

    public:
        bool specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool specifyViewport(const AnthemViewport* viewport);
        bool specifyRenderPass(const AnthemRenderPass* renderPass);
        bool specifyShaderModule(const AnthemShaderModule* shaderModule);
        bool specifyVertexBuffer(AnthemVertexBuffer* vertexBuffer);
        bool specifyUniformBuffer(AnthemUniformBuffer* uniformBuffer);
        bool specifyDescriptor(AnthemDescriptorPool* pool);


        bool preparePreqPipelineCreateInfo();
        bool createPipelineLayout();
        bool createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry);
        bool destroyPipelineLayout();
        
        bool createPipeline();
        bool destroyPipeline();

        const VkPipeline* getPipeline() const;
        const VkPipelineLayout* getPipelineLayout() const;
    };
}