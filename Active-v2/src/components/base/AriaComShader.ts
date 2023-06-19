import { AriaComponent } from "../../core/AriaComponent"
import { AriaEnv } from "../../core/graphics/AriaEnv"
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps"
import { AriaShaderOps } from "../../core/graphics/AriaShaderOps"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore"
import { IAriaShader } from "../../core/interface/IAriaShader"

export enum AriaComShaderRenderSide{
    ACSRS_FRONT = 'front',
    ACSRS_BACK = 'back'
}

export class AriaComShader extends AriaComponent implements IAriaShader{
    shaderProgram:WebGLProgram|null = null
    enabled:boolean
    freeTexId: number
    warnMaps: Map<string,boolean>
    vertexSource:string
    fragmentSource:string
    renderSide:AriaComShaderRenderSide = AriaComShaderRenderSide.ACSRS_FRONT

    constructor(vertexSource:string,fragmentSource:string,side:string=AriaComShaderRenderSide.ACSRS_FRONT){
        super("AriaCom/Shader")
        
        this.enabled = false
        this.vertexSource=vertexSource
        this.fragmentSource=fragmentSource
        this.warnMaps = new Map<string,boolean>
        this.freeTexId = 0
        if(side!='front' && side!='back'){
            this._logError("invalid side setting")
        }
        this.renderSide = <AriaComShaderRenderSide>side
    }
    public compileShader(renderer:IAriaRendererCore){
        const gl = renderer.getEnv()
        if(this.vertexSource==""||this.fragmentSource==""){
            this.shaderProgram = <WebGLProgram>gl.createProgram()
            this._logError("Failed to create shader")
        }else{
            let px = renderer.initShaderProgram(this.vertexSource,this.fragmentSource)
            if(px!=null){
                this.enabled = true
                this.shaderProgram = <WebGLProgram>px
                this._logInfo("Created shader")
            }else{
                this.shaderProgram = <WebGLProgram>gl.createProgram()
                this._logError("Failed to create shader")
            }
        }

        
    }
    getShaderProgram(): WebGLProgram {
        return this.shaderProgram!
    }
    public getAttribute(renderer:IAriaRendererCore,key:string){
        if(!this.enabled){
            this._logError("Cannot perform actions on invalid shader")
        }
        const w = renderer.getEnv().getAttribLocation(this.shaderProgram!,key)
        if(w==-1&&!this.warnMaps.has(key)){
            this._logWarn("Shader does not provide attribute: "+key)
            this.warnMaps.set(key,true)
        }
        return w
    }
    public getUniform(renderer:IAriaRendererCore,key:string){
        if(!this.enabled){
            this._logError("Cannot perform actions on invalid shader")
        }
        const w = renderer.getEnv().getUniformLocation(this.shaderProgram!,key)
        if(w==null&&!this.warnMaps.has(key)){
            this._logWarn("Shader does not provide uniform: "+key)
            this.warnMaps.set(key,true)
        }
        return w
    }
    public use(renderer:IAriaRendererCore){
        if(!this.enabled){
            this.compileShader(renderer)
        }
        if(!this.enabled){
            this._logError("Cannot perform actions on invalid shader")
        }
        renderer.useShader(this,()=>{
            this.freeTexId = 0
            renderer.getEnv().useProgram(this.shaderProgram)
        })
    }
    public setSide(x:AriaComShaderRenderSide){
        this.renderSide = x
    }
    public getSide(): string {
        return this.renderSide
    }
    public allocateTexture(){
        this.freeTexId++
        return this.freeTexId-1 + 33984
    }
}