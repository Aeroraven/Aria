#version 300 es
precision highp float;
in vec4 vLT;
in vec4 vLB;
in vec4 vRT;
in vec4 vRB;
out vec4 fDiff;
uniform vec3 uRef;

void main(){
    vec4 edgA = vRT - vLT;
    vec4 edgB = vLB - vRT;
    vec3 n = normalize(cross(edgA.xyz, edgB.xyz));
    float d = abs(dot(n,normalize(uRef.xyz)));
    float ac = acos(d)/3.1415926;
    fDiff = vec4(n*0.5+0.5,1.0);
}