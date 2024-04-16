#include "../../../include/core/pipeline/AnthemGraphicsPipeline.h"

namespace Anthem::Core{
    bool AnthemGraphicsPipeline::specifyLogicalDevice(const AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyViewport(const AnthemViewport* viewport){
        this->viewport = viewport;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyRenderPass(const AnthemRenderPass* renderPass){
        this->renderPass = renderPass;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyShaderModule(const AnthemShaderModule* shaderModule){
        this->shaderModule = shaderModule;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyVertexBuffer(IAnthemVertexBufferAttrLayout* vertexBuffer){
        this->vertexBuffer = vertexBuffer;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyUniformBuffer(AnthemUniformBuffer* uniformBuffer){
        this->uniformBuffer = uniformBuffer;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyDescriptor(AnthemDescriptorPool* pool){
        this->descriptorPool = pool;
        return true;
    }
    bool AnthemGraphicsPipeline::specifyProps(AnthemGraphicsPipelineCreateProps* props){
        this->extraProps = *props;
        return true;
    }
    bool AnthemGraphicsPipeline::loadCustomizedVertexStageLayout() {
        auto w = this->extraProps.vertStageLayout.value();
        for (int i = 0; i < w.size(); i++) {
            w[i]->updateLayoutSpecification(&this->vxLayoutSpec, i);
        }
        for (auto& [key, value] : vxLayoutSpec.registeredDesc) {
            vxLayoutSpec.attributeDesc.push_back(value);
        }
        this->vertexInputStateCreateInfo.vertexBindingDescriptionCount = vxLayoutSpec.bindingDesc.size();
        this->vertexInputStateCreateInfo.pVertexBindingDescriptions = vxLayoutSpec.bindingDesc.data();
        this->vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vxLayoutSpec.attributeDesc.size();
        this->vertexInputStateCreateInfo.pVertexAttributeDescriptions = vxLayoutSpec.attributeDesc.data();
        return true;
    }
    
    bool AnthemGraphicsPipeline::preparePreqPipelineCreateInfo(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->viewport != nullptr,"Viewport not specified");
        ANTH_ASSERT(this->renderPass != nullptr,"Render pass not specified");

        //Specify Dynamic States
        this->dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        this->dynamicStateCreateInfo.pNext = nullptr;
        this->dynamicStateCreateInfo.flags = 0;
        if (this->extraProps.enableDynamicStencilTesting) {
            this->reqiredDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
            this->reqiredDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
            this->reqiredDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
            this->reqiredDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
        }

        this->dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(this->reqiredDynamicStates.size());
        this->dynamicStateCreateInfo.pDynamicStates = this->reqiredDynamicStates.data();
        
        //Specify Vertex Shader Input Info
        if (this->extraProps.vertStageLayout.has_value()) {
            this->vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            this->vertexInputStateCreateInfo.pNext = nullptr;
            this->vertexInputStateCreateInfo.flags = 0;
            loadCustomizedVertexStageLayout();
        }
        else if(this->vertexBuffer==nullptr){
            ANTH_LOGW("Vertex buffer not specified, using default vertex input state");
            this->vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            this->vertexInputStateCreateInfo.pNext = nullptr;
            this->vertexInputStateCreateInfo.flags = 0;
            this->vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            this->vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
            this->vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            this->vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
        }else{
            this->vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            this->vertexInputStateCreateInfo.pNext = nullptr;
            this->vertexInputStateCreateInfo.flags = 0;
            this->vertexBuffer->prepareVertexInputInfo(&(this->vertexInputStateCreateInfo),0);
        }

        //Specify Input Assembly Info
        this->inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        this->inputAssemblyStateCreateInfo.pNext = nullptr;
        this->inputAssemblyStateCreateInfo.flags = 0;
        using topoEnum = std::remove_cvref<decltype(extraProps.inputTopo)>::type;
        if (extraProps.inputTopo == topoEnum::AT_AIAT_TRIANGLE_LIST) {
            this->inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        else if (extraProps.inputTopo == topoEnum::AT_AIAT_POINT_LIST) {
            this->inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }
        else if (extraProps.inputTopo == topoEnum::AT_AIAT_LINE) {
            this->inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
        else if (extraProps.inputTopo == topoEnum::AT_AIAT_PATCH_LIST) {
            this->inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        }
        else {
            ANTH_LOGE("Unknown topology");
        }
        this->inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        //Specify Tessellation Info
        if (extraProps.enableTessellation) {
            this->tessStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
            this->tessStateCreateInfo.patchControlPoints = extraProps.patchControlPoints;
        }

        //Specify Viewport Info
        ANTH_ASSERT(this->viewport != nullptr,"Viewport not specified");
        if (extraProps.customViewport == std::nullopt) {
            this->viewport->getViewportStateCreateInfo(&(this->viewportStateCreateInfo));
        }
        else {
            extraProps.customViewport.value()->getViewportStateCreateInfo(&(this->viewportStateCreateInfo));
        }
        

        //Specify Rasterizer Info
        this->rasterizerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        this->rasterizerStateCreateInfo.pNext = nullptr;
        this->rasterizerStateCreateInfo.flags = 0;
        this->rasterizerStateCreateInfo.depthClampEnable = VK_FALSE;
        this->rasterizerStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        if (extraProps.polygonMode == AnthemRasterizerPolygonMode::AT_ARPM_POLYGON_FILL) {
            this->rasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        }
        else if (extraProps.polygonMode == AnthemRasterizerPolygonMode::AT_ARPM_WIREFRAME) {
            this->rasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
        }
        else {
            ANTH_LOGE("Unsupported polygon fill mode");
        }

        this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        this->rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        if (extraProps.enableCullMode) {
            if (extraProps.cullMode == AnthemCullMode::AT_ACM_NONE) {
                this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
            }
            else if (extraProps.cullMode == AnthemCullMode::AT_ACM_BACK) {
                this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            }
            else if (extraProps.cullMode == AnthemCullMode::AT_ACM_FRONT) {
                this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
            }
            else if (extraProps.cullMode == AnthemCullMode::AT_ACM_FRONT_AND_BACK) {
                this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            }
            else {
                ANTH_LOGE("Unsupported cull mode");
            }
            if (extraProps.frontFace == AnthemFrontFace::AT_AFF_CLOCKWISE) {
				this->rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			}
            else if (extraProps.frontFace == AnthemFrontFace::AT_AFF_COUNTER_CLOCKWISE) {
				this->rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			}
            else {
				ANTH_LOGE("Unsupported front face");
			}
        }
        this->rasterizerStateCreateInfo.depthBiasEnable = VK_FALSE;
        this->rasterizerStateCreateInfo.depthBiasConstantFactor = 0.0f;
        this->rasterizerStateCreateInfo.depthBiasClamp = 0.0f;
        this->rasterizerStateCreateInfo.depthBiasSlopeFactor = 0.0f;
        this->rasterizerStateCreateInfo.lineWidth = 1.0f;

        //Specify Multisamplling Info
        this->multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        this->multisampleStateCreateInfo.pNext = nullptr;
        this->multisampleStateCreateInfo.flags = 0;
        if(renderPass->getSetupOption().msaaType != AT_ARPMT_MSAA){
            this->multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        }else{
            this->multisampleStateCreateInfo.rasterizationSamples = renderPass->getSetupOption().msaaSamples;
        }
        this->multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        this->multisampleStateCreateInfo.minSampleShading = 1.0f;
        this->multisampleStateCreateInfo.pSampleMask = nullptr;
        this->multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        this->multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        //Specify Color Blending Info
        auto numColorAttachments = renderPass->getFilteredAttachmentCnt(AT_ARPCA_COLOR);
        this->colorBlendAttachmentState.resize(numColorAttachments);


        for(uint32_t i=0;i<numColorAttachments;i++){
            if (this->extraProps.blendPreset[i] == AnthemBlendPreset::AT_ABP_NO_BLEND) {
                this->colorBlendAttachmentState[i].blendEnable = VK_FALSE;
            }
            else if(this->extraProps.blendPreset[i] == AnthemBlendPreset::AT_ABP_DEFAULT_TRANSPARENCY){
                this->colorBlendAttachmentState[i].blendEnable = VK_TRUE;
                this->colorBlendAttachmentState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                this->colorBlendAttachmentState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                this->colorBlendAttachmentState[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                this->colorBlendAttachmentState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                this->colorBlendAttachmentState[i].colorBlendOp = VK_BLEND_OP_ADD;
            }
            else if (this->extraProps.blendPreset[i] == AnthemBlendPreset::AT_ABP_WEIGHTED_BLENDED_ACCUM) {
                this->colorBlendAttachmentState[i].blendEnable = VK_TRUE;
                this->colorBlendAttachmentState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                this->colorBlendAttachmentState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                this->colorBlendAttachmentState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                this->colorBlendAttachmentState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                this->colorBlendAttachmentState[i].colorBlendOp = VK_BLEND_OP_ADD;
            }
            else if (this->extraProps.blendPreset[i] == AnthemBlendPreset::AT_ABP_WEIGHTED_BLENDED_REVEAL) {
                this->colorBlendAttachmentState[i].blendEnable = VK_TRUE;
                this->colorBlendAttachmentState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                this->colorBlendAttachmentState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                this->colorBlendAttachmentState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                this->colorBlendAttachmentState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                this->colorBlendAttachmentState[i].colorBlendOp = VK_BLEND_OP_ADD;
            }
            else {
                ANTH_LOGE("Blend preset not available");
            }
            this->colorBlendAttachmentState[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        this->colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        this->colorBlendStateCreateInfo.pNext = nullptr;
        this->colorBlendStateCreateInfo.flags = 0;
        this->colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        this->colorBlendStateCreateInfo.attachmentCount = static_cast<uint32_t>(colorBlendAttachmentState.size());
        this->colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentState.data();

        //Specify Depth & Stencil Info
        this->depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        this->depthStencilStateCreateInfo.pNext = nullptr;  
        this->depthStencilStateCreateInfo.flags = 0;
        if (!this->extraProps.enableDepthTestsing) {
            this->depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
            this->depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        }
        else {
            this->depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            this->depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        }
        this->depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        this->depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        this->depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        if (this->extraProps.enableDynamicStencilTesting) {
            this->depthStencilStateCreateInfo.stencilTestEnable = VK_TRUE;
        }
        this->depthStencilStateCreateInfo.front = {};
        this->depthStencilStateCreateInfo.back = {};
        this->depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        this->depthStencilStateCreateInfo.maxDepthBounds = 1.0f;


        //Setup Flag
        this->prerequisiteInfoSpecified = true;
        return true;
    }
    bool AnthemGraphicsPipeline::createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry, const std::vector<AnthemPushConstant*> pushConsts){
        // Create Pipeline
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        for(const auto& p:entry){
            if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER){
                p.descPool->appendSamplerDescriptorLayoutIdx(&layouts,p.inTypeIndex);
            }else if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER){
                p.descPool->appendUniformDescriptorLayoutIdx(&layouts,p.inTypeIndex);
            }else{
                ANTH_LOGE("Invalid layout type");
            }
        }

        this->pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
        this->pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());

        // Push Constants
        std::vector<VkPushConstantRange> constRanges;
        this->pipelineLayoutCreateInfo.pushConstantRangeCount = pushConsts.size();
        for (const auto& p : pushConsts) {
            constRanges.push_back(p->getRange());
        }
        this->pipelineLayoutCreateInfo.pPushConstantRanges = constRanges.data();

        //Create Layout
        auto result = vkCreatePipelineLayout(this->logicalDevice->getLogicalDevice(),&(this->pipelineLayoutCreateInfo),nullptr,&(this->pipelineLayout));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create pipeline layout",result);
            return false;
        }
        ANTH_LOGI("Pipeline layout created");
        return true;
    }
    bool AnthemGraphicsPipeline::createPipelineLayout(){
        //Specify Pipeline Layout Creation Info
        
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        
        this->descriptorPool->getAllDescriptorLayouts(&layouts);
        for(auto x:layouts){
            ANTH_LOGI("Layouts Are:",(long long)(x));
        }
        this->pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
        this->pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        ANTH_LOGI("Specified pipeline layout");
        
        //Create Layout
        auto result = vkCreatePipelineLayout(this->logicalDevice->getLogicalDevice(),&(this->pipelineLayoutCreateInfo),nullptr,&(this->pipelineLayout));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create pipeline layout",result);
            return false;
        }
        ANTH_LOGI("Pipeline layout created");
        return true;
    }
    bool AnthemGraphicsPipeline::destroyPipelineLayout(){
        ANTH_LOGI("Destroying pipeline layout");
        vkDestroyPipelineLayout(this->logicalDevice->getLogicalDevice(),this->pipelineLayout,nullptr);
        return true;
    }
    bool AnthemGraphicsPipeline::createPipeline(){
        ANTH_ASSERT(this->prerequisiteInfoSpecified,"Prerequisite info not specified");

        //Prepare Shader Stage Info
        this->shaderModule->specifyShaderStageCreateInfo(&shaderStageCreateInfo);

        //Specify Pipeline Creation Info
        this->pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        this->pipelineCreateInfo.pNext = nullptr;
        this->pipelineCreateInfo.flags = 0;

        this->pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());
        this->pipelineCreateInfo.pStages = shaderStageCreateInfo.data();

        if (extraProps.emptyVertexStage) {
            this->pipelineCreateInfo.pVertexInputState = nullptr;
        }
        else {
            this->pipelineCreateInfo.pVertexInputState = &(this->vertexInputStateCreateInfo);
        }
        this->pipelineCreateInfo.pInputAssemblyState = &(this->inputAssemblyStateCreateInfo);
        this->pipelineCreateInfo.pTessellationState = nullptr;
        if (extraProps.enableTessellation) {
            this->pipelineCreateInfo.pTessellationState = &(this->tessStateCreateInfo);
        }
        this->pipelineCreateInfo.pViewportState = &(this->viewportStateCreateInfo);
        this->pipelineCreateInfo.pRasterizationState = &(this->rasterizerStateCreateInfo);
        this->pipelineCreateInfo.pMultisampleState = &(this->multisampleStateCreateInfo);
        this->pipelineCreateInfo.pDepthStencilState = &(this->depthStencilStateCreateInfo);
        this->pipelineCreateInfo.pColorBlendState = &(this->colorBlendStateCreateInfo);
        this->pipelineCreateInfo.pDynamicState = &(this->dynamicStateCreateInfo);
        this->pipelineCreateInfo.layout = this->pipelineLayout;
        this->pipelineCreateInfo.renderPass = *(this->renderPass->getRenderPass());
        this->pipelineCreateInfo.subpass = 0;
        this->pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        this->pipelineCreateInfo.basePipelineIndex = -1;

        //Create Pipeline
        auto result = vkCreateGraphicsPipelines(this->logicalDevice->getLogicalDevice(),VK_NULL_HANDLE,1,&(this->pipelineCreateInfo),nullptr,&(this->pipeline));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create graphics pipeline",result);
            return false;
        }
        ANTH_LOGI("Graphics pipeline created");
        this->pipelineCreated = true;
        return true;
    }

    bool AnthemGraphicsPipeline::destroyPipeline(){
        ANTH_LOGI("Destroying graphics pipeline");
        vkDestroyPipeline(this->logicalDevice->getLogicalDevice(),this->pipeline,nullptr);
        this->pipelineCreated = false;
        return true;
    }
    const VkPipeline* AnthemGraphicsPipeline::getPipeline() const{
        ANTH_ASSERT(this->pipelineCreated,"Invalid pipeline");
        return &(this->pipeline);
    }
    const VkPipelineLayout* AnthemGraphicsPipeline::getPipelineLayout() const{
        ANTH_ASSERT(this->pipelineCreated,"Invalid pipeline");
        return &(this->pipelineLayout);
    }
}