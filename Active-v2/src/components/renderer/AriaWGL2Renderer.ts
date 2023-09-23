import { AriaWGL2RendererCore } from "../../core/renderer/AriaWGL2RendererCore";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCamera } from "../camera/AriaComCamera";
import { AriaComCanvas } from "../canvas/AriaComCanvas";
import { AriaComScene } from "../scene/AriaComScene";
import { AriaRendererWrapper } from "./AriaRendererWrapper";

export class AriaWGL2Renderer extends AriaRendererWrapper{
    env:AriaWGL2RendererCore
    constructor(canvasId:string){
        super()
        this.env = new AriaWGL2RendererCore(canvasId)
    }
    public getEngine(){
        return this.env
    }
    public renderScene(camera:AriaComCamera, scene:AriaComScene){
        scene.render(this.getEngine(),[
            (x)=>{camera.exportToShader(x)}
        ])
    }
    public renderComposite(canvas:AriaComCanvas,procedure:()=>any){
        canvas.compose(this.getEngine(),procedure)
    }
    public renderSimple(postpass:IAriaRenderable){
        postpass.render(this.getEngine())
    }
    
}