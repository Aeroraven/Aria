#include "../../../include/components/camera/AnthemOrbitControl.h"
namespace Anthem::Components::Camera {
	using namespace Anthem::Core::Math;
	using namespace Anthem::Core::Math::Abbr;
	using Anthem::Core::Tool::AnthemDirtyFlag;

	AnthemOrbitControl::AnthemOrbitControl() {
		std::function<void(AtMatf4&)> modelMatrixUpdater = std::bind(&AnthemOrbitControl::calculateModelMatrix,this,std::placeholders::_1);
		this->modelMatrix.setUpdater(modelMatrixUpdater);
		this->modelMatrix = this->rotationQuaternion.toMatrix();
	}
	void AnthemOrbitControl::calculateModelMatrix(AtMatf4& out) {

	}
	void AnthemOrbitControl::specifyTranslation(float x, float y, float z) {
		this->translationMatrix = AnthemLinAlg::eye<float, 4>();
		this->translationMatrix[0][3] = x;
		this->translationMatrix[1][3] = y;
		this->translationMatrix[2][3] = z;

		this->inverseTranslationMatrix = AnthemLinAlg::eye<float, 4>();
		this->inverseTranslationMatrix[0][3] = -x;
		this->inverseTranslationMatrix[1][3] = -y;
		this->inverseTranslationMatrix[2][3] = -z;
	
	}
	std::function<void(int, int, int)> AnthemOrbitControl::getMouseController(float sensitivity) {
		return [&, sensitivity](int button, int action, int mods) {
			//ANTH_LOGI("Mouse click", button, action, mods);
			this->modelMatrix.markDirty();
			if (button == 0 && action == 1) {
				this->dragStarted = true;
				//ANTH_LOGI("Drag started");
			}
			if (button == 0 && action == 0) {
				this->dragStarted = false;
				//ANTH_LOGI("Drag ended");
			}
		};
	}
	std::function<void(double, double)> AnthemOrbitControl::getMouseMoveController(float sensitivity) {
		return [&, sensitivity](double x, double y) {
			this->modelMatrix.markDirty();
			static double lastX = 0.0;
			static double lastY = 0.0;
			if (this->dragStarted) {
				double xoffset = x - lastX;
				double yoffset = y - lastY;

				xoffset *= sensitivity;
				yoffset *= sensitivity;

				//ANTH_LOGI("Mouse move", xoffset, yoffset);

				auto deltaRotQuat = AnthemQuaternion<float>();
				deltaRotQuat.setFromEuler(-xoffset,0 , -yoffset);
				this->rotationQuaternion = deltaRotQuat * this->rotationQuaternion;
				this->modelMatrix = this->rotationQuaternion.toMatrix();
			}
			lastX = x;
			lastY = y;
		};
	}
	bool AnthemOrbitControl::getModelMatrix(AtMatf4& out) {
		out = this->translationMatrix;
		out = this->modelMatrix.get().multiply(out);
		//out = out.multiply(this->inverseTranslationMatrix);
		return true;
	}
}