import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/smaa-thresholding/vertex.glsl"
import fragment from "../shaders/postprocess/smaa-thresholding//fragment.glsl"

export class AriaPostFxSMAAThresholding extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxSMAAThresholding")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}