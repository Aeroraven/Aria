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
    bool AnthemGraphicsPipeline::preparePreqPipelineCreateInfo(){
        
        //Specify Dynamic States
        this->dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        this->dynamicStateCreateInfo.pNext = nullptr;
        this->dynamicStateCreateInfo.flags = 0;
        this->dynamicStateCreateInfo.dynamicStateCount = this->reqiredDynamicStates.size();
        this->dynamicStateCreateInfo.pDynamicStates = this->reqiredDynamicStates.data();
        
        //Specify Vertex Shader Input Info
        this->vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        this->vertexInputStateCreateInfo.pNext = nullptr;
        this->vertexInputStateCreateInfo.flags = 0;
        this->vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
        this->vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
        this->vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        this->vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

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
        this->rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
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
        this->multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        this->multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        this->multisampleStateCreateInfo.minSampleShading = 1.0f;
        this->multisampleStateCreateInfo.pSampleMask = nullptr;
        this->multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        this->multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        //Specify Color Blending Info
        this->colorBlendAttachmentState.blendEnable = VK_FALSE;
        this->colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        this->colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        this->colorBlendStateCreateInfo.pNext = nullptr;
        this->colorBlendStateCreateInfo.flags = 0;
        this->colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        this->colorBlendStateCreateInfo.attachmentCount = 1;
        this->colorBlendStateCreateInfo.pAttachments = &(this->colorBlendAttachmentState);

        //Setup Flag
        this->prerequisiteInfoSpecified = true;
        return true;
    }

    bool AnthemGraphicsPipeline::createPipelineLayout(){
        //Specify Pipeline Layout Creation Info
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        
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
}