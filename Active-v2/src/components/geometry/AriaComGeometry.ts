import { mat3, mat4, vec3 } from "gl-matrix-ts";
import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/AriaShaderOps";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaGeometry } from "../base/interface/IAriaGeometry";

export enum AriaGeometryVars{
    AGV_POSITION = "aPos",
    AGV_TEXTURE_POSITION = "aTex",
    AGV_TANGENT = "aTangent",
    AGV_BITANGENT = "aBitangent",
    AGV_NORMAL = "aNormal"
}

export class AriaComGeometry extends AriaComponent implements IAriaShaderEmitter, IAriaGeometry{
    protected _localMat = mat4.create()
    protected _valid = false
    constructor(name:string){
        super(name)
        mat4.identity(this._localMat)
    }
    public localRotateX(r:number){
        mat4.rotateX(this._localMat,this._localMat,r)
    }
    public localRotateY(r:number){
        mat4.rotateY(this._localMat,this._localMat,r)
    }
    public localRotateZ(r:number){
        mat4.rotateZ(this._localMat,this._localMat,r)
    }
    public localTranslate(x:number,y:number,z:number){
        const tvec = vec3.create()
        tvec[0] = x
        tvec[1] = y
        tvec[2] = z
        mat4.translate(this._localMat,this._localMat,tvec)
    }
    public localScale(r:number|number[]){
        const tvec = vec3.create()
        if(typeof r == 'number'){
            tvec[0] = r
            tvec[1] = r
            tvec[2] = r
        }else{
            tvec[0] = r[0]
            tvec[1] = r[1]
            tvec[2] = r[2]
        }
        mat4.scale(this._localMat,this._localMat,tvec)
    }

    exportToShader(): void {
        if(!this._valid){
            this._logError("exportToShader: Method not implemented")
        }
        AriaShaderOps.defineUniform("uLocal",AriaShaderUniformTp.ASU_MAT4,this._localMat)
    }
    getVertexNumber(): number{
        this._logError("getVertexNumber: Method not implemented")
        return 0
    }
}