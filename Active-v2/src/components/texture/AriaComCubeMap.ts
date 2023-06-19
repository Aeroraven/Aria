import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
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
            this.texLoadProcedure = (renderer:IAriaRendererCore)=>{
                this.setTex(renderer.createCubicTexture(images))
            }
        }
    }
    
}