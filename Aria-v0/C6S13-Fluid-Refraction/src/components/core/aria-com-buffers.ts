import { AriaBufferMap } from "../../core/aria-buffer-map";
import { AriaComponent } from "../base/aria-component";
import { IAriaComFrameUpdateable } from "./interfaces/aria-com-frame-updateable";

export class AriaComBuffers extends AriaComponent implements IAriaComFrameUpdateable{
    constructor(gl:WebGL2RenderingContext) {
        super(gl)
        this.setAttr("buffer_map",new AriaBufferMap())
    }
    frameUpdate(): void {
        this.components.forEach((value,keys) => {
            if("frameUpdate" in value){
                (<IAriaComFrameUpdateable><unknown>value).frameUpdate()
            }
        });
    }
    protected register(): void {
        
    }
    protected respond(command: string, args: any[]) {
        const mp = <AriaBufferMap>this.getAttr("buffer_map")
        if(command=="add"){
            mp.set(<string>args[0],<WebGLBuffer>args[1])
        }
    }
    public getBuffer(){
        return <AriaBufferMap>this.getAttr("buffer_map")
    }
    public addGeometry(o:AriaComponent){
        this.addComponent("geometry",o)
        return this
    }

}
