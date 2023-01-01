import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { AriaComTexture } from "../base/AriaComTexture";

export class AriaComCubeMap extends AriaComTexture{
    constructor(){
        super()
        this._rename("AriaCom/CubeMap")
    }
    loadFromImage(images: HTMLImageElement|HTMLImageElement[]): void {
        if(images instanceof HTMLImageElement){
            this._logError("cubemap requires more than one image")
        }else{
            this.setTex(AriaRenderOps.createCubicTexture(images))
        }
    }
    
}