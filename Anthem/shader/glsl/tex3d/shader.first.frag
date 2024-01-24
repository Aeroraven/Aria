#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inPos;

void main(){
    outColor = vec4(vec3((inPos+1.0)/2.0), 1.0);
}