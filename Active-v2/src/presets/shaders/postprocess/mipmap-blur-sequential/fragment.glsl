#version 300 es
precision mediump float;
uniform sampler2D uSourceFrame;
uniform sampler2D uUpsampledLastMip;
uniform float uBlurRange;

out vec4 fragmentColor;
in vec2 vTex;

vec4 kawaseBlur(int lod){
    ivec2 texSize = textureSize(uSourceFrame, lod);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));
    float ofx = texOffset.x * (uBlurRange+0.5);
    float ofy = texOffset.y * (uBlurRange+0.5);
    vec4 retColor = vec4(0.0);
    retColor += textureLod(uSourceFrame,vTex+vec2(ofx,ofy),float(lod));
    retColor += textureLod(uSourceFrame,vTex+vec2(-ofx,ofy),float(lod));
    retColor += textureLod(uSourceFrame,vTex+vec2(ofx,-ofy),float(lod));
    retColor += textureLod(uSourceFrame,vTex+vec2(-ofx,-ofy),float(lod));
    return retColor/4.0;
}

void main(){
    const int iters = 8;
    vec4 color = kawaseBlur(iters);
    for(int i=iters-1;i>=0;i--){
        color = color + kawaseBlur(i);
    }
    fragmentColor = color;
}