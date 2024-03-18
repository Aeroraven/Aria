#pragma once
#include "./AnthemAccelerationStruct.h"
#include "./AnthemAccStructInstance.h"

namespace Anthem::Core {
	class AnthemTopLevelAccStruct :
		public virtual AnthemAccelerationStruct {
	public:
		std::vector<AnthemAccStructInstance*> instances;

	public:
		bool buildTLAS();
		bool addInstance(std::vector<AnthemAccStructInstance*> ls);
	};

}