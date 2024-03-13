#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../AnthemDescriptorPoolReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "AnthemImageContainer.h"
#include "../buffer/AnthemGeneralBufferUtilBase.h"
#include "./AnthemImageInfo.h"

namespace Anthem::Core {

    class AnthemImageCubic :
        public virtual AnthemGeneralBufferUtilBase,
        public virtual AnthemImageContainer,
        public virtual AnthemDescriptorPoolReqBase {

    private:
        uint8_t* rawImageData = nullptr;
        uint32_t width = 0, height = 0, channels = 0, depth = 1;
        AnthemGeneralBufferProp stagingBuffer{};
        bool msaaOn = false;
        AnthemImageUsage definedUsage = AT_IU_UNDEFINED;
        AnthemImageFormat desiredFormat = AT_IF_UNDEFINED;

    protected:
        uint32_t virtual calculateBufferSize() override;
        bool destroyStagingBuffer();
        bool createStagingBuffer();
        bool copyBufferToImage();

    public:
        AnthemImageCubic();
        bool loadImageData(std::array<uint8_t*,6> data,uint32_t width,uint32_t height,uint32_t channels);
        bool setImageSize(uint32_t width, uint32_t height);
        bool setImageFormat(AnthemImageFormat format);
        bool specifyUsage(AnthemImageUsage usage);
        bool prepareImage();
        bool destroyImage();

        bool addAccessStage(uint32_t stageFlag);

        const VkImageView* getImageView() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getDepth() const;

    };
}