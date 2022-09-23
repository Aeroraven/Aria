#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;
out vec4 fragmentColor;

void main(){
    vec3 colSSR = texture(uDiffuse,vTex).xyz;
    vec3 colDif = texture(uSpecular,vTex).xyz;
    float da = texture(uDiffuse,vTex).a;
    if(da<0.0||da>1.45){
        fragmentColor = vec4(1.0,0.0,0.0,1.0);
    }else{
        vec3 mx = mix(colSSR,colDif,smoothstep(0.0,1.0,da));
        fragmentColor = vec4(mx,1.0);
    }
    
    
}