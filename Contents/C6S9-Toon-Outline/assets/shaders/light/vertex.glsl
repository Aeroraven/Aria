attribute vec4 aVert;

uniform mat4 uModel;
uniform mat4 uProj;

void main(){
    gl_Position = uProj * uModel * aVert;
}