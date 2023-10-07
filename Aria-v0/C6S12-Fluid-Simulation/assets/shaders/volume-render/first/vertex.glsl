#version 300 es
precision lowp float;

in vec4 aVert;
in vec3 aNorm;

out vec3 vLocalPos;

uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uModelIT;

void main(){
    gl_Position = uProj * uModel * aVert;
    gl_Position.z = -gl_Position.z;
    vec3 tNorm = (uModel * vec4(aNorm,0.0)).xyz;
    vLocalPos = ((aVert.xyz + 0.5) );
    vLocalPos.z = -vLocalPos.z;
    
}