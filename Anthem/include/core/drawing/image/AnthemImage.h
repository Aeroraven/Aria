#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../AnthemDescriptorPoolReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "AnthemImageContainer.h"
#include "../buffer/AnthemGeneralBufferUtilBase.h"

namespace Anthem::Core{
    enum AnthemImageUsage{
        AT_IU_UNDEFINED = 0,
        AT_IU_TEXTURE2D = 1,
        AT_IU_COLOR_ATTACHMENT = 2
    };

    class AnthemImage:
    public AnthemGeneralBufferUtilBase,
    public AnthemImageContainer,
    public AnthemDescriptorPoolReqBase{
    private:
        char* rawImageData = nullptr;
        bool samplerCreated = false;
        uint32_t width,height,channels;
        AnthemGeneralBufferProp stagingBuffer;
        VkSampler sampler;
        AnthemImageUsage definedUsage;

    protected: 
        uint32_t virtual calculateBufferSize() override;
        bool destroyStagingBuffer();
        bool createStagingBuffer();
        bool copyBufferToImage();
        bool createSampler();
    
    public:
        bool specifyUsage(AnthemImageUsage usage);
        bool prepareImage();
        bool destroyImage();
        bool enableMipMapping();
        bool loadImageData(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels);
        bool setImageSize(uint32_t width, uint32_t height);
        const VkImageView* getImageView() const;
        const VkSampler* getSampler() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
    };
}