#version 450

layout (points) in;
layout (line_strip, max_vertices = 2) out;

layout (location = 0) in vec4 inRandom[];
layout (location = 1) in vec4 inPosition[];

layout (location = 0) out vec4 inRandom2;
layout (location = 1) out vec4 inPosition2;


layout (set = 0, binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 localMat;
    mat4 normalMat;
}ubo;

void main(){
    for(int i=0;i<2;i++){
        vec3 pos = gl_in[i].gl_Position.xyz;
        vec3 normal = inRandom[i].xyz;

        gl_Position = ubo.projMat *  ubo.viewMat*ubo.localMat*vec4(pos,1.0);
        inRandom2 = inRandom[i];
        inPosition2 = inPosition[i];
        EmitVertex();

        gl_Position = ubo.projMat * ubo.viewMat*ubo.localMat* vec4(pos + normal * 0.1,1.0);
        inRandom2 = inRandom[i];
        inPosition2 = inPosition[i];
        EmitVertex();

        EndPrimitive();
    }
}