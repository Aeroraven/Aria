import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaComFramebuffer, AriaFramebufferOption } from "../base/AriaComFramebuffer";
import { AriaComCanvas } from "./AriaComCanvas";

export class AriaComCubicCanvasEx extends AriaComCanvas{
    w:number
    h:number
    constructor(w:number,h:number,enableHdr:boolean=true,enableMipMap:boolean=false){
        super(1,enableHdr,enableMipMap)
        this.w = w
        this.h = h
    }
    protected override createFBO(renderer:IAriaRendererCore,scale: number=1, enableHdr: boolean=true, enableMipMap: boolean=false): void {
        const fboOpt = AriaFramebufferOption.create().setWH(this.w,this.h).setHdr(enableHdr).setMipMap(enableMipMap)
        this.fbo = new AriaComFramebuffer(renderer,fboOpt)
    }
}