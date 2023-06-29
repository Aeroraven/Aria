import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/normal/vertex.glsl"
import fragment from "../shaders/material/normal/fragment.glsl"
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaNormalMaterial extends AriaComShaderMaterial{
    constructor(){
        super({
            vertex:vertex,
            fragment:fragment
        })
        this._rename("AriaCom/Preset/NormalMaterial")
    }

    public use(renderer:IAriaRendererCore): void {
        super.use(renderer)
    }
}