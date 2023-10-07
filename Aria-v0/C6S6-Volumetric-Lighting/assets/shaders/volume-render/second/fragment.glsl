#version 300 es
precision lowp float;

in vec3 vLocalPos;
in vec4 vProjPos;

out vec4 fragColor;

uniform sampler2D uSpecular; //Back
uniform sampler2D uVTransfer;
uniform sampler2D uDiffuse; //Opacity

//Begin Ref
//Function sampleAs3DTexture is from "lebarba"
//Source:https://github.com/lebarba/WebGLVolumeRendering/blob/5de6351a17d53645919232804dded6de7a060c61/Web/Index.html
vec4 sampleAs3DTexture(sampler2D cubeTex, vec3 texCoord )
{
    vec4 colorSlice1, colorSlice2;
    vec2 texCoordSlice1, texCoordSlice2;   

    
    float zSliceNumber1 = floor(texCoord.z  * 255.0);    
    float zSliceNumber2 = min( zSliceNumber1 + 1.0, 255.0);     
    
    texCoord.xy /= 16.0;    
    texCoordSlice1 = texCoordSlice2 = texCoord.xy;    
    texCoordSlice1.x += (mod(zSliceNumber1, 16.0 ) / 16.0);
    texCoordSlice1.y += floor((255.0 - zSliceNumber1) / 16.0) / 16.0;
    
    texCoordSlice2.x += (mod(zSliceNumber2, 16.0 ) / 16.0);
    texCoordSlice2.y += floor((255.0 - zSliceNumber2) / 16.0) / 16.0;    
    
    colorSlice1 = texture( cubeTex, vec2(texCoordSlice1.x,1.0-texCoordSlice1.y) );
    colorSlice2 = texture( cubeTex, vec2(texCoordSlice2.x,1.0-texCoordSlice2.y) );    

    vec4 col0 = vec4(0.0, 250.0/255.0, 134.0/255.0,1.0);
    vec4 col1 = vec4(221.0/255.0, 135.0/255.0, 0.0,1.0);

    colorSlice1.rgb = mix(col0,col1,colorSlice1.a).xyz;
    colorSlice2.rgb = mix(col0,col1,colorSlice2.a).xyz;
    
    float zDifference = mod(texCoord.z * 255.0, 1.0);    
    return mix(colorSlice1, colorSlice2, zDifference) ;
}
//End Ref

void main(){
    vec3 pcoord = (vProjPos/vProjPos.w).xyz;
    vec2 ptex = (vec2(pcoord.xy) + 1.0)/2.0;
    
    vec3 backpoint = texture(uSpecular,ptex).xyz;
    vec3 frontpoint = vLocalPos;
    vec3 raydir = backpoint - frontpoint;
    float raylen = length(raydir);
    
    //Ray marching
    const float pstep = 256.0;
    const int maxStep = 400;
    float delta = 1.0/pstep;
    vec3 deltaDir = normalize(raydir) * delta;
    float deltaStepLen = length(deltaDir);

    float acclAlpha = 0.0;
    vec3 acclColor = vec3(0.0);
    float acclLen = 0.0;
    vec3 curPos = frontpoint;

    for(int i=0;i<maxStep;i++){
        vec4 colorCur = sampleAs3DTexture(uDiffuse,curPos);
        float colorAlpha = colorCur.a*0.02;
        
        acclColor += (1.0-acclAlpha)*colorCur.xyz*colorAlpha;
        acclAlpha += colorAlpha;

        curPos += deltaDir;
        acclLen += deltaStepLen;
        if(acclLen>raylen||acclAlpha>1.0){
            break;
        }
    }

    fragColor =  vec4(acclColor,1.0);
}