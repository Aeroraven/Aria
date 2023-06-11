import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaComTexture } from "../base/AriaComTexture";

export class AriaComCubeMap extends AriaComTexture{
    _renderer:IAriaRendererCore
    constructor(renderer:IAriaRendererCore){
        super(renderer)
        this._renderer= renderer
        this._rename("AriaCom/CubeMap")
    }
    loadFromImage(images: HTMLImageElement|HTMLImageElement[]): void {
        if(images instanceof HTMLImageElement){
            this._logError("cubemap requires more than one image")
        }else{
            this.setTex(this._renderer.createCubicTexture(images))
        }
    }
    
}