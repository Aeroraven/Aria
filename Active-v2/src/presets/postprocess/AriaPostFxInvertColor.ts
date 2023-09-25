import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/invert-color/vertex.glsl"
import fragment from "../shaders/postprocess/invert-color/fragment.glsl"

export class AriaPostFxInvertColor extends AriaComRenderPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxInvertColor")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial()
        material.setMaterial(shaderSource)
        this.setMaterial(material)
    }
}