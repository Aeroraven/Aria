import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/AriaEnv";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";

export class AriaComEBO extends AriaComponent implements IAriaGLBuffer{
    private _buf:WebGLBuffer
    constructor(bf:WebGLBuffer|null = null){
        super("AriaCom/EBO")
        if(bf==null){
            this._buf = <WebGLBuffer>AriaEnv.env.createBuffer()
        }else{
            this._buf = bf
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

    public setData(v:number[]){
        this.bind()
        AriaEnv.env.bufferData(AriaEnv.env.ELEMENT_ARRAY_BUFFER, new Uint16Array(v), AriaEnv.env.STATIC_DRAW)
        this.unbind()
    }

    public setDataA(v:WebGLBuffer){
        this._buf = v
    }
}