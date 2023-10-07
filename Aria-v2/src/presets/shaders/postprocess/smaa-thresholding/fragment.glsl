#version 300 es
precision highp float;
uniform sampler2D uSourceFrame;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    const float thres = 0.04;
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));

    float D = texture(uSourceFrame,vTex).z;
    float DL = texture(uSourceFrame,vTex + vec2(-1.0, 0.0)*texOffset).z;
    float DR = texture(uSourceFrame,vTex + vec2(+1.0, 0.0)*texOffset).z;
    float DT = texture(uSourceFrame,vTex + vec2(0.0, -1.0)*texOffset).z;
    float DB = texture(uSourceFrame,vTex + vec2(0.0, 1.0)*texOffset).z;

    float DL2 = texture(uSourceFrame,vTex + vec2(-2.0, 0.0)*texOffset).z;
    float DR2 = texture(uSourceFrame,vTex + vec2(+2.0, 0.0)*texOffset).z;
    float DT2 = texture(uSourceFrame,vTex + vec2(0.0, -2.0)*texOffset).z;
    float DB2 = texture(uSourceFrame,vTex + vec2(0.0, 2.0)*texOffset).z;
    
    vec4 depthDelta = abs(vec4(DL,DT,DR,DB)-D);
    vec4 depthDelta2 = abs(vec4(DL2,DT2,DR2,DB2)-vec4(DL,DT,DR,DB));
    float depthDeltaMax = max(depthDelta.x,max(depthDelta.y,max(depthDelta.z,depthDelta.w)));
    vec4 depthDeltaMax2 = vec4(
        max(depthDeltaMax,depthDelta2.x),
        max(depthDeltaMax,depthDelta2.y),
        max(depthDeltaMax,depthDelta2.z),
        max(depthDeltaMax,depthDelta2.w)
    );

    vec4 depthDeltaTh = step(thres, depthDelta);
    vec4 depthDeltaTh2 = step(0.5*depthDeltaMax2,depthDelta);
    vec4 mixedDepthTh = depthDeltaTh * depthDeltaTh2;

    if(dot(mixedDepthTh,vec4(1.0)) < 1e-9){
        discard;
    }
    fragmentColor = depthDeltaTh;  
}