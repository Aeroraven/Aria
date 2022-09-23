import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComShaderEmitter } from "../core/interfaces/aria-com-shader-emitter";

export class AriaComGeometryInstancing extends AriaComponent implements IAriaComShaderEmitter{
    num: number
    pos: number[]
    gl:WebGL2RenderingContext

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.num = 0
        this.pos = []
        this.gl = gl
    }

    emitUniforms(shader: AriaShader): void {
        const gl = this.gl
        for(let i=0;i<this.num;i++){
            gl.uniform3f(shader.getUniform("uInstPos["+i+"]"),this.pos[i*3],this.pos[i*3+1],this.pos[i*3+2])
        }
    }
    setNumber(x:number){
        this.num = x
        return this
    }

    generateUniform(){
        this.pos = []
        for(let i=0;i<this.num * 3;i++){
            this.pos.push(Math.random()*4.0-2.0)
        }
        return this
    }

    protected register(): void {
        
    }
}