#pragma once
#include "../base/AnthemBaseImports.h"

namespace Anthem::Core{
    class AnthemUniformBuffer{

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
        std::array<bool, sizeof...(UniTp)> uniIsFloat = {std::is_same<UniTp,float>::value ...};
        std::array<bool, sizeof...(UniTp)> uniIsInt = {std::is_same<UniTp,int>::value ...};
        
    };
}