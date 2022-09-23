precision highp float;
varying mediump vec4 vColor;
varying mediump vec2 vTex;
varying mediump vec3 vNorm;
varying mediump vec3 vFragPos;

uniform sampler2D uSampler;
uniform sampler2D uSpecSampler;
uniform vec3 uCamPos;
uniform vec3 uCamFront;

void main(){
    vec4 lightCol = vec4(1.0);
    vec4 texCol = vColor * 0.05 + texture2D(uSampler,vec2(vTex.s,vTex.t)) * 0.95;
    vec3 lightPos = vec3(1.5,0.5,2.0);

    //Ambient
    float ambientStrength = 0.1;
    vec4 ambientPart = ambientStrength * lightCol;

    //Diffuse
    float diffuseStrength = 0.4;
    vec3 norm = normalize(vNorm);
    vec3 lightDir = normalize(uCamPos-vFragPos);
    float spotTheta = dot(-lightDir,normalize(uCamFront));
    vec4 diffusePart = vec4(0.0,0.0,0.0,1.0);
    float diffusePower = dot(norm,lightDir);
    float fadeEps = cos(12.5*3.14159/180.0) - cos(17.5*3.14159/180.0);
    float intensity = clamp((spotTheta-cos(17.5*3.14159/180.0))/fadeEps,0.0,1.0);

    diffusePart = lightCol * diffuseStrength * diffusePower * intensity;
    gl_FragColor = (ambientPart + diffusePart)*texCol;
}