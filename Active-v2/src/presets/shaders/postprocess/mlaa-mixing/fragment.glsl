#version 300 es
precision highp float;
uniform sampler2D uSourceFrame;
uniform sampler2D uBlendWeight;
out vec4 fragmentColor;
in vec2 vTex;
vec4 mixResult(vec2 direction, float coef){
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));
    vec4 srcColor = texture(uSourceFrame, vTex);
    vec4 targetColor = texture(uSourceFrame,vTex+direction*texOffset); 
    return  (targetColor)*coef + (1.0-coef)*srcColor;
}
void main(){
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));
    float wL = texture(uBlendWeight, vTex+vec2(-1.0,0.0)*texOffset).b;
    float wR = texture(uBlendWeight, vTex+vec2(1.0,0.0)*texOffset).r;
    float wT = texture(uBlendWeight, vTex+vec2(0.0,-1.0)*texOffset).a;
    float wB = texture(uBlendWeight, vTex+vec2(0.0,1.0)*texOffset).g;

    vec4 weights = vec4(wL,wT,wR,wB);
    vec4 avgWeights = weights * weights * weights;
    float sum = dot(avgWeights,vec4(1.0));
    if(sum==0.0){
        //discard;
        fragmentColor = texture(uSourceFrame, vTex);
        //fragmentColor = vec4(1.0, 0.0,0.0,1.0);
        return ;
    }
    vec4 blendLeft = mixResult(vec2(-1.0,0.0), wL) * avgWeights.r;
    vec4 blendRight = mixResult(vec2(1.0,0.0),wR) * avgWeights.b;
    vec4 blendTop = mixResult(vec2(0.0,-1.0),wT)* avgWeights.g;
    vec4 blendBottom = mixResult(vec2(0.0,1.0),wB) * avgWeights.a;
    vec4 result = (blendLeft+blendRight+blendTop+blendBottom)/sum;
    fragmentColor = result;
}