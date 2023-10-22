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
            return this->getBufferSize();
        }

    public:
        const std::vector<AnthemUniformBufferProp>* getBuffers(){
            return &(this->uniformBuffers);
        }
        size_t virtual getBufferSize() = 0;
        bool createBuffer(uint32_t numCopies){
            uniformBuffers.clear();
            for(int i=0;i<numCopies;i++){
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
    
    template<typename Tp,uint32_t Rk,uint32_t Sz> 
    requires TmplDefs::ATpIsdValidUniform<Tp,Rk,Sz>
    struct AnthemUBDesc;

    template<typename... AUBTpDesc>
    class AnthemUniformBufferImpl;

    template<uint32_t Rk>
    using AnthemUniformMatf = AnthemUBDesc<float,2,Rk>;

    template<uint32_t Rk>
    using AnthemUniformVecf = AnthemUBDesc<float,1,Rk>;

    template<typename... UniTp, uint32_t... UniRk, uint32_t... UniSz>
    class AnthemUniformBufferImpl<AnthemUBDesc<UniTp,UniRk,UniSz>...>:public AnthemUniformBuffer{
    private:
        std::array<uint32_t,sizeof...(UniRk)> uniRanks = {UniRk...};
        std::array<uint32_t,sizeof...(UniSz)> uniVecSize = {UniSz...};
        std::array<uint32_t,sizeof...(UniTp)> uniTpSizeOf = {sizeof(UniTp)...};
        std::array<bool, sizeof...(UniTp)> uniIsFloat = {std::is_same<UniTp,float>::value ...};
        std::array<bool, sizeof...(UniTp)> uniIsInt = {std::is_same<UniTp,int>::value ...};
        
        std::vector<int> alignOffsets = {};
        std::vector<int> variableSize = {};

        char* rawBuffer = nullptr;

    protected:
       
        size_t getRequiredBufferSize(){
            size_t totalSize = 0;
            return alignOffsets.at(alignOffsets.size()-1)+variableSize.at(variableSize.size()-1);
        }
        bool allocateBuffer(){
            rawBuffer = new char[getRequiredBufferSize()];
            return true;
        }
        bool calculatingOffset(){
            int curOffset = 0;
            for(int i=0;i<sizeof...(UniTp);i++){
                int seqLength = uniVecSize.at(i)*((uniVecSize.at(i)-1)*(uniRanks.at(i)==2)+1)*uniTpSizeOf.at(i);
                int reqAlign = uniTpSizeOf.at(i);
                if(uniVecSize.at(i)==1){ //Scalar
                    reqAlign *= 1;
                }else if(uniVecSize.at(i)==2 && uniRanks.at(i)==1){ //Vec2
                    reqAlign *= 2;
                }else if(uniVecSize.at(i)==3 && uniRanks.at(i)==1){ //Vec3
                    reqAlign *= 4;
                }else if(uniVecSize.at(i)==4 && uniRanks.at(i)==1){ //Vec4
                    reqAlign *= 4;
                }else if(uniVecSize.at(i)==4 && uniRanks.at(i)==2){ //Mat4
                    reqAlign *= 4;
                }else{
                    ANTH_LOGE("Unsupported uniform type");
                    return false;
                }
                int alignedOffset = 0;
                if(curOffset%reqAlign==0){
                    alignedOffset = curOffset;
                }else{
                    alignedOffset = (curOffset-curOffset%reqAlign)+reqAlign;
                }
                alignOffsets.push_back(alignedOffset);
                variableSize.push_back(seqLength);
                curOffset = alignedOffset+seqLength;
            }
            return true;
        }
    protected:
        bool getRawBufferData(void** ptr) override{
            *ptr = rawBuffer;
            return true;
        }
        size_t getBufferSize() override{
            return this->getRequiredBufferSize();
        }
    public:
        AnthemUniformBufferImpl(){
            this->calculatingOffset();
            this->allocateBuffer();
        }
        ~AnthemUniformBufferImpl(){
            delete[] rawBuffer;
        }
        bool specifyUniforms(UniTp*... variables){
            std::array<void*,sizeof...(UniTp)> voidPtrs = {variables...};
            for(int i=0;i<sizeof...(UniTp);i++){
                memcpy(rawBuffer+alignOffsets.at(i),voidPtrs.at(i),variableSize.at(i));
            }
            return true;
        }
        

    };
}