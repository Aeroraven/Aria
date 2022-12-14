import { AriaComponent } from "../../core/AriaComponent";
import { AriaAsyncURLOp } from "../../core/network/AriaAsyncURLOp";
import { IAriaTexture } from "../../core/interface/IAriaTexture"
import { IAriaComShaderSource } from "../base/interface/IAriaComShaderSource";
import { AriaComTexture } from "../../components/base/AriaComTexture";
import { AriaAsyncImageOp } from "../../core/network/AriaAsyncImageOp";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";

export class AriaComTextureLoader extends AriaComponent{
    constructor(){
        super("AriaCom/TextureLoader")
    }
    public async load(path:string):Promise<IAriaTexture>{
        this._logInfo("Loading texture:"+path)
        const vp = await AriaAsyncImageOp.loadImage(path)
        this._logInfo("Loaded texture:"+path)
        const txw = AriaRenderOps.createTexture(vp)
        const tx = new AriaComTexture()
        tx.setTex(txw)
        return tx
    }
}