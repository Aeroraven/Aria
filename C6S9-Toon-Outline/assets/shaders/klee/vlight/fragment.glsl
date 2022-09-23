#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse; //World Pos (X,Y,Depth)
uniform sampler2D uSpecular; //Shadow Map
uniform sampler2D uNormal; //Original Map

uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uLightModel[20];
uniform mat4 uShadowOrtho;

out vec4 fragmentColor;

//Begin Ref
//From http://www.alexandre-pestana.com/volumetric-lights/
float ComputeScattering(float lightDotView){
    const float G_SCATTERING = 0.1;
    float result = 1.0 - G_SCATTERING * G_SCATTERING;
    result /= (4.0 * 3.14159 * pow(1.0 + G_SCATTERING * G_SCATTERING - (2.0 * G_SCATTERING) * lightDotView, 1.5));
    return result;
}
//End Ref

void main(){
    const int mSteps = 60;
    const float mStepsf = 60.0;

    vec3 lightDir = (uModel * vec4(1.0,-1.0,0.0,0.0)).xyz;
    vec3 sunColor = pow(vec3(3.3),vec3(1.0));
    lightDir = normalize(lightDir);
    vec3 destPos = texture(uDiffuse,vTex).rgb;
    if(destPos.r==0.0){
        fragmentColor = vec4(0.0);
        return;
    }

    vec3 srcPos = uCamPos;
    vec3 rayDir = destPos - srcPos;
    vec3 rayDirDelta = rayDir / mStepsf;
    vec3 rayDirNorm = normalize(rayDirDelta);
    float rayDirDeltaLen = length(rayDirDelta);

    //Ray Marching
    vec3 curPos = srcPos;
    vec3 accFog = vec3(0.0,0.0,0.0);
    
    for(int i=0;i<mSteps;i++){
        //Progress
        curPos += rayDirDelta;

        vec4 wposFrLightH = (uShadowOrtho*uLightModel[0]*vec4(curPos,1.0)); 
        vec3 wposFrLight = (wposFrLightH/wposFrLightH.w).xyz;
        wposFrLight.x *= 0.15;
        wposFrLight.y *= 0.15;
        wposFrLight = (wposFrLight+1.0)*0.5;
        float shadowMapVal = texture(uSpecular,wposFrLight.xy).r;
        if(wposFrLight.z-5e-4<shadowMapVal){
            accFog +=  ComputeScattering(dot(rayDirNorm, lightDir)) * sunColor;
        }
        
        
    }
    accFog /= mStepsf;
    //accFog = pow(accFog,vec3(1.0/2.2));
    fragmentColor = vec4(vec3(accFog),1.0);
}