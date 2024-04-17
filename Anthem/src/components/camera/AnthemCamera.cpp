#include "../../../include/components/camera/AnthemCamera.h"

namespace Anthem::Components::Camera{
    AnthemCamera::AnthemCamera(AnthemCameraProjectionType projTp){
        this->projType = projTp;
        std::function<void(AtMatf4&)> projMatrixUpdater = std::bind(&AnthemCamera::calculateProjectionMatrix,this,std::placeholders::_1);
        std::function<void(AtMatf4&)> viewMatrixUpdater = std::bind(&AnthemCamera::calculateViewMatrix,this,std::placeholders::_1);
        this->projectionMatrix.setUpdater(projMatrixUpdater);
        this->viewMatrix.setUpdater(viewMatrixUpdater);
    }
    void AnthemCamera::calculateProjectionMatrix(AtMatf4& out){
        if(this->projType == AT_ACPT_PERSPECTIVE){
            out = AnthemLinAlg::spatialPerspectiveTransformWithFovAspectNegateY(this->frustumNearDepth,this->frustumFarDepth,this->frustumFov,this->frustumAspectRatio);
        }else if(this->projType == AT_ACPT_ORTHO){
            ANTH_LOGI("zN/zF=",this->frustumNearDepth,"/",this->frustumFarDepth,"/",this->frustumAspectRatio,"/",this->csHalfHeight);
            auto w = AnthemLinAlg::spatialOrthoTransform(this->frustumNearDepth,this->frustumFarDepth,
                -this->frustumAspectRatio*this->csHalfHeight,this->frustumAspectRatio*this->csHalfHeight,
                -1.0f*this->csHalfHeight,1.0f*this->csHalfHeight);
            w.print();
            out = w;
        }
    }
    void AnthemCamera::calculateViewMatrix(AtMatf4& out){
        AtVecf3 front = this->pos + this->frontDirection;
        out = AnthemLinAlg::lookAtTransform(this->pos,front,this->up);

    }
    bool AnthemCamera::getProjectionMatrix(AtMatf4& out){
        out = this->projectionMatrix.get();
        return true;
    }
    bool AnthemCamera::getViewMatrix(AtMatf4& out){
        out = this->viewMatrix.get();
        return true;
    }
    bool AnthemCamera::getPosition(AtVecf3& out) {
        out = this->pos;
        return true;
    }
    bool AnthemCamera::specifyOrthoClipSpace(float nearDepth,float farDepth, float aspectRatio,float clipSpaceHalfHeight){
        this->frustumNearDepth = nearDepth;
        this->frustumFarDepth = farDepth;
        this->frustumAspectRatio = aspectRatio;
        this->csHalfHeight = clipSpaceHalfHeight;
        this->projectionMatrix.markDirty();
        return true;
    }
    bool AnthemCamera::specifyFrustum(float fov,float nearDepth,float farDepth,float aspectRatio){
        this->frustumFov = fov;
        this->frustumNearDepth = nearDepth;
        this->frustumFarDepth = farDepth;
        this->frustumAspectRatio = aspectRatio;
        this->projectionMatrix.markDirty();
        return true;
    }
    bool AnthemCamera::specifyPosition(float x,float y,float z){
        this->pos = AtVecf3(x,y,z);
        this->viewMatrix.markDirty();
        return true;
    }
    bool AnthemCamera::specifyFrontEyeRay(float x,float y,float z){
        this->frontDirection = AtVecf3(x,y,z);
        this->viewMatrix.markDirty();
        return true;
    }
    bool AnthemCamera::specifyRotation(float pitch,float yaw){
        this->rotPch = pitch;
        this->rotYaw = yaw;
        return true;
    }
    bool AnthemCamera::specifyUp(float x, float y, float z) {
        this->up = AtVecf3(x, y, z);
        this->viewMatrix.markDirty();
        return true;
    }
    std::function<void(int, int, int, int)> AnthemCamera::getKeyboardController(float scaler) {
        return [&,scaler](int key, int scancode, int action, int mods) {
            auto sideDirection = AnthemLinAlg::cross3<float>(frontDirection, up).normalize_();
            if (key == GLFW_KEY_W) {
                pos = pos + frontDirection * scaler;
                viewMatrix.markDirty();
            }
            else if (key == GLFW_KEY_S) {
                pos = pos - frontDirection * scaler;
                viewMatrix.markDirty();
            }
            else if (key == GLFW_KEY_A) {
                pos = pos + sideDirection * scaler;
                viewMatrix.markDirty();
            }
            else if (key == GLFW_KEY_D) {
                pos = pos - sideDirection * scaler;
                viewMatrix.markDirty();
            }
            else if (key == GLFW_KEY_Q) {
                pos = pos - up * scaler;
                viewMatrix.markDirty();
            }
            else if (key == GLFW_KEY_E) {
                pos = pos + up * scaler;
                viewMatrix.markDirty();
            }
        };
    }
}