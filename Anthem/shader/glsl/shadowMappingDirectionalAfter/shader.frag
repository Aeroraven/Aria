#version 450

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inNormal;

layout (location = 0) out vec4 outColor;

layout (set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
    mat4 lightPovProj;
    mat4 lightPovView;
    vec4 lightDir;
    vec4 shadowMapParams;
}ubo;

layout(set = 1, binding = 0) uniform sampler2D shadowMap;

void main(){
    vec3 light = ubo.lightDir.rgb;
    vec3 normal = inNormal.rgb;
    vec3 position = inPosition.rgb/inPosition.w;
    float diffuse = max(dot(-light,normal),0.0);
    float ambient = 0.1;

    vec2 pcfOffset = 1.0/ubo.shadowMapParams.xy;

    float occl = 0.0;
    float totl = 0.0;

    vec4 shadowMapPos = ubo.lightPovProj * ubo.lightPovView * vec4(position,1.0);
    shadowMapPos /= shadowMapPos.w;
    float sX = (shadowMapPos.x + 1.0)/2.0;
    float sY = (shadowMapPos.y + 1.0)/2.0;

    
    float curDepth = shadowMapPos.z;

    for(int i=-1;i<=1;i++){
        for(int j=-1;j<=1;j++){
            vec2 cOffset = vec2(pcfOffset.x*float(i),pcfOffset.y*float(j));
            float refDepth = texture(shadowMap,vec2(sX,sY)+cOffset).r;
            if(curDepth-refDepth>1e-3){
                occl+=1.0;
            }
            totl+=1.0;
        }
    }
    
    diffuse *= (1.0-occl/totl);

    float color = diffuse + ambient;
    outColor = vec4(vec3(color),1.0);
}