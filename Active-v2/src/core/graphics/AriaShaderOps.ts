import { AriaEnv } from "./AriaEnv";
import { AriaCallable } from "../base/AriaBaseDefs";
import { AriaObject } from "../base/AriaObject";
import { IAriaGLBuffer } from "../interface/IAriaGLBuffer";
import { IAriaShader } from "../interface/IAriaShader";
import { IAriaTexture } from "../interface/IAriaTexture";

export enum AriaShaderUniformTp{
    ASU_MAT4 = "mat4",
    ASU_VEC3 = "vec3",
    ASU_VEC4 = "vec4",
    ASU_VEC1 = "vec1",
    ASU_TEX2D = "tex2d",
    ASU_TEXCUBE = "texcube",
    ASU_VEC1I = "int1"
}

class AriaShaderManager extends AriaObject{
    private activatedShader:IAriaShader|null = null
    private extendUniformMaps: Map<string,number> = new Map<string,number>()
    private counterUniformMaps: Map<string,number> = new Map<string,number>()
    private invariantShader:boolean = false

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
    public disableShaderChange(){
        this.invariantShader = true
    }
    public enableShaderChange(){
        this.invariantShader = false
    }
    public setShader(x:IAriaShader){
        if(this.invariantShader==false){
            this.activatedShader = x
        }
        this.extendUniformMaps = new Map<string,number>()
        this.counterUniformMaps = new Map<string,number>()
        return this.invariantShader==false
    }
    public getShader(){
        if(this.activatedShader==null){
            this._logError("Invalid shader operation")
        }
        return <IAriaShader>this.activatedShader
    }
    public allocNewExtendUniform(s:string){
        const g = this.extendUniformMaps.get(s)
        if(!g){
            this.extendUniformMaps.set(s,1)
            return 0;
        }else{
            this.extendUniformMaps.set(s,g+1)
            return g
        }
    }
    public addCounterUniform(s:string, inc:number=1, returnAfter:boolean=false){
        const g = this.counterUniformMaps.get(s)
        if(!g){
            this.counterUniformMaps.set(s,inc);
            return 0 + (returnAfter?inc:0);
        }else{
            this.counterUniformMaps.set(s,g+inc)
            return g + (returnAfter?inc:0)
        }
    }
}

export class AriaShaderOps extends AriaObject{
    constructor(){
        super("AriaShaderOps")
    }

    public static useShader(shader:IAriaShader, onSuccess:AriaCallable=()=>{}){
        if(AriaShaderManager.getInst().setShader(shader)){
            onSuccess()
        }
    }
    public static useInvariantShader(shader:IAriaShader,effRange:()=>any){
        this.useShader(shader)
        AriaShaderManager.getInst().disableShaderChange()
        effRange()
        AriaShaderManager.getInst().enableShaderChange()
    }

    public static defineAttribute(attName:string, value:IAriaGLBuffer, size:number = 3, type:number = AriaEnv.env.FLOAT){
        const gl = AriaEnv.env
        const acShader = AriaShaderManager.getInst().getShader()
        gl.bindBuffer(gl.ARRAY_BUFFER, value.getGLObject())
        gl.vertexAttribPointer(acShader.getAttribute(attName), size, type, false, 0, 0)
        gl.enableVertexAttribArray(acShader.getAttribute(attName))
    }
    public static defineUniformCounter(attName:string, increment:number=1, returnAfter:boolean=true){
        let nVal = AriaShaderManager.getInst().addCounterUniform(attName, increment, returnAfter)
        this.defineUniform(attName,AriaShaderUniformTp.ASU_VEC1I, nVal)
        return nVal
    }
    public static defineUniformExtend(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture), index:number=-1){
        let nId = index
        if(index==-1){
            nId = AriaShaderManager.getInst().allocNewExtendUniform(attName)
        }
        const newAttName = attName + "[" + nId + "]"
        this.defineUniform(newAttName, type, value)
        return nId
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
            
            case AriaShaderUniformTp.ASU_VEC4:
                if(isArrayGuard(value)){
                    gl.uniform4fv(acShader.getUniform(attName),new Float32Array(value))
                }
                else if(isFloat32(value)){
                    gl.uniform4fv(acShader.getUniform(attName),value)
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
            case AriaShaderUniformTp.ASU_VEC1I:
                if(isNumGuard(value)){
                    gl.uniform1i(acShader.getUniform(attName),value)
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
            case AriaShaderUniformTp.ASU_TEXCUBE:
                    if(isTex(value)){
                        const v = acShader.allocateTexture()
                        gl.activeTexture(v)
                        gl.bindTexture(gl.TEXTURE_CUBE_MAP, value.getTex())
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