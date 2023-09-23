import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/mipmap-downsample/vertex.glsl"
import fragment from "../shaders/postprocess/mipmap-downsample/fragment.glsl"
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { AriaPostFxIdentity } from "./AriaPostFxIdentity";
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxMipmapDownsample extends AriaComPostPass{
    private _canvas:IAriaCanavs = new AriaComCanvas(1,true,true)
    private _genMipPass = new AriaPostFxIdentity()
    private _lod = 2.0
    constructor(){
        super()
        this._rename("AriaCom/Preset/MipmapDownsample")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
        super.addInput(this._canvas)
    }
    public addInput(m: IAriaCanavs, w?: string): this {
        this._genMipPass.addInput(m,w)
        return this
    }
    public setLod(v:number){
        this._lod = v
    }
    
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        this._canvas.compose(renderer,()=>{
            this._genMipPass.render(renderer)
        })
        super.render(renderer,[
            ()=>{
                renderer.defineUniform("uLOD",AriaShaderUniformTp.ASU_VEC1,this._lod)
            }
        ])
    }
}