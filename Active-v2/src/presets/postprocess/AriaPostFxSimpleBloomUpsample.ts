import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/mip-bloom-upsample/vertex.glsl"
import fragment from "../shaders/postprocess/mip-bloom-upsample//fragment.glsl"

export class AriaPostFxSimpleBloomUpsample extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/SimpleBloomUpsample")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}