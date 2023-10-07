#version 300 es
precision lowp float;

in vec3 vLocalPos;

out vec4 fragColor;

void main(){

    fragColor =  vec4(vLocalPos.x,vLocalPos.y,-vLocalPos.z,1.0);
}