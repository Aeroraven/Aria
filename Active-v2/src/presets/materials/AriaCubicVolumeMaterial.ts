import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/cubicvol/vertex.vert"
import fragment from "../shaders/material/cubicvol/fragment.frag"
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { IAriaTexture } from "../../core/interface/IAriaTexture";

export class AriaCubicVolumeMaterial extends AriaComShaderMaterial{
    private _texture : IAriaTexture|null = null
    constructor(){
        super({
            vertex:vertex,
            fragment:fragment,
            side:'back'
        })
        this._rename("AriaCom/Preset/CubicVolumeMaterial")
    }
    public setVTexture(tex:IAriaTexture){
        this._texture = tex
    }
    public use(renderer:IAriaRendererCore): void {
        super.use(renderer)
        if(this._texture == null){
            this._logError("aria_cubic_volume_material: invalid 3d texture")
        }else{
            renderer.defineUniform("uVTexture",AriaShaderUniformTp.ASU_TEX3D,this._texture)
        }
    }
}