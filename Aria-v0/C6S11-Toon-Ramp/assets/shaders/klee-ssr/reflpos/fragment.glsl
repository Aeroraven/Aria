#version 300 es
precision highp float;

in vec3 vLocalPos;

out vec4 fragColor;

void main(){
    float z = vLocalPos.z * 2.0 - 1.0;
    float near = 0.01;
    float far = 200.0;
    float linearDepth = (2.0 * near * far) / (far + near - z * (far - near));

    fragColor =  vec4(vLocalPos.x,vLocalPos.y,vLocalPos.z,0.0); 
}