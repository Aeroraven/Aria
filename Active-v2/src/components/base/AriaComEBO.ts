import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/graphics/AriaEnv";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComEBO extends AriaComponent implements IAriaGLBuffer{
    private _buf:WebGLBuffer
    private _bufLen:number = -1
    private _data:number[] = []
    private _dataUpdated = false
    private _valid = false

    constructor(bf:WebGLBuffer|null = null, length:number = -1){
        super("AriaCom/EBO")
        if(bf==null){
            this._buf = <WebGLBuffer>0
        }else{
            this._buf = bf
            this._bufLen = length
            this._valid = true
        }
    }

    public getGLObject(): WebGLBuffer {
        if(this._valid==false){
            this._logError("ariacom.ebo: not initialized")
        }
        return this._buf
    }

    public bind(renderer:IAriaRendererCore){
        if(this._valid==false){
            this._valid=true
            this._buf = <WebGLBuffer>renderer.getEnv().createBuffer()
        }
        renderer.getEnv().bindBuffer(renderer.getEnv().ELEMENT_ARRAY_BUFFER, this._buf)
        if(this._dataUpdated==false){
            renderer.getEnv().bufferData(renderer.getEnv().ELEMENT_ARRAY_BUFFER, new Uint16Array(this._data), renderer.getEnv().STATIC_DRAW)
            this._dataUpdated = true
        }
    }

    public unbind(renderer:IAriaRendererCore){
        renderer.getEnv().bindBuffer(renderer.getEnv().ELEMENT_ARRAY_BUFFER, null)
    }

    public setData(v:number[], length:number = -1){
        this._data = v
        this._dataUpdated = false
        this._bufLen = length
        
    }

    public setDataA(v:WebGLBuffer){
        this._buf = v
    }

    public getRawData(renderer:IAriaRendererCore){
        if(this._bufLen == -1){
            this._logError("Buffer length is not specified")
        }
        let arb = new ArrayBuffer(this._bufLen * Uint16Array.BYTES_PER_ELEMENT)
        this.bind(renderer)
        renderer.readElementBuffer(arb)
        this.unbind(renderer)
        return new Uint16Array(arb)

    }
}