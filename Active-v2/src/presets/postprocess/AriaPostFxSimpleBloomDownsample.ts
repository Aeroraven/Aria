import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/mip-bloom-downsample/vertex.glsl"
import fragment from "../shaders/postprocess/mip-bloom-downsample//fragment.glsl"

export class AriaPostFxSimpleBloomDownsample extends AriaComRenderPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/SimpleBloomDownsample")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}