#version 450
precision highp float;
layout(location = 0) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler2D albedoTex;
layout(set = 1, binding = 0) uniform sampler2D normalTex;
layout(set = 2, binding = 0) uniform sampler2D positionTex;
layout(set = 3, binding = 0) uniform UniformAttrs {
    vec4 mvCameraPos;
} ubo;

layout(location = 0) out vec4 outColor;

const float metallicFactor = 0.8;
const float roughnessFactor = 0.3;
const vec3 lightPosition = vec3(0.0,-100.0,-40.0);
const float pi = 3.1415926;


float trowbridgeReitzNdf(float nh,float roughness){
    //Measures microfacets that conicides with halfway vectors (D)
    float a2 = roughness * roughness;
    return a2/(pi*((nh*nh)*(a2-1.0)+1.0));
}

float schlickGeometryFunc(float nv,float roughness){
    //Measures obscured light by microfacet roughness (G)
    float k = (roughness+1.0)*(roughness+1.0)/8.0;
    return nv/(nv*(1.0-k)+k);
}

vec3 fresnelSchlick(float hv,vec3 baseRefl){
    //Measures reflectance ratio (F)
    float a = clamp(1.0-hv,0.0,1.0);
    float a2 = a*a;
    return baseRefl + (1.0-baseRefl) * (a2*a2*a);
}

vec3 cookTorranceBrdf(vec3 albedo,float roughness,float metal,float nh,float nv,float nl,float hv){
    //Clamp
    nv = max(0.0,nv);
    nl = max(0.0,nl);
    nh = max(0.0,nh);
    hv = max(0.0,hv);

    //Specular reflectance
    float d = trowbridgeReitzNdf(nh,roughness);
    float g1 = schlickGeometryFunc(nv,roughness);
    float g2 = schlickGeometryFunc(nl,roughness);
    vec3 f0 = mix(vec3(0.4),albedo,metal);
    vec3 f = fresnelSchlick(hv,f0);
    vec3 ks = f;
    vec3 specular = d*f*g1*g2/(4.0*nl*nv+1e-3);

    //Lambertian reflectance
    vec3 kd = (1.0-ks)*(1.0-metal);
    vec3 diffuse = kd * albedo / pi;
    
    return specular+diffuse;
}


void main() {
    vec3 pos = texture(positionTex,fragTexCoord).rgb;
    vec3 albedo = pow(texture(albedoTex,fragTexCoord).rgb,vec3(2.2));
    vec3 normal = texture(normalTex,fragTexCoord).rgb;

    //Get view vectors
    normal = normalize(normal);
    vec3 eye = ubo.mvCameraPos.xyz;
    vec3 viewRay = normalize(eye-pos);
    vec3 inRay = normalize(pos-lightPosition);
    vec3 halfway = normalize(-inRay+viewRay);
    float nh = dot(normal,halfway);
    float nl = dot(normal,-inRay);
    float nv = dot(normal,viewRay);
    float hv = dot(halfway,viewRay);

    //Light Attn
    float dist = length(pos-lightPosition);
    float attn = 1.0/(dist*dist);
    vec3 radiance = vec3(0.5);

    //BRDF
    vec3 brdf = cookTorranceBrdf(albedo,roughnessFactor,metallicFactor,nh,nv,nl,hv);
    vec3 lightColor = brdf * max(0.0,nl) * radiance;

    //Rectification
    vec3 tone = lightColor / (lightColor + vec3(1.0));
    vec3 final = pow(tone, vec3(1.0 / 2.2));

    outColor = vec4(vec3(final),1.0);
}