import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComShaderEmitter } from "../core/interfaces/aria-com-shader-emitter";

export class AriaComFog extends AriaComponent implements IAriaComShaderEmitter{
    fogColor:number[]
    fogNear:number
    fogFar:number
    gl:WebGL2RenderingContext

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
        this.fogColor = [0,0,0,0]
        this.fogNear = 0.1
        this.fogFar = 2.0
    }

    emitUniforms(shader: AriaShader): void {
        const gl = this.gl
        gl.uniform3f(shader.getUniform("uFogColor"),this.fogColor[0],this.fogColor[1],this.fogColor[2])
        gl.uniform1f(shader.getUniform("uFogNear"),this.fogNear)
        gl.uniform1f(shader.getUniform("uFogFar"),this.fogFar)
           
    }

    setFogColor(r:number,g:number,b:number){
        this.fogColor[0] = r
        this.fogColor[1] = g
        this.fogColor[2] = b
        return this
    }

    setFogDist(near:number,far:number){
        this.fogNear = near
        this.fogFar = far
        return this
    }

    protected register(): void {
        
    }
}