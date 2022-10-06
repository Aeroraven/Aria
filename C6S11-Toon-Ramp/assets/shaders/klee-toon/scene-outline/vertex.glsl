#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out mediump vec4 vColor;
out mediump vec2 vTex;
out mediump vec3 vNorm;
out mediump vec4 vFragPos;


uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;

uniform float uCamAspect;


void main(){
    //Vertex
    const float fTranslation = 0.001;
    vNorm = (uModel * vec4(aNorm,0.0)).xyz;
    vec4 extDir = uProj * vec4(vNorm,0.0);
    vec3 extDir3 = normalize(extDir.xyz);

    vec4 tVert = uModel * aVert;
    vec4 pVert = uProj * tVert;
    pVert.xy += pVert.w * extDir3.xy * fTranslation * vec2(uCamAspect,1.0); 

    gl_Position = pVert;

    //Varyings
    vTex = aTex;
    vFragPos = aVert;
}