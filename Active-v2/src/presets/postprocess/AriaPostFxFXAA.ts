import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/fxaa/vertex.glsl"
import fragment from "../shaders/postprocess/fxaa/fragment.glsl"

export class AriaPostFxFXAA extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxFXAA")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}