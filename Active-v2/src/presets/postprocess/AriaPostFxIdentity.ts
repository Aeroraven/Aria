import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/identity/vertex.glsl"
import fragment from "../shaders/postprocess/identity/fragment.glsl"

export class AriaPostFxIdentity extends AriaComRenderPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxIdentity")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}