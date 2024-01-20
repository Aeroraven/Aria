#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../AnthemShaderStorageBuffer.h"
#include "../IAnthemVertexBufferAttrLayout.h"

namespace Anthem::Core {
    template<typename... Desc>
    class AnthemShaderStorageBufferImpl;

    template< template <typename Tp, uint32_t MatDim, uint32_t VecSz, uint32_t ArrSz> class... DescTp,
        typename... Tp, uint32_t... MatDim, uint32_t... VecSz, uint32_t... ArrSz>
    class AnthemShaderStorageBufferImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...> :
    public virtual AnthemShaderStorageBuffer, protected virtual AnthemBufferMemAlignerImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>,public IAnthemVertexBufferAttrLayout {
    
    private:
        uint32_t numElements = 0;
        uint32_t totlBufferSize = 0;
        VkVertexInputBindingDescription vertexInputBindingDescription = {};
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescription = {};

    public:
        uint32_t virtual calculateBufferSize() override {
            return this->totlBufferSize;
        }
        bool setTotalElements(uint32_t totalElements) {
            this->numElements = totalElements;
            auto reqSize = this->bmaCalcRequiredSpaceForDynInput(totalElements);
            this->bufferData = new char[reqSize];
            this->bmaBindBuffer(this->bufferData);
            this->totlBufferSize = reqSize;
            ANTH_LOGI("totlBufferSize=", totlBufferSize);
            return true;
        }
        bool setInput(uint32_t idx, std::array<Tp, VecSz>... args) {
            return this->bmaSetDynamicInput(idx, args...);
        }
        ~AnthemShaderStorageBufferImpl() {
            if (this->bufferData != nullptr) {
                delete[] this->bufferData;
            }
        }
        // As Vertex Buffer
        VkFormat getFormatFromTypeInfo(uint32_t attrIdx) {
            if (this->isFloatType.at(attrIdx)) {
                if (this->attrTpSize.at(attrIdx) == sizeof(float)) {
                    if (this->attrDims.at(attrIdx) == 1) {
                        return VK_FORMAT_R32_SFLOAT;
                    }
                    else if (this->attrDims.at(attrIdx) == 2) {
                        return VK_FORMAT_R32G32_SFLOAT;
                    }
                    else if (this->attrDims.at(attrIdx) == 3) {
                        return VK_FORMAT_R32G32B32_SFLOAT;
                    }
                    else if (this->attrDims.at(attrIdx) == 4) {
                        return VK_FORMAT_R32G32B32A32_SFLOAT;
                    }
                }
                else if (this->attrTpSize.at(attrIdx) == sizeof(double)) {
                    if (this->attrDims.at(attrIdx) == 1) {
                        return VK_FORMAT_R64_SFLOAT;
                    }
                    else if (this->attrDims.at(attrIdx) == 2) {
                        return VK_FORMAT_R64G64_SFLOAT;
                    }
                    else if (this->attrDims.at(attrIdx) == 3) {
                        return VK_FORMAT_R64G64B64_SFLOAT;
                    }
                    else if (this->attrDims.at(attrIdx) == 4) {
                        return VK_FORMAT_R64G64B64A64_SFLOAT;
                    }
                }
            }
            else if (this->isIntType.at(attrIdx)) {
                if (this->isUnsignedType.at(attrIdx)) {
                    if (this->attrTpSize.at(attrIdx) == sizeof(uint8_t)) {
                        if (this->attrDims.at(attrIdx) == 1) {
                            return VK_FORMAT_R8_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 2) {
                            return VK_FORMAT_R8G8_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 3) {
                            return VK_FORMAT_R8G8B8_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 4) {
                            return VK_FORMAT_R8G8B8A8_UINT;
                        }
                    }
                    else if (this->attrTpSize.at(attrIdx) == sizeof(uint16_t)) {
                        if (this->attrDims.at(attrIdx) == 1) {
                            return VK_FORMAT_R16_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 2) {
                            return VK_FORMAT_R16G16_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 3) {
                            return VK_FORMAT_R16G16B16_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 4) {
                            return VK_FORMAT_R16G16B16A16_UINT;
                        }

                    }
                    else if (this->attrTpSize.at(attrIdx) == sizeof(uint32_t)) {
                        if (this->attrDims.at(attrIdx) == 1) {
                            return VK_FORMAT_R32_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 2) {
                            return VK_FORMAT_R32G32_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 3) {
                            return VK_FORMAT_R32G32B32_UINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 4) {
                            return VK_FORMAT_R32G32B32A32_UINT;
                        }
                    }
                }
                else {
                    if (this->attrTpSize.at(attrIdx) == sizeof(int8_t)) {
                        if (this->attrDims.at(attrIdx) == 1) {
                            return VK_FORMAT_R8_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 2) {
                            return VK_FORMAT_R8G8_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 3) {
                            return VK_FORMAT_R8G8B8_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 4) {
                            return VK_FORMAT_R8G8B8A8_SINT;
                        }
                    }
                    else if (this->attrTpSize.at(attrIdx) == sizeof(int16_t)) {
                        if (this->attrDims.at(attrIdx) == 1) {
                            return VK_FORMAT_R16_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 2) {
                            return VK_FORMAT_R16G16_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 3) {
                            return VK_FORMAT_R16G16B16_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 4) {
                            return VK_FORMAT_R16G16B16A16_SINT;
                        }
                    }
                    else if (this->attrTpSize.at(attrIdx) == sizeof(int32_t)) {
                        if (this->attrDims.at(attrIdx) == 1) {
                            return VK_FORMAT_R32_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 2) {
                            return VK_FORMAT_R32G32_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 3) {
                            return VK_FORMAT_R32G32B32_SINT;
                        }
                        else if (this->attrDims.at(attrIdx) == 4) {
                            return VK_FORMAT_R32G32B32A32_SINT;
                        }
                    }
                }
            }
            return VK_FORMAT_UNDEFINED;
        }
        bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc, uint32_t bindLoc) override{
            ANTH_LOGI(this->usage == AnthemSSBOUsage::AT_ASBU_VERTEX);
            ANTH_ASSERT(desc, "Description is nullptr");
            desc->binding = bindLoc;
            desc->stride = this->bmaGetIthElementPrePadding(1);
            desc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return true;
        }
        bool virtual getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc, uint32_t bindLoc) override {
            ANTH_LOGI(this->usage == AnthemSSBOUsage::AT_ASBU_VERTEX);
            ANTH_ASSERT(desc, "Description is nullptr");
            uint32_t offset = 0;
            desc->clear();
            for (auto i = 0; i < this->numArgs; i++) {
                VkVertexInputAttributeDescription attrDesc = {};
                attrDesc.binding = bindLoc;
                attrDesc.location = i;
                attrDesc.format = this->getFormatFromTypeInfo(i);
                attrDesc.offset = this->dynamicOffsetReq.at(i);
                desc->push_back(attrDesc);
                offset += this->attrTpSize.at(i) * this->attrDims.at(i);
            }
            return true;
        }

        bool virtual prepareVertexInputInfo(VkPipelineVertexInputStateCreateInfo* info, uint32_t bindLoc) override {
            ANTH_ASSERT(this->usage == AnthemSSBOUsage::AT_ASBU_VERTEX,"");

            if (!this->getInputBindingDescriptionInternal(&vertexInputBindingDescription,bindLoc)) {
                ANTH_LOGE("Failed to get input binding description");
                return false;
            }
            if (!this->getInputAttrDescriptionInternal(&vertexInputAttributeDescription,bindLoc)) {
                ANTH_LOGE("Failed to get input attribute description");
                return false;
            }
            info->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            info->vertexBindingDescriptionCount = 1;
            info->pVertexBindingDescriptions = &vertexInputBindingDescription;
            info->vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescription.size());
            info->pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
            return true;
        }

        bool virtual prepareVertexInputInfo2(VkVertexInputBindingDescription* outBindingDesc, std::vector<VkVertexInputAttributeDescription>* outAttrDesc, uint32_t bindLoc) {
            ANTH_ASSERT(this->usage == AnthemSSBOUsage::AT_ASBU_VERTEX, "");

            if (!this->getInputBindingDescriptionInternal(&vertexInputBindingDescription, bindLoc)) {
                ANTH_LOGE("Failed to get input binding description");
                return false;
            }
            if (!this->getInputAttrDescriptionInternal(&vertexInputAttributeDescription, bindLoc)) {
                ANTH_LOGE("Failed to get input attribute description");
                return false;
            }
            *outBindingDesc = vertexInputBindingDescription;
            *outAttrDesc = vertexInputAttributeDescription;
        }

        bool virtual updateLayoutSpecification(AnthemVertexStageLayoutSpec* spec, uint32_t bindLoc) {
            return true;
        }

    };
}