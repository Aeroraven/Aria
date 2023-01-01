import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/graphics/AriaEnv";
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer";

export class AriaComVAO extends AriaComponent implements IAriaGLBuffer{
    private _buf:WebGLBuffer
    constructor(bf:WebGLBuffer|null = null){
        super("AriaCom/VAO")
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
        AriaEnv.env.bindBuffer(AriaEnv.env.ARRAY_BUFFER, this._buf)
    }

    public unbind(){
        AriaEnv.env.bindBuffer(AriaEnv.env.ARRAY_BUFFER, null)
    }

    public setData(v:number[]){
        this.bind()
        AriaEnv.env.bufferData(AriaEnv.env.ARRAY_BUFFER, new Float32Array(v), AriaEnv.env.STATIC_DRAW)
        this.unbind()
    }
    public setDataA(v:WebGLBuffer){
        this._buf = v
    }
}