import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/mlaa-blending/vertex.glsl"
import fragment from "../shaders/postprocess/mlaa-blending//fragment.glsl"
import { AriaPostFxSMAAThresholding } from "./AriaPostFxSMAAThresholding";
import { AriaPostFxMLAAMixing } from "./AriaPostFxMLAAMixing";
import { AriaPostFxGrayscale } from "./AriaPostFxGrayscale";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";
import { AriaPostFxSMAABlending } from "./AriaPostFxSMAABlending";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaPostFxSMAABlendResult extends AriaComPostPass{
    pass0: AriaComPostPass
    pass1: AriaComPostPass
    pass2: AriaComPostPass
    pass3: AriaComPostPass

    canvas0: AriaComCanvas
    canvas1: AriaComCanvas
    canvas2: AriaComCanvas

    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxSMAABlendResult")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        this.pass0 = new AriaPostFxGrayscale()
        this.pass1 = new AriaPostFxSMAAThresholding()
        this.pass2 = new AriaPostFxSMAABlending()
        this.pass3 = new AriaPostFxMLAAMixing()

        this.canvas0 = new AriaComCanvas()
        this.canvas1 = new AriaComCanvas()
        this.canvas2 = new AriaComCanvas()

        this.pass1.addInput(this.canvas0)
        this.pass2.addInput(this.canvas1)
        this.pass3.addInput(this.canvas2, "uBlendWeight")
    }
    public addInput(m: IAriaCanavs, w: string = "uSourceFrame") {
        super.addInput(m,w)
        this.pass0.addInput(m)
        this.pass3.addInput(m)
        return this
    }

    public render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        this.canvas0.compose(renderer,()=>{
            this.pass0.render(renderer)
        })
        this.canvas1.compose(renderer,()=>{
            this.pass1.render(renderer)
        })
        this.pass2.render(renderer)
    }
}