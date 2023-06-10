import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/normal/vertex.glsl"
import fragment from "../shaders/material/normal/fragment.glsl"
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";

export class AriaNormalMaterial extends AriaComShaderMaterial{
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
        this._rename("AriaCom/Preset/NormalMaterial")
    }

    public use(): void {
        super.use()
    }
}