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
        AnthemCameraProjectionType projType;
        AtVecf3 pos;
        AtVecf3 up = AtVecf3(0.0f,1.0f,0.0f);
        AtVecf3 frontDirection = AtVecf3(0.0f,0.0f,1.0f);
        float frustumFov,frustumNearDepth,frustumFarDepth,frustumAspectRatio;
        float csHalfHeight;
        float rotPch,rotYaw;
        bool negateYAxis = true;

        AnthemDirtyFlag<AtMatf4> projectionMatrix;
        AnthemDirtyFlag<AtMatf4> viewMatrix;
    
    protected:
        void calculateProjectionMatrix(AtMatf4& out);
        void calculateViewMatrix(AtMatf4& out);

    public:
        AnthemCamera(AnthemCameraProjectionType projTp);
        bool specifyFrustum(float fov,float nearDepth,float farDepth,float aspectRatio);
        bool specifyOrthoClipSpace(float nearDepth,float farDepth, float aspectRatio,float clipSpaceHalfHeight);
        bool specifyPosition(float x,float y,float z);
        bool specifyFrontEyeRay(float x,float y,float z);
        bool specifyUp(float x, float y, float z);
        bool specifyRotation(float pitch,float yaw);

        bool getProjectionMatrix(AtMatf4& out);
        bool getViewMatrix(AtMatf4& out);
    };
}