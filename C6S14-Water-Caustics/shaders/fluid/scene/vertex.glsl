#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out highp vec4 vColor;
out highp vec2 vTex;
out highp vec3 vNorm;
out highp vec4 vFragPos;


uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;


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

    gl_Position = uProj * uModel * aVertModded;
    vNorm = (uModel * vec4(aNorm,0.0)).xyz;
    vTex = aTex;
    vFragPos = aVertModded;
}