import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/combine/vertex.glsl"
import fragment from "../shaders/postprocess/combine/fragment.glsl"
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxCombine extends AriaComPostPass{
    private _ids = 0
    constructor(){
        super()
        this._rename("AriaCom/Preset/Combine")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
    public addInput(m: IAriaCanavs, w: string = "uSourceFrame") {
        if(w!="uSourceFrame"){
            this._logError("combine: input name cannot be modified")
        }
        super.addInput(m,w+"["+this._ids+"]")
        return this
    }
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        super.render(renderer,[
            ()=>{
                renderer.defineUniform("uSourceCount",AriaShaderUniformTp.ASU_VEC1I,this._ids)
            }
        ])
    }
}