import { AriaComponent } from "../../core/AriaComponent";
import { AriaAsyncURLOp } from "../../core/network/AriaAsyncURLOp";
import { IAriaComShaderSource } from "../base/interface/IAriaComShaderSource";

export class AriaComShaderLoader extends AriaComponent{
    constructor(){
        super("AriaCom/ShaderLoader")
    }
    public async load(vertexPath:string, fragmentPath:string):Promise<IAriaComShaderSource>{
        const vp = await AriaAsyncURLOp.fetchText(vertexPath)
        const fp = await AriaAsyncURLOp.fetchText(fragmentPath)
        return {
            vertex:vp,
            fragment:fp
        }
    }

    public async loadFolder(path:string):Promise<IAriaComShaderSource>{
        return await this.load(path+"/vertex.glsl",path+"/fragment.glsl")
    }
}