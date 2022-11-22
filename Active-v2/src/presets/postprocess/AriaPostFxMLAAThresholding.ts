import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/mlaa-thresholding/vertex.glsl"
import fragment from "../shaders/postprocess/mlaa-thresholding//fragment.glsl"

export class AriaPostFxMLAAThresholding extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxMLAAThresholding")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
}