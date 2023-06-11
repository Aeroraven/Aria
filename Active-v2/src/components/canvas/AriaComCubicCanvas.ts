import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaComFramebuffer, AriaFramebufferOption } from "../base/AriaComFramebuffer";
import { AriaComCanvas } from "./AriaComCanvas";

export class AriaComCubicCanvas extends AriaComCanvas{
    constructor(scale:number=1,enableHdr:boolean=true,enableMipMap:boolean=false){
        super(scale,enableHdr,enableMipMap)
    }
    protected override createFBO(renderer:IAriaRendererCore,scale: number=1, enableHdr: boolean=true, enableMipMap: boolean=false): void {
        const fboOpt = AriaFramebufferOption.create().setScaler(scale).setHdr(enableHdr).setMipMap(enableMipMap).setCubic(true)
        this.fbo = new AriaComFramebuffer(renderer,fboOpt)
    }
}