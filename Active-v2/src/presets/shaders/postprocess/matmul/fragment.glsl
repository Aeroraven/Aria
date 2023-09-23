#version 300 es
precision highp float;
uniform sampler2D uTex1;
out vec4 fragmentColor;
in vec2 vTex;

float random(vec2 st, float seed) {
	return fract(sin(dot(st, vec2(15.63547, 47.84849))) * seed);
}
void main(){    
	vec2 uv = (vTex.st);    
    float cl = texture(uTex1,vTex).r;   
	float pixel = random(uv, 9249.0);     
	fragmentColor = vec4(uv.s,uv.t,1.0,1.0);
}