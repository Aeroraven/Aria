#version 300 es
precision highp float;
uniform sampler2D uBackground;
uniform vec4 uPresetColor;

//Camera
uniform highp mat4 uModel;

//Light Uniforms
const int MAX_LIGHTS = 16;
uniform int uLightCount;
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS];
uniform int uLightType[MAX_LIGHTS];
uniform int uShadowMapType[MAX_LIGHTS];
uniform sampler2D uShadowMap[MAX_LIGHTS];
uniform samplerCube uShadowMapCube[MAX_LIGHTS];

//Varyings
in vec4 vNorm;
in vec4 vModelPos;
in vec4 vLocalPos;

//Out
out vec4 fragmentColor;


void main(){
    vec4 diffuse = vec4(0.0);
    for(int i=0;i<uLightCount;i++){
        
        if(uLightType[i]==0){
            //Directional Light
            diffuse += uLightColor[i] * max(0.0, dot(-uModel*normalize(vec4(uLightPos[i],0.0)), normalize(vNorm.xyzw)));
        }else if(uLightType[i]==1){
            //Point Light
            vec4 absLightPosH = vec4(uLightPos[i],1.0);
            vec3 absLightPos = absLightPosH.xyz / absLightPosH.w;
            vec3 indir = vLocalPos.xyz-absLightPos;

            vec4 modelLightPosH = uModel * vec4(uLightPos[i],1.0);
            vec3 modelLightPos = modelLightPosH.xyz / modelLightPosH.w;
            vec3 indirModel = vModelPos.xyz-modelLightPos;

            float power = max(0.0,dot(normalize(indirModel),normalize(vNorm.xyz)));
            float attn = 1.0/(length(indir)*0.5+1.0); 
            //Check shadow
            float occlusion = 1.0;
            if(uShadowMapType[i]==2){
                vec4 shadowCubeColor = vec4(0.0);
                vec3 nIndir = indir;
                const float shadowPCFOffset = 0.002;
                const int shadowPCFSteps = 1;
                int shadowTotalCounter = 0;
                int shadowOcclCounter = 0;
                vec3 pcfDelta = vec3(0.0);
                for(int pdx = -shadowPCFSteps;pdx<=shadowPCFSteps;pdx+=1){
                    for(int pdy = -shadowPCFSteps;pdy<=shadowPCFSteps;pdy+=1){
                        for(int pdz = -shadowPCFSteps;pdz<=shadowPCFSteps;pdz+=1){
                            pcfDelta = vec3(float(pdx)*(shadowPCFOffset),float(pdy)*(shadowPCFOffset),float(pdz)*(shadowPCFOffset));
                            nIndir = indir+pcfDelta;
                            shadowTotalCounter += 1;
                            //!@asuSelectCubeSamplerInArray(shadowCubeColor,uShadowMapCube,i,8,nIndir)
                            float closeDepth = length(shadowCubeColor.xyz/shadowCubeColor.w);
                            float curDepth = length(indir);
                            if(closeDepth+1e-2<=curDepth){
                                shadowOcclCounter++;
                            }
                        }
                    }
                }
                occlusion = 1.0-float(shadowOcclCounter)/float(shadowTotalCounter);
                
            }
            
            diffuse += uLightColor[i] * power * attn * occlusion;
        }else if(uLightType[i]==2){
            diffuse += uLightColor[i];
        }
        
    }

    fragmentColor = vec4(diffuse * uPresetColor);
}