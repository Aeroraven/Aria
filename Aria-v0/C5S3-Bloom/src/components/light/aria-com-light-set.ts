import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComLight } from "../core/interfaces/aria-com-light";

export class AriaComLightSet extends AriaComponent implements IAriaComLight{
    lightPos:number[][]
    lightColor:number[][]
    gl:WebGL2RenderingContext
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
        this.lightPos = []
        this.lightColor = []
    }
    protected register(): void {
        
    }
    addLight(pos:number[],col:number[]){
        this.lightPos.push(pos)
        this.lightColor.push(col)
        return this
    }

    emitUniforms(shader: AriaShader): void {
        const gl = this.gl
        for(let i=0;i<this.lightPos.length;i++){
            gl.uniform3f(shader.getUniform("uLightPos"),this.lightPos[i][0],this.lightPos[i][1],this.lightPos[i][2])
            gl.uniform3f(shader.getUniform("uLightColor"),this.lightColor[i][0],this.lightColor[i][1],this.lightColor[i][2])       
        }
    }
    
}