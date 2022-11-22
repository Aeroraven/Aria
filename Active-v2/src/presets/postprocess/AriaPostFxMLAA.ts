import { IAriaComShaderSource } from "../../components/base/interface/IAriaComShaderSource";
import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import { AriaComPostPass } from "../../components/postproc/AriaComPostPass";
import vertex from "../shaders/postprocess/mlaa-blending/vertex.glsl"
import fragment from "../shaders/postprocess/mlaa-blending//fragment.glsl"
import { AriaPostFxMLAAThresholding } from "./AriaPostFxMLAAThresholding";
import { AriaPostFxMLAABlending } from "./AriaPostFxMLAABlending";
import { AriaPostFxMLAAMixing } from "./AriaPostFxMLAAMixing";
import { AriaPostFxGrayscale } from "./AriaPostFxGrayscale";
import { AriaComCanvas } from "../../components/canvas/AriaComCanvas";
import { IAriaCanavs } from "../../components/base/interface/IAriaCanvas";

export class AriaPostFxMLAA extends AriaComPostPass{
    pass0: AriaComPostPass
    pass1: AriaComPostPass
    pass2: AriaComPostPass
    pass3: AriaComPostPass

    canvas0: AriaComCanvas
    canvas1: AriaComCanvas
    canvas2: AriaComCanvas

    constructor(){
        super()
        this._rename("AriaCom/Preset/PostFxMLAA")
        const shaderSource:IAriaComShaderSource = {
            vertex:vertex,
            fragment:fragment
        }
        this.pass0 = new AriaPostFxGrayscale()
        this.pass1 = new AriaPostFxMLAAThresholding()
        this.pass2 = new AriaPostFxMLAABlending()
        this.pass3 = new AriaPostFxMLAAMixing()

        this.canvas0 = new AriaComCanvas()
        this.canvas1 = new AriaComCanvas()
        this.canvas2 = new AriaComCanvas()

        this.pass1.addInput(this.canvas0)
        this.pass2.addInput(this.canvas1)
        this.pass3.addInput(this.canvas2, "uBlendWeight")
    }
    public addInputW(m: IAriaCanavs, w: string = "uSourceFrame") {
        this._inputCanvas.push(m)
        this._inputCanvasName.push(w)
        this.pass0.addInput(m)
        this.pass3.addInput(m)
        return this
    }

    public render(preTriggers?: (() => any)[] | undefined, postTriggers?: (() => any)[] | undefined): void {
        this.canvas0.compose(()=>{
            this.pass0.render()
        })
        this.canvas1.compose(()=>{
            this.pass1.render()
        })
        this.canvas2.compose(()=>{
            this.pass2.render()
        })
        this.pass3.render()
    }
}