#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "AnthemGeneralBufferBase.h"
#include "../AnthemDescriptorPoolReqBase.h"

namespace Anthem::Core{

    struct AnthemUniformBufferProp:public AnthemGeneralBufferProp{
        
    };

    class AnthemUniformBuffer:
    public virtual AnthemGeneralBufferBase{

    private:
        std::vector<AnthemUniformBufferProp> uniformBuffers = {};

    protected:
        
        bool virtual getRawBufferData(void** ptr) = 0;

    protected:
        uint32_t calculateBufferSize() override{
            return static_cast<uint32_t>(this->getBufferSize());
        }

    public:
        const std::vector<AnthemUniformBufferProp>* getBuffers(){
            return &(this->uniformBuffers);
        }
        size_t virtual getBufferSize() = 0;
        bool createBuffer(uint32_t numCopies){
            uniformBuffers.clear();
            for(uint32_t i=0;i<numCopies;i++){
                uniformBuffers.push_back({});
                this->createBufferInternal(&(uniformBuffers.at(i)),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkMapMemory(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).bufferMem,0,this->getBufferSize(),0,&(uniformBuffers.at(i).mappedMem));
            }
            return true;
        }
        bool destroyBuffers(){
            for(int i=0;i<uniformBuffers.size();i++){
                vkUnmapMemory(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).bufferMem);
                vkFreeMemory(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).bufferMem,nullptr);
                vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).buffer,nullptr);
            }
            return true;
        }
        bool updateBuffer(uint32_t idx){
            ANTH_LOGV("Updating Uniforms");
            void* data;
            this->getRawBufferData(&data);
            memcpy(uniformBuffers.at(idx).mappedMem,data,this->getBufferSize());
            return true;
        }
    };
    
   
}