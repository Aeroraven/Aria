import { AriaShaderOps } from "../../core/graphics/AriaShaderOps";
import { AriaCallable } from "../../core/base/AriaBaseDefs";
import { IAriaShader } from "../../core/interface/IAriaShader";
import { AriaComShader } from "../base/AriaComShader";
import { IAriaComShaderSource } from "../base/interface/IAriaComShaderSource";
import { AriaComMaterial } from "./AriaComMaterial";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComShaderMaterial extends AriaComMaterial{
    private _shader: IAriaShader|null = null

    constructor(m:IAriaComShaderSource|null = null){
        super("AriaCom/ShaderMaterial")
        if(m==null){
            this._shader=null
        }else{
            this._shader = new AriaComShader(m!.vertex,m!.fragment)
        }
        
    }
    
    public setMaterial(shader:IAriaComShaderSource){
        this._shader = new AriaComShader(shader.vertex,shader.fragment)
        return this
    }
    
    public use(renderer:IAriaRendererCore): void {
        if(this._shader==null){
            this._logError("Cannot use empty shader")
            return
        }
        this._shader.use(renderer)
        this.exportToShader(renderer)
    }

    public withShader(renderer:IAriaRendererCore,callable:AriaCallable){
        if(this._shader==null){
            this._logError("Cannot use empty shader")
            return
        }
        this._shader.use(renderer)
        renderer.useInvariantShader(this._shader,()=>{
            this.exportToShader(renderer)
            callable()
        })
    }
}