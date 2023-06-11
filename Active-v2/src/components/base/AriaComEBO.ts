import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/graphics/AriaEnv";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComEBO extends AriaComponent implements IAriaGLBuffer{
    private _buf:WebGLBuffer
    private _bufLen:number = -1

    constructor(bf:WebGLBuffer|null = null, length:number = -1){
        super("AriaCom/EBO")
        if(bf==null){
            this._buf = <WebGLBuffer>AriaEnv.env.createBuffer()
        }else{
            this._buf = bf
            this._bufLen = length
        }
    }

    public getGLObject(): WebGLBuffer {
        return this._buf
    }

    public bind(){
        AriaEnv.env.bindBuffer(AriaEnv.env.ELEMENT_ARRAY_BUFFER, this._buf)
    }

    public unbind(){
        AriaEnv.env.bindBuffer(AriaEnv.env.ELEMENT_ARRAY_BUFFER, null)
    }

    public setData(v:number[], length:number = -1){
        this.bind()
        this._bufLen = length
        AriaEnv.env.bufferData(AriaEnv.env.ELEMENT_ARRAY_BUFFER, new Uint16Array(v), AriaEnv.env.STATIC_DRAW)
        this.unbind()
    }

    public setDataA(v:WebGLBuffer){
        this._buf = v
    }

    public getRawData(renderer:IAriaRendererCore){
        if(this._bufLen == -1){
            this._logError("Buffer length is not specified")
        }
        let arb = new ArrayBuffer(this._bufLen * Uint16Array.BYTES_PER_ELEMENT)
        this.bind()
        renderer.readElementBuffer(arb)
        this.unbind()
        return new Uint16Array(arb)

    }
}