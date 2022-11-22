#version 300 es
precision highp float;
uniform sampler2D uSourceFrame;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    const float thres = 0.05;
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));

    float D = texture(uSourceFrame,vTex).z;
    float DL = texture(uSourceFrame,vTex + vec2(-1.0, 0.0)*texOffset).z;
    float DR = texture(uSourceFrame,vTex + vec2(+1.0, 0.0)*texOffset).z;
    float DT = texture(uSourceFrame,vTex + vec2(0.0, -1.0)*texOffset).z;
    float DB = texture(uSourceFrame,vTex + vec2(0.0, 1.0)*texOffset).z;
    
    vec4 depthDelta = abs(vec4(DL,DT,DR,DB)-D) * vec4(1.0,1.0,1.0,1.0);
    vec4 depthDeltaTh = step(thres, depthDelta);
    if(dot(depthDeltaTh,vec4(1.0)) < 1e-9){
        discard;
    }
    fragmentColor = depthDeltaTh;  
}