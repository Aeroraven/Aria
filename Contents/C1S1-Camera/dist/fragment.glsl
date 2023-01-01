varying mediump vec4 vColor;
varying mediump vec2 vTex;

uniform sampler2D uSampler;

void main(){
    gl_FragColor = vColor * 0.5 + texture2D(uSampler,vec2(vTex.s,vTex.t)) * 0.5;
}