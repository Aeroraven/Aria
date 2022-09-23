#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse; //World Pos (XYZ)
uniform sampler2D uSpecular; //Normal Map
uniform mat4 uModel;
uniform mat4 uProj;
out vec4 fragmentColor;

float randomScalar(vec3 worldPos){
    float d = dot(vec3(414.159, 514.1110, 37.719),cos(worldPos));
    return fract(sin(d)*145414.1934810);
}

vec3 randomVec3(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    float f = dot(vec3(88.9464, 36.4364, 47.564),sin(worldPos));
    float g = dot(vec3(89.6464, 19.4949, 80.386),sin(worldPos));
    return 2.0*(vec3(fract(sin(d)*11451.1919810),fract(sin(f)*3110.35),fract(sin(g)*65472.65472))-0.5);
}

void main(){
    //Constants
    const int SAMPLES = 32;
    const float RADIUS = 1.0;

    //Receiving Coordinates
    vec4 worldPos = texture(uDiffuse,vTex);
    vec4 normalDir = texture(uSpecular,vTex); //Camera Space
    vec4 modelPos = uModel*worldPos;
    vec4 scrPos = uProj*worldPos;

    //Random Tangent Vector
    vec3 randomTangent = normalize(vec3(randomVec3(cos(worldPos.xyz)+worldPos.xyz).xy,0.0));

    //Gram-Schmidt Method -> TBN
    vec3 normalX = normalize(normalDir.xyz*2.0-1.0);
    vec3 tangentX = normalize(randomTangent-normalX*dot(normalX,randomTangent));
    vec3 bitangentX = cross(normalX,tangentX);
    mat3 tbn = mat3(tangentX,bitangentX,normalX);

    //Occlusion Calc
    float occls = 0.0;
    for(int i=0;i<SAMPLES;i++){
        float ix = float(i);
        vec3 disturbance = vec3(ix,ix*115.0,ix*515.0);
        vec3 rndSp = randomVec3(worldPos.xyz+disturbance);
        rndSp.z = abs(rndSp.z);
        vec3 samplePos = normalize(rndSp)*randomScalar(worldPos.xyz+disturbance); //Tangent Space
        vec3 samplePosW = tbn*samplePos; 
        vec4 samplePosM = vec4(samplePosW * RADIUS + modelPos.xyz,1.0); 
        vec4 samplePosP = uProj*samplePosM; //Viewport Space

        vec3 samplePosF = (samplePosP/samplePosP.w).xyz * 0.5 + 0.5;
        vec4 samplePosDestW = uModel*texture(uDiffuse,samplePosF.xy);
        if(samplePosDestW.z>=samplePosM.z){
            float rangeCheck = smoothstep(0.0,1.0,RADIUS/abs(samplePosDestW.z-modelPos.z));
            occls+=rangeCheck;
        }
    }
    occls/=float(SAMPLES);
    fragmentColor = vec4(vec3(1.0-occls),1.0);   
    //fragmentColor = texture(uDiffuse,vTex);
}