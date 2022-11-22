import { AriaComponent } from "../../core/AriaComponent"
import { AriaEnv } from "../../core/AriaEnv"
import { AriaRenderOps } from "../../core/AriaRenderOps"
import { AriaShaderOps } from "../../core/AriaShaderOps"
import { IAriaShader } from "../../core/interface/IAriaShader"

export class AriaComShader extends AriaComponent implements IAriaShader{
    shaderProgram:WebGLProgram
    gl:WebGL2RenderingContext
    enabled:boolean
    freeTexId: number
    warnMaps: Map<string,boolean>

    constructor(vertexSource:string,fragmentSource:string){
        super("AriaCom/Shader")
        const gl = AriaEnv.env
        this.enabled = false
        if(vertexSource==""||fragmentSource==""){
            this.shaderProgram = <WebGLProgram>gl.createProgram()
            this._logError("Failed to create shader")
        }else{
            let px = AriaRenderOps.initShaderProgram(vertexSource,fragmentSource)
            if(px!=null){
                this.enabled = true
                this.shaderProgram = <WebGLProgram>px
                this._logInfo("Created shader")
            }else{
                this.shaderProgram = <WebGLProgram>gl.createProgram()
                this._logError("Failed to create shader")
            }
        }
        this.warnMaps = new Map<string,boolean>
        this.gl = gl
        this.freeTexId = 0
    }
    public getAttribute(key:string){
        if(!this.enabled){
            this._logError("Cannot perform actions on invalid shader")
        }
        const w = this.gl.getAttribLocation(this.shaderProgram,key)
        if(w==-1&&!this.warnMaps.has(key)){
            this._logWarn("Shader does not provide attribute: "+key)
            this.warnMaps.set(key,true)
        }
        return w
    }
    public getUniform(key:string){
        if(!this.enabled){
            this._logError("Cannot perform actions on invalid shader")
        }
        const w = this.gl.getUniformLocation(this.shaderProgram,key)
        if(w==null&&!this.warnMaps.has(key)){
            this._logWarn("Shader does not provide uniform: "+key)
            this.warnMaps.set(key,true)
        }
        return w
    }
    public use(){
        if(!this.enabled){
            this._logError("Cannot perform actions on invalid shader")
        }
        this.freeTexId = 0
        this.gl.useProgram(this.shaderProgram)
        AriaShaderOps.useShader(this)
    }
    public allocateTexture(){
        this.freeTexId++
        return this.freeTexId-1 + AriaEnv.env.TEXTURE0
    }
}