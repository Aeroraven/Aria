#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core {

    enum AnthemImageUsage {
        AT_IU_UNDEFINED = 0,
        AT_IU_TEXTURE = 1,
        AT_IU_COLOR_ATTACHMENT = 2,
        AT_IU_COMPUTE_OUTPUT = 3,
    };
    enum AnthemImageFormat {
        AT_IF_UNDEFINED,
        AT_IF_SRGB_UINT8,
        AT_IF_SRGB_FLOAT32,
        AT_IF_SBGR_UINT8,
        AT_IF_R_UINT8,
    };
    class AnthemImageInfoProcessing {
    public:
        static VkFormat getPendingFormat(AnthemImageFormat formatx) {
            VkFormat pendingFormat = VK_FORMAT_R8G8B8A8_SRGB;
            if (formatx == AT_IF_SRGB_UINT8) {
                pendingFormat = VK_FORMAT_R8G8B8A8_SRGB;
            }
            else if (formatx == AT_IF_SRGB_FLOAT32) {
                pendingFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            else if (formatx == AT_IF_SBGR_UINT8) {
                pendingFormat = VK_FORMAT_B8G8R8A8_SRGB;
            }
            else if (formatx == AT_IF_R_UINT8) {
                pendingFormat = VK_FORMAT_R8_SRGB;
            }
            else {
                ANTH_LOGE("Unknown pending format");
            }
            return pendingFormat;
        }
    };

}