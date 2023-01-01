#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uSampler;

out vec4 fragmentColor;

void main(){
    vec4 cl = texture(uSampler,vTex);
    vec3 clrgb = cl.rgb;
    clrgb = clrgb / (clrgb + vec3(1.0));
    clrgb = pow(clrgb,vec3(1.0/2.2));
    fragmentColor = vec4(clrgb,cl.a);
    
}