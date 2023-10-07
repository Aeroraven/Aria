import { AriaWGL2RendererCore } from "../../core/renderer/AriaWGL2RendererCore";
import { AriaWGPURendererCore } from "../../core/renderer/AriaWGPURendererCore";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCamera } from "../camera/AriaComCamera";
import { AriaComCanvas } from "../canvas/AriaComCanvas";
import { AriaComScene } from "../scene/AriaComScene";
import { AriaRendererWrapper } from "./AriaRendererWrapper";

export class AriaWGL2Renderer extends AriaRendererWrapper{
    env:AriaWGPURendererCore
    constructor(canvasId:string,presentationFormat:GPUTextureFormat){
        super()
        this.env = new AriaWGPURendererCore()
    }
    public getEngine(){
        return this.env
    }
}