#version 450
precision highp float;
layout(location = 0) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler2D normalTex;
layout(set = 1, binding = 0) uniform sampler2D positionTex;
layout(set = 2, binding = 0) uniform sampler2D noiseTex;
layout(set = 3, binding = 0) uniform UniformAttrs {
    mat4 deferProjMat;
    vec4 windowState;
    vec4 samples[64];
} ubo;

layout(location = 0) out vec4 outColor;

const int numSamples = 64;
const float sampleRaidus = 0.1f;

void main() {
    
    float wH = ubo.windowState.x;
    float wW = ubo.windowState.y;
    vec2 noiseScaler = vec2(wH,wW);

    //Get Attributes
    vec3 normal = normalize(texture(normalTex,fragTexCoord).rgb);
    vec3 position = texture(positionTex,fragTexCoord).rgb;
    vec3 noise = normalize(texture(noiseTex,fragTexCoord*noiseScaler).rgb);
    float occls = 0.0;
    float totls = 0.0;

    //Get Local TBNs
    vec3 tangent = noise - normal * dot(noise,normal);
    vec3 bitangent = cross(normal,tangent);
    mat3 tbn = mat3(tangent,bitangent,normal);


    //Get AOs
    for(int i=0;i<numSamples;i++){
        totls += 1.0;
        vec3 samp = tbn * ubo.samples[i].rgb;
        vec3 sampPos = samp * sampleRaidus + position;
        vec4 sampNdcPos = ubo.deferProjMat * vec4(sampPos,1.0);
        sampNdcPos/=sampNdcPos.w;

        float tX = (sampNdcPos.x+1.0)/2.0;
        float tY = (sampNdcPos.y+1.0)/2.0;
        vec4 sampRefPos = texture(positionTex,vec2(tX,tY));

        if(position.z<sampRefPos.z){
            occls+=1.0;
        }
    }
    outColor = vec4(noise,1.0);
}