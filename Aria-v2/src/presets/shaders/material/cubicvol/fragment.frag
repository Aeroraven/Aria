#version 300 es
precision highp float;
uniform sampler2D uBackground;
uniform vec4 uPresetColor;

//Camera
uniform highp mat4 uModel;
uniform vec3 uCamPos;

//Light Uniforms
const int MAX_LIGHTS = 16;
uniform highp sampler3D uVTexture;

//Varyings
in vec4 vNorm;
in vec4 vModelPos;
in vec4 vLocalPos;

//Out
out vec4 fragmentColor;

vec3 findStartPoint(vec3 camPos,vec3 backPos){
    vec3 dir = backPos - camPos;
    vec3 dinv = 1.0/normalize(dir);
    vec3 t1 = (vec3(1.0)-camPos)*dinv;
    vec3 t2 = (vec3(-1.0)-camPos)*dinv;
    t1 = max(t1,vec3(0.0));
    t2 = max(t2,vec3(0.0));
    float tb = max(max(min(t1.x,t2.x),min(t1.y,t2.y)),min(t1.z,t2.z));
    return tb*vec3(1.0)/dinv+camPos;
}
void main(){
    vec3 startPos = findStartPoint(uCamPos,vLocalPos.xyz);
    vec3 destPos = vLocalPos.xyz;
    vec3 dir = destPos - startPos;
    vec3 ndir = normalize(dir);
    float ldir = length(dir);

    float accColor = 0.0;
    const float STEP_LENGTH = 0.005;
    const int STEP_MAX = int(8.0 / STEP_LENGTH);
    const float STEP_ALPHA = 0.004;
    float r = 0.0;
    int i = 0;
    for(;i<STEP_MAX;r+=STEP_LENGTH,i+=1){
        if(r>ldir){
            break;
        }
        vec3 cPos = startPos + r*ndir;
        float cPosR = max((1.0 - length(cPos)),1e-6);
        vec3 rmcPos = cPos*0.5 + vec3(0.5);
        accColor += texture(uVTexture,rmcPos).r * STEP_ALPHA * pow(cPosR,2.0);
    }
    fragmentColor = vec4(vec3(accColor),1.0);
}