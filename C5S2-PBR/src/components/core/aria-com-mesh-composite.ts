import { AriaComponent } from "../base/aria-component";
import { IAriaRenderable } from "./aria-com-mesh";



export class AriaComMeshComposite extends AriaComponent implements IAriaRenderable{
    containers:IAriaRenderable[]
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.containers = []
    }
    addObject(o:IAriaRenderable){
        this.containers.push(o)
    }
    render(): void {
        this.containers.forEach(el=>{
            el.render()
        })
    }
}

export class AriaComScene extends AriaComMeshComposite{

}