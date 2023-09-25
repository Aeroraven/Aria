import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/img-show/vertex.glsl"
import fragment from "../shaders/postprocess/img-show/fragment.glsl"

export class AriaPostFxImageDisplay extends AriaComRenderPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/ImageDisplay")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}