import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/graphics/AriaEnv";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";

export class AriaComVAO extends AriaComponent implements IAriaGLBuffer{
    private _buf:WebGLBuffer
    private _bufLen:number = -1

    constructor(bf:WebGLBuffer|null = null, length:number = -1){
        super("AriaCom/VAO")
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
        AriaEnv.env.bindBuffer(AriaEnv.env.ARRAY_BUFFER, this._buf)
    }

    public unbind(){
        AriaEnv.env.bindBuffer(AriaEnv.env.ARRAY_BUFFER, null)
    }

    public setData(v:number[],length:number = -1){
        this.bind()
        AriaEnv.env.bufferData(AriaEnv.env.ARRAY_BUFFER, new Float32Array(v), AriaEnv.env.STATIC_DRAW)
        this._bufLen = length
        this.unbind()
    }
    public setDataA(v:WebGLBuffer){
        this._buf = v
    }

    public getRawData(steps:number = 3){
        if(this._bufLen == -1){
            this._logError("Buffer length is not specified")
        }
        let arb = new ArrayBuffer(this._bufLen * steps * Float32Array.BYTES_PER_ELEMENT)
        this.bind()
        AriaRenderOps.readArrayBuffer(arb)
        this.unbind()
        return new Float32Array(arb)

    }

}