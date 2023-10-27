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
    bool AnthemGraphicsPipeline::specifyVertexBuffer(AnthemVertexBuffer* vertexBuffer){
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

    bool AnthemGraphicsPipeline::preparePreqPipelineCreateInfo(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->viewport != nullptr,"Viewport not specified");
        ANTH_ASSERT(this->renderPass != nullptr,"Render pass not specified");


        //Specify Dynamic States
        this->dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        this->dynamicStateCreateInfo.pNext = nullptr;
        this->dynamicStateCreateInfo.flags = 0;
        this->dynamicStateCreateInfo.dynamicStateCount = this->reqiredDynamicStates.size();
        this->dynamicStateCreateInfo.pDynamicStates = this->reqiredDynamicStates.data();
        
        //Specify Vertex Shader Input Info
        if(this->vertexBuffer==nullptr){
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
            this->vertexBuffer->prepareVertexInputInfo(&(this->vertexInputStateCreateInfo));
        }

        //Specify Input Assembly Info
        this->inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        this->inputAssemblyStateCreateInfo.pNext = nullptr;
        this->inputAssemblyStateCreateInfo.flags = 0;
        this->inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        this->inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        //Specify Viewport Info
        ANTH_ASSERT(this->viewport != nullptr,"Viewport not specified");
        this->viewport->getViewportStateCreateInfo(&(this->viewportStateCreateInfo));

        //Specify Rasterizer Info
        this->rasterizerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        this->rasterizerStateCreateInfo.pNext = nullptr;
        this->rasterizerStateCreateInfo.flags = 0;
        this->rasterizerStateCreateInfo.depthClampEnable = VK_FALSE;
        this->rasterizerStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        this->rasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        this->rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
        for(int i=0;i<numColorAttachments;i++){
             this->colorBlendAttachmentState[i].blendEnable = VK_FALSE;
             this->colorBlendAttachmentState[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
        

        this->colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        this->colorBlendStateCreateInfo.pNext = nullptr;
        this->colorBlendStateCreateInfo.flags = 0;
        this->colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        this->colorBlendStateCreateInfo.attachmentCount = colorBlendAttachmentState.size();
        this->colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentState.data();

        ANTH_ASSERT( colorBlendAttachmentState.size()>0, "Color blend attachment should not be empty");

        //Specify Depth & Stencil Info
        this->depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        this->depthStencilStateCreateInfo.pNext = nullptr;  
        this->depthStencilStateCreateInfo.flags = 0;
        this->depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        this->depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        this->depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        this->depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        this->depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        this->depthStencilStateCreateInfo.front = {};
        this->depthStencilStateCreateInfo.back = {};
        this->depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        this->depthStencilStateCreateInfo.maxDepthBounds = 1.0f;


        //Setup Flag
        this->prerequisiteInfoSpecified = true;
        return true;
    }
    bool AnthemGraphicsPipeline::createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry){
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        for(const auto& p:entry){
            if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER){
                p.descPool->appendSamplerDescriptorLayoutIdx(&layouts,p.inTypeIndex);
                ANTH_LOGI("Sampler:",p.inTypeIndex);
            }else if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER){
                p.descPool->appendUniformDescriptorLayoutIdx(&layouts,p.inTypeIndex);
                ANTH_LOGI("Uniform:",p.inTypeIndex);
            }else{
                ANTH_LOGE("Invalid layout type");
            }
        }

        for(auto x:layouts){
            ANTH_LOGI("Layouts Are:",(long long)(x));
        }

        this->pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
        this->pipelineLayoutCreateInfo.setLayoutCount = layouts.size();
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
    bool AnthemGraphicsPipeline::createPipelineLayout(){
        //Specify Pipeline Layout Creation Info
        
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        
        ANTH_LOGI("Here");
        this->descriptorPool->getAllDescriptorLayouts(&layouts);
        for(auto x:layouts){
            ANTH_LOGI("Layouts Are:",(long long)(x));
        }
        this->pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
        this->pipelineLayoutCreateInfo.setLayoutCount = layouts.size();
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
        this->pipelineCreateInfo.stageCount = shaderStageCreateInfo.size();
        this->pipelineCreateInfo.pStages = shaderStageCreateInfo.data();
        this->pipelineCreateInfo.pVertexInputState = &(this->vertexInputStateCreateInfo);
        this->pipelineCreateInfo.pInputAssemblyState = &(this->inputAssemblyStateCreateInfo);
        this->pipelineCreateInfo.pTessellationState = nullptr;
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