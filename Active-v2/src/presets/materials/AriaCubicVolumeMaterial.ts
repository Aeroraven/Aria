import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/cubicvol/vertex.vert"
import fragment from "../shaders/material/cubicvol/fragment.frag"
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaCubicVolumeMaterial extends AriaComShaderMaterial{
    constructor(){
        super({
            vertex:vertex,
            fragment:fragment,
            side:'back'
        })
        this._rename("AriaCom/Preset/CubicVolumeMaterial")
    }

    public use(renderer:IAriaRendererCore): void {
        super.use(renderer)
    }
}