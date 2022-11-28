import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaPostFxGaussianBlurVertical } from "./AriaPostFxGaussianBlurVertical";
import { AriaPostFxGaussianBlurHorizontal } from "./AriaPostFxGaussianBlurHorizontal";

export class AriaPostFxGaussianBlur extends AriaComPostPass{
    private pass0: AriaPostFxGaussianBlurVertical
    private pass1: AriaPostFxGaussianBlurHorizontal

    private canvas0: AriaComCanvas
    private _sigma = 0

    constructor(){
        super()
        this._rename("AriaCom/Preset/GaussianBlur")
        
        this.pass0 = new AriaPostFxGaussianBlurVertical()
        this.pass1 = new AriaPostFxGaussianBlurHorizontal()
        this.canvas0 = new AriaComCanvas()
        this.pass1.addInput(this.canvas0)
    }
    public addInput(m: IAriaCanavs, w: string = "uSourceFrame") {
        super.addInput(m,w)
        if(w=="uSourceFrame"){
            this.pass0.addInput(m)
        }
        return this
    }
    public setSigma(v:number){
        this._sigma = v
        this.pass0.setSigma(v)
        this.pass1.setSigma(v)
    }

    public render(preTriggers?: (() => any)[] | undefined, postTriggers?: (() => any)[] | undefined): void {
        this.canvas0.compose(()=>{
            this.pass0.render()
        })
        this.pass1.render()
    }
}