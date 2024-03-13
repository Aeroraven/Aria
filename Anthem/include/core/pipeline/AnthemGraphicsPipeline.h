#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemLogicalDevice.h"
#include "AnthemShaderModule.h"
#include "AnthemViewport.h"
#include "AnthemRenderPass.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "../drawing/buffer/AnthemVertexBuffer.h"
#include "../drawing/buffer/AnthemUniformBuffer.h"
#include "../drawing/buffer/AnthemVertexStageLayoutSpec.h"
#include "../drawing/buffer/AnthemPushConstant.h"
#include "./AnthemDescriptorSetEntry.h"

namespace Anthem::Core{
    enum class AnthemInputAssemblerTopology {
        AT_AIAT_UNDEFINED = 0,
        AT_AIAT_TRIANGLE_LIST = 1,
        AT_AIAT_POINT_LIST = 2,
        AT_AIAT_LINE = 3,
        AT_AIAT_PATCH_LIST = 4
    };

    enum class AnthemBlendPreset {
        AT_ABP_NO_BLEND = 0,
        AT_ABP_MANUAL = 1,
        AT_ABP_DEFAULT_TRANSPARENCY = 2,
        AT_ABP_WEIGHTED_BLENDED_ACCUM = 3,
        AT_ABP_WEIGHTED_BLENDED_REVEAL = 4
    };

    enum class AnthemRasterizerPolygonMode {
        AT_ARPM_POLYGON_FILL = 1,
        AT_ARPM_WIREFRAME = 2
    };

    struct AnthemGraphicsPipelineCreateProps {
        AnthemInputAssemblerTopology inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
        std::optional<std::vector<IAnthemVertexBufferAttrLayout*>> vertStageLayout = std::nullopt;
        std::vector<AnthemBlendPreset> blendPreset = { AnthemBlendPreset::AT_ABP_NO_BLEND };
        bool writeDepthStencil = true;

        // Tessellation
        bool enableTessellation = false;
        uint32_t patchControlPoints = 0;

        // Rasterizer
        AnthemRasterizerPolygonMode polygonMode = AnthemRasterizerPolygonMode::AT_ARPM_POLYGON_FILL;

        // Mesh Shader
        bool emptyVertexStage = false;
    };

    class AnthemGraphicsPipeline{     
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        const AnthemShaderModule* shaderModule = nullptr;
        const AnthemViewport* viewport = nullptr;
        const AnthemRenderPass* renderPass = nullptr;
    
        IAnthemVertexBufferAttrLayout* vertexBuffer = nullptr;
        AnthemUniformBuffer* uniformBuffer = nullptr;
        AnthemDescriptorPool* descriptorPool = nullptr;
        AnthemGraphicsPipelineCreateProps extraProps = {};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        std::vector<VkDynamicState> reqiredDynamicStates={
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
        };
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};

        
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo = {};
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentState = {};
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
        VkPipelineTessellationStateCreateInfo tessStateCreateInfo = {};

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo = {};

        bool prerequisiteInfoSpecified = false;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        VkPipelineLayout pipelineLayout = {};

        bool pipelineCreated = false;
        VkPipeline pipeline;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        std::vector<VkDescriptorSetLayout> layouts;

        AnthemVertexStageLayoutSpec vxLayoutSpec = {};
    protected:
        bool loadCustomizedVertexStageLayout();

    public:
        bool specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool specifyViewport(const AnthemViewport* viewport);
        bool specifyRenderPass(const AnthemRenderPass* renderPass);
        bool specifyShaderModule(const AnthemShaderModule* shaderModule);
        bool specifyVertexBuffer(IAnthemVertexBufferAttrLayout* vertexBuffer);
        bool specifyUniformBuffer(AnthemUniformBuffer* uniformBuffer);
        bool specifyDescriptor(AnthemDescriptorPool* pool);
        bool specifyProps(AnthemGraphicsPipelineCreateProps* props);


        bool preparePreqPipelineCreateInfo();
        bool createPipelineLayout();
        bool createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry,const std::vector<AnthemPushConstant*> pushConsts);
        bool destroyPipelineLayout();
        
        bool createPipeline();
        bool destroyPipeline();

        const VkPipeline* getPipeline() const;
        const VkPipelineLayout* getPipelineLayout() const;
    };
}