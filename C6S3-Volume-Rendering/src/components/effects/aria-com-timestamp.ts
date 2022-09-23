import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComShaderEmitter } from "../core/interfaces/aria-com-shader-emitter";

export class AriaComTimestamp extends AriaComponent implements IAriaComShaderEmitter{
    
    gl:WebGL2RenderingContext
    ts:number
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
        this.ts = Date.now()
    }
    emitUniforms(shader: AriaShader): void {
        const gl = this.gl
        gl.uniform1f(shader.getUniform("uTimestamp"),Date.now()-this.ts)
    }
    protected override register(): void {
        
    }
}