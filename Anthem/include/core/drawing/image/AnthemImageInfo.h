#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../base/AnthemSwapChain.h"
namespace Anthem::Core {

    enum AnthemImageUsage {
        AT_IU_UNDEFINED = 0,
        AT_IU_TEXTURE = 1,
        AT_IU_COLOR_ATTACHMENT = 2,
        AT_IU_COMPUTE_OUTPUT = 3,
        AT_IU_RAYTRACING_DEST = 4
    };
    enum AnthemImageFormat {
        AT_IF_UNDEFINED,
        AT_IF_SRGB_UINT8,
        AT_IF_SRGB_FLOAT32,
        AT_IF_SBGR_UINT8,
        AT_IF_R_UINT8,
        AT_IF_SWAPCHAIN
    };
    class AnthemImageInfoProcessing {
    public:
        static uint32_t getPerChannelSize(AnthemImageFormat formatx) {
            if (formatx == AT_IF_SRGB_UINT8 || formatx == AT_IF_SBGR_UINT8 ||
                formatx == AT_IF_R_UINT8 || formatx == AT_IF_SWAPCHAIN) {
                return 1;
            }
            else if (formatx == AT_IF_SRGB_FLOAT32) {
                return 4;
            }
            ANTH_LOGE("Unknown format");
            return 0;
        }
        static VkFormat getPendingFormat(AnthemImageFormat formatx,AnthemSwapChain* swapchain = nullptr) {
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
            else if (formatx == AT_IF_SWAPCHAIN) {
                ANTH_ASSERT(swapchain != nullptr, "Empty swapchain ptr");
                pendingFormat = *swapchain->getFormat();
            }
            else {
                ANTH_LOGE("Unknown pending format");
            }
            return pendingFormat;
        }
        static bool setImageLayout(VkImage image, VkCommandBuffer cmdBuf, VkImageLayout oldLayout, VkImageLayout newLayout,
            VkPipelineStageFlags srcFlag, VkPipelineStageFlags dstFlag, uint32_t layers, uint32_t mipmapLvl,
            VkImageAspectFlags aspectFlag) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = aspectFlag;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = mipmapLvl;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = layers;

            switch (oldLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                barrier.srcAccessMask = 0;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            default:
                break;
            }

            switch (newLayout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            default:
                break;
            }
            //
            vkCmdPipelineBarrier(cmdBuf, srcFlag, dstFlag,
                0, 0, nullptr, 0, nullptr, 1, &barrier);
            return true;
        }
    };

}