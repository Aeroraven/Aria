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
    ASU_TEX3D = "tex3d",
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

}