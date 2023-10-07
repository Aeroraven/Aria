import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import vertex from "../shaders/postprocess/mipmap-blur-sequential/vertex.glsl"
import fragment from "../shaders/postprocess/mipmap-blur-sequential/fragment.glsl"
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { AriaPostFxSimpleBloomUpsample } from "./AriaPostFxSimpleBloomUpsample";
import { AriaPostFxMipmapDownsample } from "./AriaPostFxMipmapDownsample";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxSimpleBloom extends AriaComRenderPass{
    private _upsampleCanvases:IAriaCanavs[] = []
    private _upsamplePasses:AriaPostFxSimpleBloomUpsample[] = []
    private _upLods:number[] = []

    private _downsampleCanvases:IAriaCanavs[] = []
    private _downsamplePasses:AriaPostFxMipmapDownsample[] = []
    private _downLods:number[] = []

    private _ipCanvas:IAriaCanavs = new AriaComCanvas(1,true,true)

    constructor(){
        super()
        this._rename("AriaCom/Preset/SimpleBloom")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        const material = new AriaComShaderMaterial(shaderSource)
        this.setMaterial(material)
    }
    public addInput(m: IAriaCanavs, w?: string): this {
        this._ipCanvas = m
        return this
    }

    public setIteration(v:number){
        for(let i=0;i<v;i++){
            const downCanvas = new AriaComCanvas(Math.pow(0.5,i+1))
            const downPass = new AriaPostFxMipmapDownsample()
            downPass.addInput(this._downsampleCanvases[this._downsampleCanvases.length-1])
            this._downLods.push(i)
            this._downsampleCanvases.push(downCanvas)
            this._downsamplePasses.push(downPass)
        }
        for(let i=v-1;i>=0;i--){
            const upCanvas = new AriaComCanvas(Math.pow(0.5,i+1))
            const upPass = new AriaPostFxSimpleBloomUpsample()
            upPass.addInput(this._downsampleCanvases[i],"uDownsample")
            if(this._upsampleCanvases.length!=0){
                upPass.addInput(this._upsampleCanvases[this._upsampleCanvases.length-1])
            }
            this._upsampleCanvases.push(upCanvas)
            this._upsamplePasses.push(upPass)
            this._upLods.push(i)
        }
    }
    
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        //Downsample
        this._downsamplePasses[0].addInput(this._ipCanvas)
        for(let i=0;i<this._downsamplePasses.length;i++){
            this._downsampleCanvases[i].compose(renderer,()=>{
                this._downsamplePasses[i].render(renderer)
            })
        }
        //Upsample
        for(let i=0;i<this._upsampleCanvases.length-1;i++){
            this._upsampleCanvases[i].compose(renderer,()=>{
                this._upsamplePasses[i].render(renderer)
            })
        }
        const i = this._upsampleCanvases.length-1
        this._upsamplePasses[i].render(renderer)
        //this._downsamplePasses[5].render()

    }
}