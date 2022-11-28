import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/kawase-blur-single/vertex.glsl"
import fragment from "../shaders/postprocess/kawase-blur-single/fragment.glsl"
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/AriaShaderOps";

export class AriaPostFxKawaseBlurSinglePass extends AriaComPostPass{
    private _blurRange = 0

    constructor(){
        super()
        this._rename("AriaCom/Preset/KawaseBlurSinglePass")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
    setBlurRange(v:number){
        this._blurRange = v
    }
    render(preTriggers?: (() => any)[] | undefined, postTriggers?: (() => any)[] | undefined): void {
        super.render([()=>{
            AriaShaderOps.defineUniform("uBlurRange",AriaShaderUniformTp.ASU_VEC1, this._blurRange)
        }])
    }
}