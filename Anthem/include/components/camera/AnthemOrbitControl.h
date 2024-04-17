#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/math/AnthemLinAlg.h"
#include "../../core/math/AnthemMathAbbrs.h"
#include "../../core/tools/AnthemDirtyFlag.h"

namespace Anthem::Components::Camera {
	using namespace Anthem::Core::Math;
	using namespace Anthem::Core::Math::Abbr;
	using Anthem::Core::Tool::AnthemDirtyFlag;

	class AnthemOrbitControl {
	private:
		AnthemDirtyFlag<AtMatf4> modelMatrix;
		AtMatf4 translationMatrix;
		AtMatf4 inverseTranslationMatrix;
		AnthemQuaternion<float> rotationQuaternion;;

	protected:
		void calculateModelMatrix(AtMatf4& out);
		bool dragStarted = false;
	public:
		AnthemOrbitControl();
		void specifyTranslation(float x, float y, float z);
		std::function<void(int, int, int)> getMouseController(float sensitivity = 0.1f);
		std::function<void(double, double)> getMouseMoveController(float sensitivity = 0.1f);
		bool getModelMatrix(AtMatf4& out);
	};
}