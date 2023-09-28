import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../assets/pathtracer/shaders/base/vertex.glsl"
import fragment from "../assets/pathtracer/shaders/base/fragment.glsl"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { AriaComRTFragShaderProcessor } from "./AriaComRTFragShaderProcessor";
import { AriaComCanvas } from "../canvas/AriaComCanvas";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";

export class AriaComRTRenderPass extends AriaComRenderPass{
    private fovx:number
    private fovy:number
    private dist:number
    private aspect:number
    private renderedFrames:number = 0
    private renderedResult:IAriaCanavs|null = null
    private origin:number[] = [0,0,0]
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
        this.fovx = fovx / 180 * Math.PI
        this.fovy = fovy / 180 * Math.PI
        this.dist = dist
        this.aspect = aspect
        this.origin = this.origin
    }
    setOrigin(x:number[]){
        this.origin = x
    }
    setRenderFrame(x:IAriaCanavs){
        this.renderedResult = x
    }
    addRenderedFrames(){
        this.renderedFrames += 1
    }
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        super.render(renderer,[()=>{
            renderer.defineUniform("ufovx",AriaShaderUniformTp.ASU_VEC1, this.fovx)
            renderer.defineUniform("ufovy",AriaShaderUniformTp.ASU_VEC1, this.fovy)
            renderer.defineUniform("udist",AriaShaderUniformTp.ASU_VEC1, this.dist)
            renderer.defineUniform("uaspect",AriaShaderUniformTp.ASU_VEC1, this.aspect)
            renderer.defineUniform("uTime",AriaShaderUniformTp.ASU_VEC1, Math.random())
            renderer.defineUniform("uorigin",AriaShaderUniformTp.ASU_VEC3,this.origin)
            if(this.renderedResult!=null){
                renderer.defineUniform("uframes",AriaShaderUniformTp.ASU_VEC1, this.renderedFrames)
                renderer.defineUniform("uSrcFrame",AriaShaderUniformTp.ASU_TEX2D, this.renderedResult.getTex())
            }
        }])
    }
}