import { AriaComponent } from "../../core/AriaComponent";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";
import { AriaComFramebuffer, AriaFramebufferOption } from "../base/AriaComFramebuffer";
import { AriaComTexture } from "../base/AriaComTexture";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { IAriaTextureAttached } from "../base/interface/IAriaTextureAttached";
import { AriaComCanvasManager } from "./AriaComCanvasManager";

export class AriaComCanvas extends AriaComponent implements IAriaCanavs{
    fbo:IAriaGLBuffer<WebGLFramebuffer> & IAriaTextureAttached
    constructor(scale:number=1,enableHdr:boolean=true,enableMipMap:boolean=false){
        super("AriaCom/Canvas")
        const fboOpt = AriaFramebufferOption.create().setScaler(scale).setHdr(enableHdr).setMipMap(enableMipMap)
        this.fbo = new AriaComFramebuffer(fboOpt)
    }
    compose(proc: () => any) {
        this.canvasUse()
        proc()
        this.canvasDetach()
    }
    getTex(): AriaComTexture {
        return this.fbo.getTex()
    }
    canvasUse(): void {
        this.fbo.bind()
        AriaComCanvasManager.getInstance().setCanvas(this)
    }
    canvasDetach(): void {
        this.fbo.unbind()
        AriaComCanvasManager.getInstance().detachCanvas()
    }
}