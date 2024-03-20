#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../AnthemShaderModule.h"
#include "../../drawing/AnthemDescriptorPool.h"
#include "../../drawing/buffer/AnthemGeneralBufferUtilBase.h"
#include "../AnthemDescriptorSetEntry.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "./AnthemRayTracingShaders.h"
#include "../AnthemPipelineBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
namespace Anthem::Core {

    struct AnthemRayTracingShaderBindingTable {
        AnthemGeneralBufferProp raygenSbt;
        AnthemGeneralBufferProp missSbt;
        AnthemGeneralBufferProp hitSbt;
        AnthemGeneralBufferProp callableSbt;

        bool raygenCreated;
        bool missCreated;
        bool hitCreated;
        bool callableCreated;
    };

    class AnthemRayTracingPipeline :public virtual AnthemPipelineBase,
        public virtual Util::AnthemUtlPhyDeviceReqBase,
    public virtual AnthemGeneralBufferUtilBase {
    public:
        uint32_t rayRecursion = 1;
        bool pipelineCreated = false;
        bool prerequisiteInfoSpecified = false;

        VkPipeline pipeline = nullptr;
        VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo = {};
        const AnthemRayTracingShaders* shaderModule = nullptr;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo = {};
        uint32_t handleSize = 0;
        uint32_t handleSizeAligned = 0;
        AnthemRayTracingShaderBindingTable bindingTableBuffer;

        bool specifyShaderModule(const AnthemRayTracingShaders* shaderModule);
        bool createBindingTable();
        bool createPipeline();
        const VkPipeline* getPipeline() const;

        bool destroyPipeline();
        bool destroyPipelineLayout();
        bool setRayRecursion(uint32_t rayRec);
        std::vector<VkStridedDeviceAddressRegionKHR> getTraceRayRegions() const;
        const VkPipelineLayout* getPipelineLayout() const;
    };
}