import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComMaterial } from "../core/interfaces/aria-com-material";

export class AriaComSimplePBR extends AriaComponent implements IAriaComMaterial{
    albedo:number[]
    roughness:number
    metallic:number
    ao:number
    gl:WebGL2RenderingContext

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
        this.albedo = [0,0,0]
        this.roughness = 0
        this.metallic =  1
        this.ao = 0
    }
    protected override register(): void {
        
    }
    setAlbedo(dx:number,dy:number,dz:number){
        this.albedo[0] = dx
        this.albedo[1] = dy
        this.albedo[2] = dz
        return this
    }

    setRoughness(dv:number){
        this.roughness = dv
        return this
    }

    setMetallic(dv:number){
        this.metallic = dv
        return this
    }

    setAO(dv:number){
        this.ao = dv
        return this;
    }

    emitUniforms(shader:AriaShader): void {
        const gl = this.gl
        gl.uniform3f(shader.getUniform("uS_Albedo"),this.albedo[0],this.albedo[1],this.albedo[2])
        gl.uniform1f(shader.getUniform("uS_Roughness"),this.roughness)
        gl.uniform1f(shader.getUniform("uS_Metallic"),this.metallic)
        gl.uniform1f(shader.getUniform("uS_AO"),this.ao)
    }
    
}