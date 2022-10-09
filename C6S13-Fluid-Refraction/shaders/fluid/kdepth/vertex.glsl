#version 300 es
precision highp float;

in vec4 aVert;
in vec3 aNorm;

out vec3 vLocalPos;

uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uModelIT;
uniform mat4 uShadowOrtho;

void main(){
    vec4 aVertModded = vec4(aVert.xyz*0.05,aVert.w);
    mat4 extraRotation = mat4(
        vec4(1.0,0.0,0.0,0.0),
        vec4(0.0,0.0,-1.0,0.0),
        vec4(0.0,1.0,0.0,0.0),
        vec4(0.0,0.0,0.0,1.0)
    );
    aVertModded = extraRotation * aVertModded;
    aVertModded.y -= 5.0;
    aVertModded.z += 5.0;
    aVertModded.x -= 1.0;

    aVertModded.w = 1.0;
    vec4 locp = uProj * uModel * aVertModded;
    vec4 tocp = uModel * aVertModded;
    vLocalPos = tocp.xyz;
    gl_Position = locp;
}