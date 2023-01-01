import { AriaComponent } from "../../core/AriaComponent";
import { IAriaFramebuffer } from "../../core/interface/IAriaFramebuffer";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";
import { AriaComFramebuffer, AriaFramebufferOption } from "../base/AriaComFramebuffer";
import { AriaComTexture } from "../base/AriaComTexture";
import { IAriaCanavs, IAriaCanvasComposeAttributes } from "../base/interface/IAriaCanvas";
import { IAriaTextureAttached } from "../base/interface/IAriaTextureAttached";
import { AriaComCanvasManager } from "./AriaComCanvasManager";

export class AriaComCanvas extends AriaComponent implements IAriaCanavs{
    protected fbo:IAriaFramebuffer|undefined

    constructor(scale:number=1,enableHdr:boolean=true,enableMipMap:boolean=false){
        super("AriaCom/Canvas")
        this.createFBO(scale,enableHdr,enableMipMap)
    }
    protected createFBO(scale:number=1,enableHdr:boolean=true,enableMipMap:boolean=false){
        const fboOpt = AriaFramebufferOption.create().setScaler(scale).setHdr(enableHdr).setMipMap(enableMipMap)
        this.fbo = new AriaComFramebuffer(fboOpt)
    }
    compose(proc: () => any,attrs:IAriaCanvasComposeAttributes={preserve:false}) {
        this.canvasUse()
        if(attrs.preserve==false){
            this.fbo?.onClear()
        }
        proc()
        this.canvasDetach()
    }
    getTex(): AriaComTexture {
        if(!this.fbo){
            this._logError("Empty framebuffer")
            throw Error()
        }
        return this.fbo.getTex()
    }
    canvasUse(): void {
        if(!this.fbo){
            this._logError("Empty framebuffer")
            throw Error()
        }
        this.fbo.bind()
        AriaComCanvasManager.getInstance().setCanvas(this)
    }
    canvasDetach(): void {
        if(!this.fbo){
            this._logError("Empty framebuffer")
            throw Error()
        }
        this.fbo.unbind()
        AriaComCanvasManager.getInstance().detachCanvas()
    }
}