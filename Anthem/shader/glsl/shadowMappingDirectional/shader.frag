#version 450

layout (location = 0) in vec4 inPosition;

layout (location = 0) out vec4 outColor;

void main(){
    outColor = vec4(vec3(inPosition.z),1.0);
}