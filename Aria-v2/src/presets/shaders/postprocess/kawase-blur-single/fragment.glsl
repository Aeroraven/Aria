#version 300 es
precision mediump float;
uniform sampler2D uSourceFrame;
uniform float uBlurRange;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));
    float ofx = texOffset.x * (uBlurRange+0.5);
    float ofy = texOffset.y * (uBlurRange+0.5);
    vec4 retColor = vec4(0.0);
    retColor += texture(uSourceFrame,vTex+vec2(ofx,ofy));
    retColor += texture(uSourceFrame,vTex+vec2(-ofx,ofy));
    retColor += texture(uSourceFrame,vTex+vec2(ofx,-ofy));
    retColor += texture(uSourceFrame,vTex+vec2(-ofx,-ofy));

    fragmentColor = retColor/4.0;
}