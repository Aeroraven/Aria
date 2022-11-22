import { AriaShaderOps, AriaShaderUniformTp } from "../../core/AriaShaderOps";
import { IAriaShader } from "../../core/interface/IAriaShader";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaTexture } from "../../core/interface/IAriaTexture";
import { AriaComShader } from "../base/AriaComShader";
import { IAriaComShaderSource } from "../base/interface/IAriaComShaderSource";
import { AriaComMaterial } from "./AriaComMaterial";

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
    
    public use(): void {
        if(this._shader==null){
            this._logError("Cannot use empty shader")
            return
        }
        this._shader.use()
        this.exportToShader()
    }
}