import { AriaComponent } from "../../core/AriaComponent";
import { IAriaCoreBuffer } from "../../core/base/AriaRendererCompatDef";
import { IAriaBufferReader } from "../../core/interface/IAriaBufferReader";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComEBO extends AriaComponent implements IAriaGLBuffer,IAriaBufferReader<Uint16Array>{
    private _buf:IAriaCoreBuffer
    private _bufLen:number = -1
    private _data:number[] = []
    private _dataUpdated = false
    private _valid = false

    constructor(bf:IAriaCoreBuffer|null = null, length:number = -1){
        super("AriaCom/EBO")
        if(bf==null){
            this._buf = <IAriaCoreBuffer>{data:null}
        }else{
            this._buf = bf
            this._bufLen = length
            this._valid = true
            this._dataUpdated = true
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
            this._buf = renderer.createElementBuffer()
        }
        renderer.useElementBuffer(this._buf)
        if(this._dataUpdated==false){
            renderer.setElementBufferData(this._buf,new Uint16Array(this._data))
            this._dataUpdated = true
        }
    }

    public unbind(renderer:IAriaRendererCore){
        //renderer.getEnv().bindBuffer(renderer.getEnv().ELEMENT_ARRAY_BUFFER, null)
    }

    public setData(v:number[], length:number = -1){
        this._data = v
        this._dataUpdated = false
        this._bufLen = length
        
    }

    public setDataA(v:IAriaCoreBuffer){
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