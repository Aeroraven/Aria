#pragma once
#include "../../base/AnthemBaseImports.h"
#include "./AnthemBufferMemAligner.h"
#include "./AnthemGeneralBufferBase.h"
#include "./AnthemGeneralBufferProp.h"


namespace Anthem::Core{

    struct AnthemShaderStorageBufferProp{
        AnthemGeneralBufferProp* ssbo;
        AnthemGeneralBufferProp staging;
    };
    
    class AnthemShaderStorageBuffer:public virtual AnthemGeneralBufferBase{
    private:
        AnthemShaderStorageBufferProp bufferProp;
        char* bufferData;
        uint32_t numCopies = 1;
    public:
        uint32_t virtual getBufferSize() = 0;
        bool specifyNumCopies(uint32_t numCopies){
            this->numCopies = numCopies;
            return true;
        }

        bool prepareStagingBuffer(){
            this->createBufferInternal(&this->bufferProp.staging,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            vkMapMemory(this->logicalDevice->getLogicalDevice(), this->bufferProp.staging.bufferMem, 0, getBufferSize(), 0, &this->bufferProp.staging.mappedMem);
            memcpy(this->bufferProp.staging.mappedMem, this->bufferData, (size_t)getBufferSize());
            vkUnmapMemory(this->logicalDevice->getLogicalDevice(), this->bufferProp.staging.bufferMem);
            return true;
        }

    };

    template<typename... Desc>
    class AnthemShaderStorageBufferImpl;

    template< template <typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz> class... DescTp, 
        typename... Tp,uint32_t... MatDim,uint32_t... VecSz,uint32_t... ArrSz>
    class AnthemShaderStorageBufferImpl<DescTp<Tp,MatDim,VecSz,ArrSz>...>:
    public virtual AnthemShaderStorageBuffer,public virtual AnthemBufferMemAligner{
    
    };

}