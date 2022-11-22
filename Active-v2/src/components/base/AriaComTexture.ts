import { AriaComponent } from "../../core/AriaComponent"
import { AriaEnv } from "../../core/AriaEnv"
import { IAriaTexture } from "../../core/interface/IAriaTexture"

export class AriaComTexture extends AriaComponent implements IAriaTexture{
    tex:WebGLTexture|null = null
    constructor(){
        super("AriaCom/Texture")
        const gl = AriaEnv.env
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
}