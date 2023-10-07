import { AriaComponent } from "../../core/AriaComponent"
import { IAriaCoreTexture } from "../../core/base/AriaRendererCompatDef"
import { IAriaBufferReader } from "../../core/interface/IAriaBufferReader"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore"
import { IAriaTexture } from "../../core/interface/IAriaTexture"
import { AriaTextureDataType, AriaTextureFormat, IAriaTextureArgs } from "../../core/interface/IAriaTextureArgs"

export class AriaComTexture extends AriaComponent implements IAriaTexture,IAriaBufferReader<unknown>{
    protected tex:IAriaCoreTexture|null = null
    protected texLoadProcedure = (renderer:IAriaRendererCore)=>{}
    protected texLoaded = false
    protected texArgs:IAriaTextureArgs = {format:AriaTextureFormat.ATF_MONOCHANNEL,datatype:AriaTextureDataType.ATDT_UNSIGNED_BYTE}
    constructor(){
        super("AriaCom/Texture")
    }
    getRawData(renderer: IAriaRendererCore): unknown {
        if(this.texArgs.w==undefined || this.texArgs.h==undefined){
            this._logError("aria.texture: get raw data not supported")
        }
        let dataType = undefined
        if(this.texArgs.datatype == AriaTextureDataType.ATDT_UNSIGNED_BYTE){
            dataType = WebGL2RenderingContext.UNSIGNED_BYTE
        }
        let format = undefined
        if(this.texArgs.format == AriaTextureFormat.ATF_MONOCHANNEL){
            dataType = WebGL2RenderingContext.R8
        }
        return renderer.getTextureBufferData(this.tex!,dataType!,dataType!,this.texArgs.w!,this.texArgs.h!)
    }
    setTex(o:IAriaCoreTexture,args?:IAriaTextureArgs){
        this.tex = o
        this.texLoaded = true
        if(args!=undefined){
            this.texArgs = args
        }
    }
    getTex(renderer:IAriaRendererCore){
        if(this.texLoaded==false){
            this.texLoadProcedure(renderer)
            this.texLoaded=true
        }
        if(this.tex==null){
            this._logWarn("Cannot export null texture")
        }
        return <IAriaCoreTexture>this.tex
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