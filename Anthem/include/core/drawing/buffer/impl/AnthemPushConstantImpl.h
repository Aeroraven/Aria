#pragma once
#include "../AnthemPushConstant.h"
#include "../AnthemBufferMemAligner.h"

namespace Anthem::Core {
	template<typename... Desc>
	class AnthemPushConstantImpl;

	template< template <typename Tp, uint32_t MatDim, uint32_t VecSz, uint32_t ArrSz> class... DescTp,
		typename... Tp, uint32_t... MatDim, uint32_t... VecSz, uint32_t... ArrSz>
	class AnthemPushConstantImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...> :
	public virtual AnthemPushConstant, 
	protected virtual AnthemBufferMemAlignerImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>{
	
	private:
		char* xbuf = nullptr;

	public:
		AnthemPushConstantImpl() {
			this->xbuf = new char[getSize()];
			this->bmaBindBuffer(this->xbuf);
		}
		virtual uint32_t getSize() override{
			return this->bmaGetUniformAlignOffsetsAll();
		}
		virtual bool setConstant(Tp*... args) {
			return this->bmaSetAllUniformInput2(args...);
		}
		virtual void* getData() override {
			return this->xbuf;
		}
	};
}
