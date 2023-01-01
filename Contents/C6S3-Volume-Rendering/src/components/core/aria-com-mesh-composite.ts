import { AriaComponent } from "../base/aria-component";
import { IAriaRenderable } from "./aria-com-mesh";
import { IAriaComLight } from "./interfaces/aria-com-light";
import { IAriaComShadowBaker } from "./interfaces/aria-com-shadow-baker";



export class AriaComMeshComposite extends AriaComponent implements IAriaRenderable,IAriaComShadowBaker{
    containers:(IAriaRenderable&IAriaComShadowBaker)[]
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.containers = []
    }
    renderLightDepthMap(x: AriaComponent & IAriaComLight, id: number): void {
        throw new Error("Method not implemented.");
    }
    renderLightDepthMapS(id: number): void {
        this.containers.forEach(el=>{
            el.renderLightDepthMapS(id)
        })
    }
    addObject(o:IAriaRenderable&IAriaComShadowBaker){
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