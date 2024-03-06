#pragma once
#include "../AnthemPushConstant.h"

namespace Anthem::Core {
	template<typename... Desc>
	class AnthemPushConstantImpl;

	template< template <typename Tp, uint32_t MatDim, uint32_t VecSz, uint32_t ArrSz> class... DescTp,
		typename... Tp, uint32_t... MatDim, uint32_t... VecSz, uint32_t... ArrSz>
	class AnthemPushConstantImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...> :
	public virtual AnthemPushConstant,
	protected virtual AnthemBufferMemAlignerImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>, {

	};
}
