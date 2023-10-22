#include "../../../include/components/camera/AnthemCamera.h"

namespace Anthem::Components::Camera{
    AnthemCamera::AnthemCamera(){
        //this->projectionMatrix.setUpdater(std::bind(&AnthemCamera::calculateProjectionMatrix,this));
        //this->viewMatrix.setUpdater(std::bind(&AnthemCamera::calculateViewMatrix,this));
    }
    bool AnthemCamera::getProjectionMatrix(AtMatf4& out){
        //out = this->projectionMatrix.get();
        return true;
    }
    bool AnthemCamera::getViewMatrix(AtMatf4& out){
        //out = this->viewMatrix.get();
        return true;
    }
    bool AnthemCamera::specifyFrustum(float fov,float nearDepth,float farDepth,float aspectRatio){
        this->frustumFov = fov;
        this->frustumNearDepth = nearDepth;
        this->frustumFarDepth = farDepth;
        this->frustumAspectRatio = aspectRatio;
        this->projectionMatrix = AtMatf4();
        return true;
    }
    bool AnthemCamera::specifyPosition(float x,float y,float z){
        this->pos = AtVecf3(x,y,z);
        this->viewMatrix = AtMatf4();
        return true;
    }
    bool AnthemCamera::specifyRotation(float pitch,float yaw){
        this->rotPch = pitch;
        this->rotYaw = yaw;
        //this->viewMatrix = AtMatf4();
        return true;
    }
}