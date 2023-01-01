import { AriaBufferMap } from "../../core/aria-buffer-map";
import { AriaComponent } from "../base/aria-component";

export class AriaComBuffers extends AriaComponent{
    constructor(gl:WebGL2RenderingContext) {
        super(gl)
        this.setAttr("buffer_map",new AriaBufferMap())
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
