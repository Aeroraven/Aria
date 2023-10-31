#version 450
precision highp float;
layout(location = 0) in vec2 fragTexCoord;

const int numSamples = 128;
const float sampleRaidus = 1.0f;

layout(set = 0, binding = 0) uniform sampler2D normalTex;
layout(set = 1, binding = 0) uniform sampler2D positionTex;
layout(set = 2, binding = 0) uniform sampler2D noiseTex;
layout(set = 3, binding = 0) uniform UniformAttrs {
    mat4 deferProjMat;
    vec4 windowState;
    vec4 samples[numSamples];
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    
    float wH = ubo.windowState.x;
    float wW = ubo.windowState.y;
    vec2 noiseScaler = vec2(wH,wW)/4.0;

    //Get Attributes
    vec3 normal = normalize(texture(normalTex,fragTexCoord).rgb);
    vec3 position = texture(positionTex,fragTexCoord).rgb;
    vec3 noise = normalize(texture(noiseTex,fragTexCoord*noiseScaler).rgb*2.0-1.0);

    float occls = 0.0;
    float totls = 0.0;

    //Get Local TBNs
    vec3 tangent = noise - normal * dot(noise,normal);
    vec3 bitangent = cross(normal,tangent);
    mat3 tbn = mat3(tangent,bitangent,normal);


    //Get AOs
    vec3 avgPos = vec3(0.0);
    float avgDiff = 0;
    for(int i=0;i<numSamples;i++){
        totls += 1.0;
        vec3 samp = tbn * ubo.samples[i].rgb;
        vec3 sampPos = samp * sampleRaidus + position;
        vec4 sampNdcPos = ubo.deferProjMat * vec4(sampPos,1.0);
        sampNdcPos/=sampNdcPos.w;

        float tX = (sampNdcPos.x+1.0)/2.0;
        float tY = (sampNdcPos.y+1.0)/2.0;
        vec4 sampRefPos = texture(positionTex,vec2(tX,tY));
        sampRefPos/=sampRefPos.w;

        float rangeCheck = smoothstep(0.0,1.0,sampleRaidus/abs(sampRefPos.z-position.z));
        avgPos += texture(normalTex,vec2(tX,tY)).rgb;

        if(sampRefPos.z-position.z>1e-3){
            occls+=1.0*rangeCheck;
        }
        avgDiff += sampRefPos.z-position.z;
    }
    avgPos /= numSamples;
    avgDiff /= numSamples;
    float ao = 1.0 - occls/totls;
    outColor = vec4(vec3(ao),1.0);
}