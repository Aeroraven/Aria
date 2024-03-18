#pragma once
#include "./AnthemAccelerationStruct.h"
#include "./AnthemAccStructGeometry.h"

namespace Anthem::Core {
	class AnthemBottomLevelAccStruct :
	public virtual AnthemAccelerationStruct {
	public:
		std::vector<AnthemAccStructGeometry*> geometries;

	public:
		bool buildBLAS();
		bool addGeometry(std::vector<AnthemAccStructGeometry*> ls);
	};

}