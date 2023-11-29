#version 300 es
precision mediump float;
uniform mat4 uProj;
uniform mat4 uModelView;

in vec3 aPos;
in vec3 aCol;
out vec3 vCol;
void main() {
    vCol = aCol;
    gl_Position = uProj * uModelView * vec4(aPos, 1.0);
}