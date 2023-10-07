import { AriaCamera } from "../../core/aria-camera";
import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { AriaComMesh, AriaComMeshTextureType, IAriaRenderable } from "../core/aria-com-mesh";
import { IAriaComLight } from "../core/interfaces/aria-com-light";
import { IAriaComShadowBaker } from "../core/interfaces/aria-com-shadow-baker";
import { AriaComRect } from "../geometry/aria-com-rect";

export class AriaComPostprocessPass extends AriaComponent implements IAriaRenderable,IAriaComShadowBaker{
    mesh:AriaComMesh

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        const geometry = (<AriaComRect>AriaComRect.create(gl))
        const buffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(geometry)
        this.mesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setBuffer(buffer)
            .setCamera(new AriaCamera())
    }
    renderLightDepthMap(x: AriaComponent & IAriaComLight, id: number): void {
        throw new Error("Method not supported.");
    }
    renderLightDepthMapS(id: number): void {
        throw new Error("Method not supported.");
    }
    render(): void {
        this.mesh.render()
    }
    setTexture(tp:AriaComMeshTextureType,o:AriaComponent){
        this.mesh.setTexture(tp,o)
        return this
    }
    setShader(s:AriaShader){
        this.mesh.setShader(s)
        return this
    }
    setCamera(s:AriaCamera){
        this.mesh.setCamera(s)
        return this
    }
    
}