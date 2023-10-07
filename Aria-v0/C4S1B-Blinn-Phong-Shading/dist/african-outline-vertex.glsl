attribute vec4 aVert;

uniform mat4 uModel;
uniform mat4 uProj;


void main(){
    vec4 norm = uProj * uModel * aVert;
    vec4 scaled = vec4(norm.x*1.1,norm.y*1.1,norm.z,norm.w);
    gl_Position = scaled;
}