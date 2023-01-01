import { AriaShaderOps } from "../../core/graphics/AriaShaderOps";
import { AriaCallable } from "../../core/base/AriaBaseDefs";
import { IAriaShader } from "../../core/interface/IAriaShader";
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

    public withShader(callable:AriaCallable){
        if(this._shader==null){
            this._logError("Cannot use empty shader")
            return
        }
        this._shader.use()
        AriaShaderOps.useInvariantShader(this._shader,()=>{
            this.exportToShader()
            callable()
        })
    }
}