import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../assets/pathtracer/shaders/base/vertex.glsl"
import fragment from "../assets/pathtracer/shaders/base/fragment.glsl"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { AriaComRTFragShaderProcessor } from "./AriaComRTFragShaderProcessor";

export class AriaComRTRenderPass extends AriaComRenderPass{
    private fovx:number
    private fovy:number
    private dist:number
    private aspect:number
    constructor(fragProcessor:AriaComRTFragShaderProcessor,fovx:number,fovy:number,dist:number,aspect:number){
        super()
        this._rename("AriaCom/PathTracing/RenderPass")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragProcessor.generateCodes()
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
        this._allowEmptyInput = true
        this.fovx = fovx
        this.fovy = fovy
        this.dist = dist
        this.aspect = aspect
    }
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        super.render(renderer,[()=>{
            renderer.defineUniform("ufovx",AriaShaderUniformTp.ASU_VEC1, this.fovx)
            renderer.defineUniform("ufovy",AriaShaderUniformTp.ASU_VEC1, this.fovy)
            renderer.defineUniform("udist",AriaShaderUniformTp.ASU_VEC1, this.dist)
            renderer.defineUniform("uaspect",AriaShaderUniformTp.ASU_VEC1, this.aspect)
        }])
    }
}