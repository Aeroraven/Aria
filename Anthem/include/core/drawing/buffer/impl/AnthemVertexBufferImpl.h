#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../AnthemVertexBuffer.h"

namespace Anthem::Core {
    template<typename Tp, uint32_t Sz>
        requires TmplDefs::ATpIsdValidAttr<Tp, Sz>
    struct AnthemVAOAttrDesc;

    template<uint32_t Sz>
    using AtAttributeVecf = AnthemVAOAttrDesc<float, Sz>;

    template<typename... AVAOTpDesc>
    class AnthemVertexBufferImpl;

    template<typename... AttrTp, uint32_t... AttrSz>
    class AnthemVertexBufferImpl<AnthemVAOAttrDesc<AttrTp, AttrSz>...> :public AnthemVertexBuffer {
        static_assert(sizeof...(AttrSz) == sizeof...(AttrTp), "Invalid data size");
    private:
        uint32_t singleVertexSize = 0;
        std::array<uint32_t, sizeof...(AttrSz)> attrDims = { AttrSz... };
        std::array<uint32_t, sizeof...(AttrTp)> attrTpSize = { sizeof(AttrTp)... };

        std::array<bool, sizeof...(AttrTp)> isFloatType = { std::is_floating_point<AttrTp>::value... };
        std::array<bool, sizeof...(AttrTp)> isIntType = { std::is_integral<AttrTp>::value... };
        std::array<bool, sizeof...(AttrTp)> isUnsignedType = { std::is_unsigned<AttrTp>::value... };

    public:
        AnthemVertexBufferImpl() {
            this->singleVertexSize = 0;

            for (auto i = 0; i < sizeof...(AttrSz); i++) {
                ANTH_LOGI("Param Size=", this->attrDims[i] * this->attrTpSize[i]);
                this->singleVertexSize += this->attrDims[i] * this->attrTpSize[i];
            }
            ANTH_LOGI("Total Size=", this->singleVertexSize);
        }
        bool setTotalVertices(uint32_t vertexNum) {
            ANTH_ASSERT(vertexNum > 0, "Invalid vertex number");
            this->totalVertices = vertexNum;
            this->rawBufferData = new char[this->calculateBufferSize()];
            ANTH_LOGI("Allocated:", this->calculateBufferSize());
            return true;
        }
        bool insertData(int idx, std::array<AttrTp, AttrSz>... data) {
            ANTH_ASSERT(idx < this->totalVertices, "Invalid index");

            //Convert Src Data to Void Pointers
            std::vector<void*> dataPtrs = { data.data()... };

            //Memcpy
            ANTH_TODO("Bug");
            auto offset = idx * this->singleVertexSize;
            auto dataOffset = 0;
            for (auto i = 0; i < sizeof...(AttrSz); i++) {
                auto dataSz = this->attrTpSize.at(i) * this->attrDims.at(i);
                memcpy(this->rawBufferData + offset + dataOffset, dataPtrs[i], dataSz);
                dataOffset += dataSz;
            }
            return true;
        }
        bool getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc) override {
            ANTH_ASSERT(desc, "Description is nullptr");
            desc->binding = 0;
            desc->stride = this->singleVertexSize;
            desc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return true;
        }
        constexpr VkFormat getFormatFromTypeInfo(uint32_t attrIdx) {
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
        bool getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc) override {
            ANTH_ASSERT(desc, "Description is nullptr");
            uint32_t offset = 0;
            desc->clear();
            for (auto i = 0; i < sizeof...(AttrSz); i++) {
                VkVertexInputAttributeDescription attrDesc = {};
                attrDesc.binding = 0;
                attrDesc.location = i;
                attrDesc.format = this->getFormatFromTypeInfo(i);
                attrDesc.offset = offset;
                desc->push_back(attrDesc);
                offset += this->attrTpSize.at(i) * this->attrDims.at(i);
            }
            return true;
        }
        uint32_t calculateBufferSize() override {
            return this->singleVertexSize * this->totalVertices;
        }
        uint32_t getOffsets() override {
            return this->singleVertexSize;
        }
    };
}