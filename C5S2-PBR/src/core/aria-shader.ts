import { initShaderProgram } from "./aria-base"

export class AriaShader{
    shaderProgram:WebGLProgram
    gl:WebGL2RenderingContext
    enabled:boolean

    constructor(gl:WebGL2RenderingContext,vertexSource:string,fragmentSource:string){
        if(vertexSource==""||fragmentSource==""){
            this.enabled = false
            this.shaderProgram = <WebGLProgram>gl.createProgram()
        }else{
            this.enabled = true
            this.shaderProgram = <WebGLProgram>initShaderProgram(gl,vertexSource,fragmentSource)

        }
        this.gl = gl
    }
    getAttr(key:string){
        if(!this.enabled){
            throw new Error("Shader is invalid")
        }
        return this.gl.getAttribLocation(this.shaderProgram,key)
    }
    getUniform(key:string){
        if(!this.enabled){
            throw new Error("Shader is invalid")
        }
        return this.gl.getUniformLocation(this.shaderProgram,key)
    }
    use(){
        if(!this.enabled){
            throw new Error("Shader is invalid")
        }
        this.gl.useProgram(this.shaderProgram)
    }
}