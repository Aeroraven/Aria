#version 300 es
precision highp float;
in mediump vec4 vColor;
in mediump vec2 vTex;
in mediump vec3 vNorm;
in mediump vec4 vFragPos;

uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;
uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uShadowOrtho;
uniform mat4 uLightModel[20];

out vec4 fragmentColor;

vec4 lerp (vec4 a, vec4 b, float x){
    return a + x * (b - a);
}

void main(){
    vec4 vPosC = uShadowOrtho * uLightModel[0] * vFragPos;
    vPosC.x*=0.15;
    vPosC.y*=0.15;
    vPosC = (vPosC+1.0)*0.5;

    vec2 shadowTelSize = vec2(1.0 / float(textureSize(uSpecular,0).x), 1.0 / float(textureSize(uSpecular,0).y));
    float shadowW = 0.0;
    float dx = -5.0;
    float dy = -5.0;
    for(int i=-5;i<=5;i++){
        dy=-5.0;
        for(int j=-5;j<=5;j++){
            vec4 shadowColor = texture(uSpecular,vPosC.xy+vec2(dx,dy)*shadowTelSize);
            if(vPosC.z>=shadowColor.x+1e-4){
                shadowW+=1.0;
            }
            dy+=1.0;
        }
        dx+=1.0;
    }
    shadowW = shadowW/121.0;

    vec3 texColor = pow(texture(uDiffuse, vTex).rgb, vec3(2.2));
    vec3 light = (uModel * normalize(vec4(-4.0,-2.0,2.0,0.0))).xyz;
    float ambient = 0.15;
    float diffusePart = 0.85;
    float diffuse = max(dot(vNorm,light),0.0);
    diffuse = (1.0-shadowW)*diffuse;

    //Diffuse Ramp
    vec4 highlightColor = vec4(vec3(0.8),1.0);
    vec4 shadowColor = vec4(vec3(0.2),1.0);
    float rampThresh = 0.25;
    float rampSmooth = 0.1;
    float ramp = smoothstep(rampThresh-rampSmooth*0.5,rampThresh+rampSmooth*0.5,diffuse);
    vec4 rampColor = lerp(shadowColor,highlightColor,ramp);
    diffuse = ramp;

    vec3 mixedColor = texColor * ambient + texColor * rampColor.xyz * diffusePart;
    fragmentColor =  vec4(pow(mixedColor,vec3(1.0/2.2)),1.0); 
    //fragmentColor =  vec4(vec3(gl_FragDepth),1.0); 
}