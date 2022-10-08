#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;

out vec4 fragmentColor;

void main(){
    vec3 ta = texture(uDiffuse,vTex).rgb;
    vec3 tb = texture(uSpecular,vTex).rgb;
    //ta = pow(ta,vec3(2.2));
    //tb = pow(tb,vec3(2.2));
    vec3 it = ta+tb;
    fragmentColor = vec4(ta+tb,1.0);
}