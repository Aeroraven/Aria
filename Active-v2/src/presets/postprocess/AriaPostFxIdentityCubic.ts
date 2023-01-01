import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/identity-cubic/vertex.glsl"
import fragment from "../shaders/postprocess/identity-cubic/fragment.glsl"
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";

export class AriaPostFxIdentityCubic extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxIdentityCubic")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
    public addInput(m: IAriaCanavs, w?: string, tp?: AriaShaderUniformTp): this {
        super.addInput(m,"uSourceFrame",AriaShaderUniformTp.ASU_TEXCUBE)
        return this
    }
}