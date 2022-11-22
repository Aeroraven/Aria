import { initShaderProgram } from "./aria-base"

export class AriaShader{
    shaderProgram:WebGLProgram
    gl:WebGL2RenderingContext
    enabled:boolean
    freeTexId: number

    constructor(gl:WebGL2RenderingContext,vertexSource:string,fragmentSource:string){
        if(vertexSource==""||fragmentSource==""){
            this.enabled = false
            this.shaderProgram = <WebGLProgram>gl.createProgram()
        }else{
            this.enabled = true
            this.shaderProgram = <WebGLProgram>initShaderProgram(gl,vertexSource,fragmentSource)

        }
        this.gl = gl
        this.freeTexId = 0
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
        this.freeTexId = 0
        this.gl.useProgram(this.shaderProgram)
    }
    getTexId(){
        this.freeTexId++
        return this.freeTexId-1
    }
}