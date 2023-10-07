import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/mlaa-blending/vertex.glsl"
import fragment from "../shaders/postprocess/mlaa-blending//fragment.glsl"

export class AriaPostFxMLAABlending extends AriaComRenderPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxMLAABlending")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}