import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/kawase-blur-single/vertex.glsl"
import fragment from "../shaders/postprocess/kawase-blur-single/fragment.glsl"
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

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
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        super.render(renderer,[()=>{
            renderer.defineUniform("uBlurRange",AriaShaderUniformTp.ASU_VEC1, this._blurRange)
        }])
    }
}