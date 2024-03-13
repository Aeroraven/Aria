#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../AnthemDescriptorPoolReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "AnthemImageContainer.h"
#include "../buffer/AnthemGeneralBufferUtilBase.h"
#include "./AnthemImageInfo.h"

namespace Anthem::Core{
    


    struct AnthemImageCreateProps {
        uint8_t* texData = nullptr;
        uint32_t texWidth = 0;
        uint32_t texHeight = 0;
        uint32_t texChannel = 0;

        bool mipmap2d = false;
        bool msaa = false;
        AnthemImageFormat format = AT_IF_SRGB_UINT8;
        AnthemImageUsage usage = AT_IU_TEXTURE;

        uint32_t extraAccessStages = 0;
    };

    class AnthemImage:
    public virtual AnthemGeneralBufferUtilBase,
    public virtual AnthemImageContainer,
    public virtual AnthemDescriptorPoolReqBase{
    private:
        char* rawImageData = nullptr;
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
        AnthemImage(){
            this->image.desiredLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        bool specifyUsage(AnthemImageUsage usage);
        bool prepareImage();
        bool destroyImage();
        bool enableMipMapping();
        bool enableMsaa();
        bool loadImageData(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels);
        bool loadImageData3(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels,uint32_t depth);
        bool setImageSize(uint32_t width, uint32_t height);
        bool setImageSize3(uint32_t width, uint32_t height, uint32_t depth);
        bool setImageFormat(AnthemImageFormat format);

        bool addAccessStage(uint32_t stageFlag);
        
        const VkImageView* getImageView() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getDepth() const;

        bool toGeneralLayout() {
            this->image.desiredLayout = VK_IMAGE_LAYOUT_GENERAL;
            this->createImageTransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            return true;
        }
    };
}