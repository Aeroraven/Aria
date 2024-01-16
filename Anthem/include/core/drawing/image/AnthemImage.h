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
    enum AnthemImageFormat{
        AT_IF_UNDEFINED,
        AT_IF_SRGB_UINT8,
        AT_IF_SRGB_FLOAT32,
        AT_IF_SBGR_UINT8,
        AT_IF_R_UINT8,
    };

    class AnthemImage:
    public AnthemGeneralBufferUtilBase,
    public AnthemImageContainer,
    public AnthemDescriptorPoolReqBase{
    private:
        char* rawImageData = nullptr;
        uint32_t width = 0,height = 0,channels = 0;
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
        bool setImageSize(uint32_t width, uint32_t height);
        bool setImageFormat(AnthemImageFormat format);
        
        const VkImageView* getImageView() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
    };
}