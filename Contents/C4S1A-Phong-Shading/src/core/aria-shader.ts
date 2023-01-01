import { initShaderProgram } from "./aria-base"

export class AriaShader{
    shaderProgram:WebGLProgram
    gl:WebGL2RenderingContext

    constructor(gl:WebGL2RenderingContext,vertexSource:string,fragmentSource:string){
        this.shaderProgram = <WebGLProgram>initShaderProgram(gl,vertexSource,fragmentSource)
        this.gl = gl
    }
    getAttr(key:string){
        return this.gl.getAttribLocation(this.shaderProgram,key)
    }
    getUniform(key:string){
        return this.gl.getUniformLocation(this.shaderProgram,key)
    }
    use(){
        this.gl.useProgram(this.shaderProgram)
    }
}