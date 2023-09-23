import { AriaComponent } from "../../core/AriaComponent";
import { IAriaCoreBuffer } from "../../core/base/AriaRendererCompatDef";
import { IAriaBufferReader } from "../../core/interface/IAriaBufferReader";
import { IAriaGeneralBuffer } from "../../core/interface/IAriaGeneralBuffer";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComVAO extends AriaComponent implements IAriaGeneralBuffer,IAriaBufferReader<Float32Array>{
    private _buf:IAriaCoreBuffer
    private _bufLen:number = -1
    private _data:number[] = []
    private _dataUpdated = false
    private _valid = false

    constructor(bf:IAriaCoreBuffer|null = null, length:number = -1){
        super("AriaCom/VAO")
        if(bf==null){
            this._buf = <IAriaCoreBuffer>{data:0}
        }else{
            this._buf = bf
            this._bufLen = length
            this._valid = true
            this._dataUpdated = true
        }
        
    }
    public getGeneralBufferObject(): IAriaCoreBuffer {
        if(this._valid==false){
            this._logError("ariacom.vao: not initialized")
        }
        return this._buf
    }

    public bind(renderer:IAriaRendererCore){
        if(this._valid==false){
            this._buf = renderer.createArrayBuffer()
            this._valid = true
        }
        renderer.useArrayBuffer(this._buf)
        if(this._dataUpdated==false){
            renderer.setArrayBufferData(this._buf, new Float32Array(this._data))
            this._dataUpdated=true
        }
    }

    public unbind(renderer:IAriaRendererCore){
        //renderer.getEnv().bindBuffer(renderer.getEnv().ARRAY_BUFFER, null)
    }

    public setData(v:number[],length:number = -1){
        this._data = v
        this._bufLen = length
        this._dataUpdated=false
    }
    public setDataA(v:IAriaCoreBuffer){
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