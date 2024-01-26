#version 450
layout(binding=0,set=0) uniform Camera{
    mat4 proj;
    mat4 view;
    mat4 local;
    vec4 camPos;
};
layout(location=0) in vec3 position;
layout(location=0) out vec3 outPos; 
layout(location=1) out vec3 outProjPos;
void main(){
    outPos = position;
    gl_Position = proj * view * local * vec4(position,1.0); 
    gl_Position/=gl_Position.w;
    outProjPos = gl_Position.xyz;

    //gl_Position.z = 1.0-gl_Position.z;
}