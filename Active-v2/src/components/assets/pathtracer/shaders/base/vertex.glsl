#version 300 es
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uViewOrtho;

uniform float ufovx;
uniform float ufovy;
uniform float udist;
uniform float uaspect;
uniform vec3 uorigin;

in vec4 aPos;
in vec2 aTex;

out vec2 vRayDirection;
out vec3 vOriginPos;
out vec2 vTex;

void main(){
    vec2 dirMapped = 2.0*(aTex-0.5);
    float fovSpanX = tan(ufovx/2.0);
    float fovSpanY = tan(ufovy/2.0)/uaspect;
    dirMapped *= vec2(fovSpanX,fovSpanY);
    vRayDirection = dirMapped;
    vOriginPos = vec3(dirMapped,udist)+vec3(uorigin);
    vTex = aTex;
    gl_Position = uViewOrtho  * aPos;
}