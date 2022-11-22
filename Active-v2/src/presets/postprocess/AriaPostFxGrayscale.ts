import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/grayscale/vertex.glsl"
import fragment from "../shaders/postprocess/grayscale/fragment.glsl"

export class AriaPostFxGrayscale extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxGrayscale")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}