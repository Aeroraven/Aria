import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/simple-ramp/vertex.glsl"
import fragment from "../shaders/material/simple-ramp/fragment.glsl"
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaSimpleColorRampMaterial extends AriaComShaderMaterial{
    private _r:number = 1.0
    private _g:number = 1.0
    private _b:number = 1.0
    private _a:number = 1.0
    
    private _enableLight:number = 1.0

    constructor(){
        super({
            vertex:vertex,
            fragment:fragment
        })
        this._rename("AriaCom/Preset/SimpleColorRampMaterial")
    }
    public setColor(r:number, g:number, b:number, a:number){
        this._r = r
        this._g = g
        this._b = b
        this._a = a
    }

    public enableLight(b:boolean){
        this._enableLight = b?1:0
    }
    public use(renderer:IAriaRendererCore): void {
        super.use(renderer)
        renderer.defineUniform("uPresetColor",AriaShaderUniformTp.ASU_VEC4,[this._r,this._g,this._b,this._a])
        renderer.defineUniform("uEnableLight",AriaShaderUniformTp.ASU_VEC1I,this._enableLight)
    }
}