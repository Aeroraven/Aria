import { AriaComponent } from "../../core/AriaComponent"
import { AriaEnv } from "../../core/graphics/AriaEnv"
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore"
import { IAriaTexture } from "../../core/interface/IAriaTexture"

export class AriaComTexture extends AriaComponent implements IAriaTexture{
    protected tex:WebGLTexture|null = null
    protected renderer:IAriaRendererCore
    constructor(renderer:IAriaRendererCore){
        super("AriaCom/Texture")
        const gl = AriaEnv.env
        this.renderer = renderer
    }
    setTex(o:WebGLTexture){
        this.tex = o
    }
    getTex(){
        if(this.tex==null){
            this._logWarn("Cannot export null texture")
        }
        return <WebGLTexture>this.tex
    }
    loadFromImage(image:HTMLImageElement|HTMLImageElement[]){
        if(image instanceof HTMLImageElement){
            const txw = this.renderer.createTexture(image)
            this.setTex(txw)
        }else{
            this._logError("image format not supported")
        }
        
    }
}