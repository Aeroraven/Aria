import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/single-color/vertex.glsl"
import fragment from "../shaders/material/single-color/fragment.glsl"
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaSingleColorMaterial extends AriaComShaderMaterial{
    private _r:number = 1.0
    private _g:number = 1.0
    private _b:number = 1.0
    private _a:number = 1.0

    constructor(){
        super({
            vertex:vertex,
            fragment:fragment
        })
        this._rename("AriaCom/Preset/SingleColorMaterial")
    }
    public setColor(r:number, g:number, b:number, a:number){
        this._r = r
        this._g = g
        this._b = b
        this._a = a
    }
    public use(renderer:IAriaRendererCore): void {
        super.use(renderer)
        renderer.defineUniform("uPresetColor",AriaShaderUniformTp.ASU_VEC4,[this._r,this._g,this._b,this._a])
    }
}