precision highp float;
varying mediump vec4 vColor;
varying mediump vec2 vTex;
varying mediump vec3 vNorm;
varying mediump vec3 vFragPos;

uniform sampler2D uSampler;
uniform sampler2D uSpecSampler;
uniform vec3 uCamPos;

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
    vec3 lightDir = normalize(lightPos-vFragPos);
    float diffusePower = dot(norm,lightDir);
    vec4 diffusePart = lightCol * diffuseStrength * diffusePower;

    //Specular
    float specStrength = 1.5;
    vec3 viewDir = normalize(uCamPos-vFragPos);
    vec3 reflDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir,reflDir),0.0),16.0);
    vec3 specTex = texture2D(uSpecSampler,vec2(vTex.s,vTex.t)).xyz;
    vec4 specularPart = specStrength * spec * lightCol * vec4(specTex.xyz,1.0);

    //gl_FragColor = vec4(1,0,0,1);
    gl_FragColor = (ambientPart + diffusePart + specularPart)*texCol;
}