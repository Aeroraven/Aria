#version 300 es
precision highp float;
in vec4 vLT;
in vec4 vLB;
in vec4 vRT;
in vec4 vRB;
in vec4 vPos;
out vec4 fDiff;
uniform vec3 uRef;

uniform float useDot;

void main(){
    vec4 edgA = vRT - vLT;
    vec4 edgB = vLB - vRT;
    vec3 n = normalize(cross(edgA.xyz, edgB.xyz));
    float d = abs(dot(n,normalize(uRef.xyz)));
    float ac = acos(d)/3.1415926*2.0;
    if(useDot >0.9){
        fDiff = vec4(abs(n),1.0);
    }else if(useDot >0.5){
        fDiff = vec4(ac,1.0-ac,0.0,1.0);
    }
    else if(useDot >0.2){
        fDiff = vPos*0.5+0.5;
    }else{
        if(dot(n,normalize(uRef.xyz))<0.0){
            fDiff = vec4(n,1.0);
        }else{
            fDiff = vec4(-n,1.0);
        }
    }
    
}