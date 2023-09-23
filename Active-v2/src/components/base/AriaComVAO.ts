import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/graphics/AriaEnv";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { IAriaBufferReader } from "../../core/interface/IAriaBufferReader";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComVAO extends AriaComponent implements IAriaGLBuffer,IAriaBufferReader<Float32Array>{
    private _buf:WebGLBuffer
    private _bufLen:number = -1
    private _data:number[] = []
    private _dataUpdated = false
    private _valid = false

    constructor(bf:WebGLBuffer|null = null, length:number = -1){
        super("AriaCom/VAO")
        if(bf==null){
            this._buf = <WebGLBuffer>0
        }else{
            this._buf = bf
            this._bufLen = length
            this._valid = true
            this._dataUpdated = true
        }
        
    }
    public getGLObject(): WebGLBuffer {
        if(this._valid==false){
            this._logError("ariacom.vao: not initialized")
        }
        return this._buf
    }

    public bind(renderer:IAriaRendererCore){
        if(this._valid==false){
            this._buf = renderer.getEnv().createBuffer()!
            this._valid = true
        }
        renderer.getEnv().bindBuffer(renderer.getEnv().ARRAY_BUFFER, this._buf)
        if(this._dataUpdated==false){
            renderer.getEnv().bufferData(renderer.getEnv().ARRAY_BUFFER, new Float32Array(this._data), renderer.getEnv().STATIC_DRAW)
            this._dataUpdated=true
        }
    }

    public unbind(renderer:IAriaRendererCore){
        renderer.getEnv().bindBuffer(renderer.getEnv().ARRAY_BUFFER, null)
    }

    public setData(v:number[],length:number = -1){
        this._data = v
        this._bufLen = length
        this._dataUpdated=false
    }
    public setDataA(v:WebGLBuffer){
        this._buf = v
    }

    public getRawData(renderer:IAriaRendererCore,steps:number = 3){
        if(this._bufLen == -1){
            this._logError("Buffer length is not specified")
        }
        let arb = new ArrayBuffer(this._bufLen * steps * Float32Array.BYTES_PER_ELEMENT)
        this.bind(renderer)
        renderer.readArrayBuffer(arb)
        this.unbind(renderer)
        return new Float32Array(arb)
    }

}