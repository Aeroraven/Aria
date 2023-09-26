import { AriaComponent } from "../../core/AriaComponent";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaPostFxIdentity } from "../../presets/postprocess/AriaPostFxIdentity";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCanvas } from "../canvas/AriaComCanvas";
import { AriaRendererWrapper } from "../renderer/AriaRendererWrapper";
import { AriaWGL2Renderer } from "../renderer/AriaWGL2Renderer";
import { AriaComRTRenderPass } from "./AriaComRTRenderPass";

export class AriaComRTPingPongTexturePass extends AriaComponent implements IAriaRenderable{
    private iter:number = 0
    private canvas:AriaComCanvas[] = []
    private identity:AriaPostFxIdentity = new AriaPostFxIdentity()
    private renderpass:AriaComRTRenderPass
    constructor(textureA:AriaComCanvas,textureB:AriaComCanvas,renderpass:AriaComRTRenderPass){
        super("AriaCom/RTPingPongTexture")
        this.iter = 0
        this.canvas.push(textureA)
        this.canvas.push(textureB)
        this.renderpass = renderpass
    }
    render(renderer: IAriaRendererCore, preTriggers?: ((_: IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_: IAriaRendererCore) => any)[] | undefined): void {
        this._logWarn("Begin Render")
        this.renderpass.setRenderFrame(this.canvas[this.iter^1])
        this.canvas[this.iter%2].compose(renderer,()=>{
            this.renderpass.render(renderer)
        })
        this.identity.addInput(this.canvas[this.iter%2])
        this.identity.render(renderer)
        this.renderpass.addRenderedFrames()
        this.iter ^= 1
    }

}