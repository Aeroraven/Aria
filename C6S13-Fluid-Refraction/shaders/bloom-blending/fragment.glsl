#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;

out vec4 fragmentColor;

void main(){
    vec3 original = pow(texture(uSpecular,vTex).xyz,vec3(2.2));
    vec3 bloom = pow(texture(uDiffuse,vTex).xyz,vec3(2.2));

    original += bloom;
    vec3 res = vec3(1.0) - exp(-original * 0.5);
    res = pow(res, vec3(1.0 / 2.2));
    fragmentColor = vec4(res, 1.0);
}