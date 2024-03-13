#pragma once
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

}