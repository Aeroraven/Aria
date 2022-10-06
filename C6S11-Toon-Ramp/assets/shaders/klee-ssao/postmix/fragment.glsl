#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;

out vec4 fragmentColor;

void main(){
    float gamma = 2.2;
    vec3 texColor = pow(texture(uDiffuse, vTex).rgb, vec3(gamma));
    float diffuse = texture(uDiffuse,vTex).a;
    float ambient = texture(uSpecular,vTex).r * 0.25;
    vec3 final = vec3((ambient+diffuse)*texColor);
    fragmentColor = vec4(pow(final,vec3(1.0/gamma)),1.0); 
}