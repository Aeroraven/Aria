#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/math/AnthemLinAlg.h"
#include "../../core/math/AnthemMathAbbrs.h"
#include "../../core/tools/AnthemDirtyFlag.h"

namespace Anthem::Components::Camera{
    using namespace Anthem::Core::Math;
    using namespace Anthem::Core::Math::Abbr;
    using Anthem::Core::Tool::AnthemDirtyFlag;

    enum AnthemCameraProjectionType{
        AT_ACPT_UNDEFINED,
        AT_ACPT_PERSPECTIVE,
        AT_ACPT_ORTHO,
        AT_ACPT_ORTHO_NDC
    };

    class AnthemCamera{
    private:
        AtVecf3 pos;
        float frustumFov,frustumNearDepth,frustumFarDepth,frustumAspectRatio;
        float rotPch,rotYaw;
        bool negateYAxis = true;

        AnthemDirtyFlag<AtMatf4> projectionMatrix;
        AnthemDirtyFlag<AtMatf4> viewMatrix;
    
    protected:
        AtMatf4 calculateProjectionMatrix();
        AtMatf4 calculateViewMatrix();

    public:
        AnthemCamera();
        bool specifyFrustum(float fov,float nearDepth,float farDepth,float aspectRatio);
        bool specifyPosition(float x,float y,float z);
        bool specifyRotation(float pitch,float yaw);

        bool getProjectionMatrix(AtMatf4& out);
        bool getViewMatrix(AtMatf4& out);
    };
}