import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaPostFxGaussianBlurVertical } from "./AriaPostFxGaussianBlurVertical";
import { AriaPostFxGaussianBlurHorizontal } from "./AriaPostFxGaussianBlurHorizontal";
import { AriaPostFxCombine } from "./AriaPostFxCombine";
import { AriaPostFxIdentity } from "./AriaPostFxIdentity";
import { AriaPostFxGaussianBlur } from "./AriaPostFxGaussianBlur";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxGaussianBloom extends AriaComPostPass{
    private _sigma:number[] = []
    private _canvasIdentity = new AriaComCanvas()
    private _passIdentity = new AriaPostFxIdentity()
    private _canvases:AriaComCanvas[] = []
    private _pass0: AriaPostFxGaussianBlur[] = []
    private _pass1: AriaPostFxCombine[] = []

    constructor(){
        super()
        this._rename("AriaCom/Preset/GaussianBloom")
        
    }
    public addInput(m: IAriaCanavs, w: string = "uSourceFrame") {
        super.addInput(m,w)
        if(w=="uSourceFrame"){
            this._passIdentity.addInput(m)
        }
        return this
    }
    public setSigma(v:number[]){
        this._sigma = []
        this._pass0 = []
        this._pass1 = []
        this._canvases = []
        for(let i=0;i<v.length;i++){
            this._sigma.push(v[i])
            const canvas0 = new AriaComCanvas()
            const blurPass = new AriaPostFxGaussianBlur()
            if(i==0){
                blurPass.addInput(this._canvasIdentity)
            }else{
                blurPass.addInput(this._canvases[(i-1)*2])
            }
            blurPass.setSigma(v[i])
            
            const canvas1 = new AriaComCanvas()
            const combinePass = new AriaPostFxCombine()
            combinePass.addInput(canvas0)
            if(i==0){
                combinePass.addInput(this._canvasIdentity)
            }else{
                combinePass.addInput(this._canvases[(i-1)*2])
            }
            this._canvases.push(canvas0)
            this._canvases.push(canvas1)
            this._pass0.push(blurPass)
            this._pass1.push(combinePass)

        }
    }

    public render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        if(this._sigma.length==0){
            this._passIdentity.render(renderer)
        }else{
            this._canvasIdentity.compose(renderer,()=>{
                this._passIdentity.render(renderer)
            })
            for(let i=0;i<this._sigma.length;i++){
                this._canvases[i*2].compose(renderer,()=>{
                    this._pass0[i].render(renderer)
                })
                if(i+1==this._sigma.length){
                    this._pass1[i].render(renderer)
                }else{
                    this._canvases[i*2+1].compose(renderer,()=>{
                        this._pass1[i].render(renderer)
                    })
                }
                
            }
        }
    }
}