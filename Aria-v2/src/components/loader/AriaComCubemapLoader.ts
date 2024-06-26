import { AriaComponent } from "../../core/AriaComponent";
import { IAriaTexture } from "../../core/interface/IAriaTexture"
import { AriaAsyncImageOp } from "../../core/network/AriaAsyncImageOp";
import { AriaComCubeMap } from "../texture/AriaComCubeMap";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComCubeMapLoader extends AriaComponent{
    constructor(){
        super("AriaCom/TextureLoader")
    }
    public async load(renderer:IAriaRendererCore,path:string):Promise<IAriaTexture>{
        const lst = [ 
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "front.jpg",
            "back.jpg"
        ]
        const imgs = []
        for(let i=0;i<6;i++){
            this._logInfo("Loading texture:"+path+"/"+lst[i])
            const vp = await AriaAsyncImageOp.loadImage(path+"/"+lst[i])
            this._logInfo("Loaded texture:"+path+"/"+lst[i])
            imgs.push(vp)
        }
        
        const txw = renderer.createCubicTexture(imgs)
        const tx = new AriaComCubeMap()
        tx.setTex(txw)
        return tx
    }
}