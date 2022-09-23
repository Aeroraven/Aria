#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uSampler;

out vec4 fragmentColor;

void main(){
    vec3 t = texture(uSampler,vTex).xyz;
    float intensity = dot(t,vec3(0.2126, 0.7152, 0.0722));
    if(intensity>1.0){
        fragmentColor = vec4(t,1.0);
    }else{
        fragmentColor = vec4(vec3(0.0),1.0);
    }

}