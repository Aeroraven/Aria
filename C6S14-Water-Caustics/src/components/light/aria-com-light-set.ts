import { AriaCamera } from "../../core/aria-camera";
import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComLight } from "../core/interfaces/aria-com-light";

export class AriaComLightSet extends AriaComponent implements IAriaComLight{
    lightPos:number[][]
    lightColor:number[][]
    lightType:number[]
    lightCam:AriaCamera[]
    gl:WebGL2RenderingContext
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
        this.lightPos = []
        this.lightColor = []
        this.lightType = []
        this.lightCam = []
    }
    protected register(): void {
        
    }
    addPointLight(pos:number[],col:number[]){
        this.lightPos.push(pos)
        this.lightColor.push(col)
        this.lightType.push(0)

        const lightCamera = new AriaCamera()
        this.lightCam.push(lightCamera)

        return this
    }
    addDirectionalLight(pos:number[],col:number[],projPos:number[] = [0,9,0]){
        this.lightPos.push(pos)
        this.lightColor.push(col)
        this.lightType.push(1)
        const lightCamera = new AriaCamera()
        lightCamera.camFront = new Float32Array(pos)
        lightCamera.camPos = new Float32Array(projPos)
        if(lightCamera.camFront[0]==0&&lightCamera.camFront[2]==0){
            lightCamera.camUp = new Float32Array([0,0,1])
        }
        lightCamera.movePos(0,0,0)
        this.lightCam.push(lightCamera)
        return this
    }

    emitUniforms(shader: AriaShader): void {
        const gl = this.gl
        for(let i=0;i<this.lightPos.length;i++){
            gl.uniform3f(shader.getUniform("uLightPos["+i+"]"),this.lightPos[i][0],this.lightPos[i][1],this.lightPos[i][2])
            gl.uniform3f(shader.getUniform("uLightColor["+i+"]"),this.lightColor[i][0],this.lightColor[i][1],this.lightColor[i][2])      
            gl.uniform1i(shader.getUniform("uLightType["+i+"]"),this.lightType[i])       
            gl.uniformMatrix4fv(shader.getUniform("uLightModel["+i+"]"),false,this.lightCam[i].camLookAt)    
            //console.log(this.lightCam[i].camLookAt)
        }
    }
    
}