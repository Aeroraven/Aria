import { AriaComponent } from "../../core/AriaComponent"
import { AriaEnv } from "../../core/graphics/AriaEnv"
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore"
import { IAriaTexture } from "../../core/interface/IAriaTexture"

export class AriaComTexture extends AriaComponent implements IAriaTexture{
    protected tex:WebGLTexture|null = null
    protected texLoadProcedure = (renderer:IAriaRendererCore)=>{}
    protected texLoaded = false
    constructor(){
        super("AriaCom/Texture")
    }
    setTex(o:WebGLTexture){
        this.tex = o
        if(o==null&&this.id!=32){
            this._logError("Invalid Operation")
        }
        this.texLoaded = true
    }
    getTex(renderer:IAriaRendererCore){
        if(this.texLoaded==false){
            this.texLoadProcedure(renderer)
            this.texLoaded=true
        }
        if(this.tex==null){
            this._logWarn("Cannot export null texture")
        }
        return <WebGLTexture>this.tex
    }
    loadFromImage(image:HTMLImageElement|HTMLImageElement[]){
        if(image instanceof HTMLImageElement){
            this.texLoadProcedure = (renderer)=>{
                const txw = renderer.createTexture(image)
                this.setTex(txw)
            }
        }else{
            this._logError("image format not supported")
        }
    }
}