#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

layout (location = 0) in vec3 inNormal[];

layout (set = 0, binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
}ubo;

void main(){
    for(int i=0;i<2;i++){
        vec3 pos = gl_in[i].gl_Position.xyz;
        vec3 normal = inNormal[i];
        gl_Position = ubo.projMat * vec4(pos,1.0);
        EmitVertex();

        gl_Position = ubo.projMat * vec4(pos + normal * 0.5,1.0);
        EmitVertex();

        EndPrimitive();
    }
}