import { AriaComRenderPass } from "../../components/renderpass/AriaComRenderPass";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaPostFxKawaseBlurSinglePass } from "./AriaPostFxKawaseBlurSinglePass";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxKawaseBlur extends AriaComRenderPass{
    private _srcCanvas: IAriaCanavs[] = []
    private _passes:AriaPostFxKawaseBlurSinglePass[] = []
    private _canvases: AriaComCanvas[] = []
    private _adjustLock = 0

    constructor(){
        super()
        this._rename("AriaCom/Preset/KawaseBlur")

    }
    public addInput(m: IAriaCanavs, w: string = "uSourceFrame") {
        super.addInput(m,w)
        if(w=="uSourceFrame"){
            this._srcCanvas = [m]
        }
        return this
    }
    public setBlurRange(v:number){
        this._adjustLock++
        for(let i=0;i<2;i++){
            const canvas = new AriaComCanvas()
            this._canvases.push(canvas)
        }
        for(let i=0;i<v;i++){
            const pass = new AriaPostFxKawaseBlurSinglePass()
            pass.setBlurRange(i)
            this._passes.push(pass)
        }
        this._adjustLock--
    }

    public render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        if(this._adjustLock!=0){
            this._logInfo("kawase blur: reconstructing framebuffer, render postponed")
            return
        }
        for(let i=0;i<this._passes.length;i++){
            if(i==0){
                this._passes[0].addInput(this._srcCanvas[0])
                this._canvases[i%2].compose(renderer,()=>{
                    this._passes[0].render(renderer)
                })
            }else{
                this._passes[i].addInput(this._canvases[(i+1)%2])
                if(i+1==this._passes.length){
                    this._passes[i].render(renderer)
                }else{
                    this._canvases[i%2].compose(renderer,()=>{
                        this._passes[i].render(renderer)
                    })
                }
            }
        }
    }
}