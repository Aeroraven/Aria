import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { AriaComGeometryAttribEnum, IAriaComGeometry } from "./aria-com-geometry";

export class AriaComMeshGeometry extends AriaComponent implements IAriaComGeometry{
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
    }
    setNumVertices(x:number){
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
            p.getBuffer().setNumVertices(x)
        }else{
            throw new Error("Parent should be AriaComBuffers")
        }
        return this
    }
    setBuffer(bufName:AriaComGeometryAttribEnum, buffer:WebGLBuffer, type:number|undefined, size:number|undefined){
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
            p.getBuffer().set(bufName,buffer,type,size)
            //console.log("SET BUFFER" + bufName)
        }else{
            throw new Error("Parent should be AriaComBuffers")
        }
        return this
    }
    protected register(): void {
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
        }else{
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}