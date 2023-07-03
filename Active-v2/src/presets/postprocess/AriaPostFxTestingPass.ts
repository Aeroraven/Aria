import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/testing-pass/vertex.glsl"
import fragment from "../shaders/postprocess/testing-pass/fragment.glsl"

export class AriaPostFxTesting extends AriaComPostPass{
    constructor(){
        super()
        this._rename("AriaCom/Preset/TestingPass")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
        this._allowEmptyInput = true
    }
}