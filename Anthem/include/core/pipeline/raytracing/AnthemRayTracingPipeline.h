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
        std::vector<AnthemGeneralBufferProp> bindingTableBuffer;

        bool specifyShaderModule(const AnthemRayTracingShaders* shaderModule);
        bool createBindingTable();
        bool createPipeline();
        const VkPipeline* getPipeline() const;

        bool destroyPipeline();
        bool destroyPipelineLayout();
        bool setRayRecursion(uint32_t rayRec);
        std::vector<VkStridedDeviceAddressRegionKHR> getTraceRayRegions(int32_t raygenId,int32_t missId,int32_t closeHitId,int32_t callableId) const;
        const VkPipelineLayout* getPipelineLayout() const;
    };
}