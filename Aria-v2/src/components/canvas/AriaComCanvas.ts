import { AriaComponent } from "../../core/AriaComponent";
import { IAriaFramebuffer } from "../../core/interface/IAriaFramebuffer";
import { IAriaGeneralBuffer } from "../../core/interface/IAriaGeneralBuffer";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaComFramebuffer, AriaFramebufferOption } from "../base/AriaComFramebuffer";
import { AriaComTexture } from "../base/AriaComTexture";
import { IAriaCanavs, IAriaCanvasComposeAttributes } from "../base/interface/IAriaCanvas";
import { IAriaTextureAttached } from "../base/interface/IAriaTextureAttached";
import { AriaComCanvasManager } from "./AriaComCanvasManager";

export class AriaComCanvas extends AriaComponent implements IAriaCanavs{
    protected fbo:IAriaFramebuffer|undefined
    protected scale = 1
    protected enableHdr = true
    protected enableMipMap = false
    protected init = false

    constructor(scale:number=1,enableHdr:boolean=true,enableMipMap:boolean=false){
        super("AriaCom/Canvas")
        this.scale = scale
        this.enableHdr = enableHdr
        this.enableMipMap = enableMipMap
        
    }
    protected createFBO(renderer:IAriaRendererCore,scale:number=1,enableHdr:boolean=true,enableMipMap:boolean=false){
        const fboOpt = AriaFramebufferOption.create().setScaler(scale).setHdr(enableHdr).setMipMap(enableMipMap)
        this.fbo = new AriaComFramebuffer(renderer,fboOpt)
    }
    forceInit(renderer:IAriaRendererCore){
        if(this.init==false){
            this.createFBO(renderer,this.scale,this.enableHdr,this.enableMipMap)
            this.init=true
        }
    }
    compose(renderer:IAriaRendererCore,proc: () => any,attrs:IAriaCanvasComposeAttributes={preserve:false}) {
        this.canvasUse(renderer)
        if(attrs.preserve==false){
            this.fbo?.onClear(renderer)
        }
        proc()
        this.canvasDetach(renderer)
    }
    getTex(): AriaComTexture {
        if(!this.fbo){
            this._logError("Empty framebuffer")
            throw Error()
        }
        return this.fbo.getTex()
    }
    canvasUse(renderer:IAriaRendererCore): void {
        if(this.init==false){
            this.createFBO(renderer,this.scale,this.enableHdr,this.enableMipMap)
            this.init=true
        }
        if(!this.fbo){
            this._logError("Empty framebuffer")
            throw Error()
        }
        this.fbo.bind(renderer)
        AriaComCanvasManager.getInstance().setCanvas(this)
    }
    canvasDetach(renderer:IAriaRendererCore): void {
        if(!this.fbo){
            this._logError("Empty framebuffer")
            throw Error()
        }
        this.fbo.unbind(renderer)
        AriaComCanvasManager.getInstance().detachCanvas(renderer)
    }
}