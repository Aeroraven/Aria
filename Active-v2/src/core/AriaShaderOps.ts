import { AriaEnv } from "./AriaEnv";
import { AriaObject } from "./base/AriaObject";
import { IAriaGLBuffer } from "./interface/IAriaGLBuffer";
import { IAriaShader } from "./interface/IAriaShader";
import { IAriaTexture } from "./interface/IAriaTexture";

export enum AriaShaderUniformTp{
    ASU_MAT4 = "mat4",
    ASU_VEC3 = "vec3",
    ASU_VEC1 = "vec1",
    ASU_TEX2D = "tex2d"
}

class AriaShaderManager extends AriaObject{
    private activatedShader:IAriaShader|null = null
    private static instance:AriaShaderManager|null = null
    public static getInst(){
        if(this.instance==null){
            this.instance=new AriaShaderManager()
        }
        return <AriaShaderManager>this.instance
    }
    constructor(){
        super("AriaShaderManager")
    }
    public setShader(x:IAriaShader){
        this.activatedShader = x
    }
    public getShader(){
        if(this.activatedShader==null){
            this._logError("Invalid shader operation")
        }
        return <IAriaShader>this.activatedShader
    }
}

export class AriaShaderOps extends AriaObject{
    constructor(){
        super("AriaShaderOps")
    }

    public static useShader(shader:IAriaShader){
        AriaShaderManager.getInst().setShader(shader)
    }

    public static defineAttribute(attName:string, value:IAriaGLBuffer, size:number = 3, type = AriaEnv.env.FLOAT){
        const gl = AriaEnv.env
        const acShader = AriaShaderManager.getInst().getShader()
        gl.bindBuffer(gl.ARRAY_BUFFER, value.getGLObject())
        gl.vertexAttribPointer(acShader.getAttribute(attName), size, type, false, 0, 0)
        gl.enableVertexAttribArray(acShader.getAttribute(attName))
    }

    public static defineUniform(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)){
        const gl = AriaEnv.env
        const acShader = AriaShaderManager.getInst().getShader();
        
        const isArrayGuard = (x:any):x is number[]=>{
            if(x instanceof Array){
                return true
            }
            return false
        }
        const isNumGuard = (x:any):x is number=>{
            if (typeof x == 'number'){
                return true
            }
            return false
        }
        const isFloat32 = (x:any):x is Float32Array=>{
            if (x instanceof Float32Array){
                return true
            }
            return false
        }
        const isTex = (x:any):x is IAriaTexture=>{
            return true
        }
        switch (type) {
            case AriaShaderUniformTp.ASU_MAT4:
                if(isArrayGuard(value)){
                    gl.uniformMatrix4fv(acShader.getUniform(attName),false,new Float32Array(value))
                }
                else if(isFloat32(value)){
                    gl.uniformMatrix4fv(acShader.getUniform(attName),false,value)
                }else{
                    (new this())._logError("Invalid type")
                }
                break;
            
            case AriaShaderUniformTp.ASU_VEC3:
                if(isArrayGuard(value)){
                    gl.uniform3fv(acShader.getUniform(attName),new Float32Array(value))
                }
                else if(isFloat32(value)){
                    gl.uniform3fv(acShader.getUniform(attName),value)
                }else{
                    (new this())._logError("Invalid type")
                }
                break;
            
            case AriaShaderUniformTp.ASU_VEC1:
                if(isNumGuard(value)){
                    gl.uniform1f(acShader.getUniform(attName),value)
                }else{
                    (new this())._logError("Invalid type")
                }
                break;
            case AriaShaderUniformTp.ASU_TEX2D:
                if(isTex(value)){
                    const v = acShader.allocateTexture()
                    gl.activeTexture(v)
                    gl.bindTexture(gl.TEXTURE_2D, value.getTex())
                    gl.uniform1i(acShader.getUniform(attName),v-AriaEnv.env.TEXTURE0);
                }else{
                    (new this())._logError("Invalid type")
                }
                break;
            default:
                (new this())._logError("Invalid type:" + type)
                break;
        }
    }
}