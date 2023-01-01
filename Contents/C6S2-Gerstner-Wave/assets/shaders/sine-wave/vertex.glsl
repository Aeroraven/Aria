#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;
in vec3 aTangent;
in vec3 aBitangent;

out vec2 vTex;
out vec3 vNorm;
out vec3 vFragPos;
out vec4 vWorldPos;
out vec3 vTangent;
out vec3 vBitangent;


uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uModelIT;
uniform float uTimestamp;

void main(){
    vec4 vert = aVert;
    float freq = 2.0;
    float ampl = 0.5;
    float fp = vert.x * freq + uTimestamp*0.005;
    vert.y = ampl*sin(fp);

    gl_Position = uProj * uModel * vert;
    vFragPos =  (uModel*vert).xyz;
    vTex = aTex;
    vNorm = vec3(uModel*vec4(0.0,0.0,1.0,0.0));

    vTangent = vec3((uModel*vec4(1,cos(fp)*freq*ampl,0.0,0.0)).xyz);
    vBitangent = vec3((uModel*(vec4(0.0,0.0,1.0,0.0))).xyz);
    vNorm = cross(vBitangent,vTangent);

    vWorldPos = vert;
}