import { AriaComponent } from "../../core/AriaComponent";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCamera } from "../camera/AriaComCamera";
import { AriaComCanvas } from "../canvas/AriaComCanvas";
import { AriaComScene } from "../scene/AriaComScene";

export abstract class AriaRendererWrapper extends AriaComponent{
    constructor(){
        super("AriaCom/RendererWrapper")
    }
    abstract getEngine():any;
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