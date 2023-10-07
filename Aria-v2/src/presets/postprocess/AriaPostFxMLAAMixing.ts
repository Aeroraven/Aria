import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/mlaa-mixing/vertex.glsl"
import fragment from "../shaders/postprocess/mlaa-mixing//fragment.glsl"

export class AriaPostFxMLAAMixing extends AriaComRenderPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxMLAAMixing")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}