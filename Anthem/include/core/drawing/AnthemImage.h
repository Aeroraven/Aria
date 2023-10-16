#pragma once
#include "../base/AnthemBaseImports.h"
#include "./AnthemGeneralBufferBase.h"
#include "./AnthemDescriptorPoolReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"

namespace Anthem::Core{

    struct AnthemImageProp{
        VkImage image;
        VkDeviceMemory memory;
        VkImageCreateInfo imageInfo = {};
    };

    class AnthemImage:
    public AnthemGeneralBufferBase,
    public Util::AnthemUtlCommandBufferReqBase,
    public AnthemDescriptorPoolReqBase{
    private:
        char* rawImageData = nullptr;
        uint32_t width,height,channels;
        AnthemGeneralBufferProp stagingBuffer;
        AnthemImageProp image = {};
        VkImageView imageView;
        VkSampler sampler;
    protected: 
        uint32_t virtual calculateBufferSize() override;
        bool destroyStagingBuffer();
        bool createStagingBuffer();
        bool createImageInternal();
        bool createImageTransitionLayout(VkImageLayout oldLayout,VkImageLayout newLayout);
        bool copyBufferToImage();
        bool createImageView();
        bool createSampler();

        bool createDescriptorPool(uint32_t maxSets);
    public:
        bool prepareImage();
        bool destroyImage();
        bool loadImageData(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels);
    };
}