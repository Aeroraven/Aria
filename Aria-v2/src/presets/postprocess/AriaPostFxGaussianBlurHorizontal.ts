import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/gaussian-blur-horizontal/vertex.glsl"
import fragment from "../shaders/postprocess/gaussian-blur-horizontal/fragment.glsl"
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxGaussianBlurHorizontal extends AriaComRenderPass{
    private _sigma = 5
    private _kernel :number[] = []
    private _kernelSum = 0
    private _kernelWid = 0
    constructor(){
        super()
        this._rename("AriaCom/Preset/GaussianBlurHorizontal")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
    setSigma(v:number){
        this._sigma = v
        this._kernel = []
        this._kernelSum = 0
        for(let i=-Math.floor(3.0*this._sigma);i<=Math.floor(3*this._sigma);i++){
            const val = Math.exp(-i*i/(2.0*this._sigma*this._sigma))
            this._kernel.push(val)
            this._kernelSum += val
        }
        this._kernelWid = Math.floor(3*this._sigma)
    }
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        super.render(renderer,[()=>{
            renderer.defineUniform("uSigma",AriaShaderUniformTp.ASU_VEC1, this._sigma)
            for(let i=0;i<2*(this._kernelWid)+1;i++){
                renderer.defineUniformExtend("kernel",AriaShaderUniformTp.ASU_VEC1,this._kernel[i],i);
            }
            renderer.defineUniform("sum",AriaShaderUniformTp.ASU_VEC1,this._kernelSum)
        }])
    }
}