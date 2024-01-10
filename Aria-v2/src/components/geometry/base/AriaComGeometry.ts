import { mat4, vec3 } from "gl-matrix-ts";
import { AriaComponent } from "../../../core/AriaComponent";
import { AriaShaderUniformTp } from "../../../core/graphics/AriaShaderOps";
import { IAriaShaderEmitter } from "../../../core/interface/IAriaShaderEmitter";
import { IAriaDynamicGeometry } from "../../base/interface/IAriaDynamicGeometry";
import { IAriaGeometry } from "../../base/interface/IAriaGeometry";
import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { AriaRenderEnumDrawingShape } from "../../../core/renderer/AriaRendererEnums";

export enum AriaGeometryVars{
    AGV_POSITION = "aPos",
    AGV_TEXTURE_POSITION = "aTex",
    AGV_TANGENT = "aTangent",
    AGV_BITANGENT = "aBitangent",
    AGV_NORMAL = "aNormal"
}

export class AriaComGeometry extends AriaComponent implements IAriaShaderEmitter, IAriaGeometry, IAriaDynamicGeometry{
    public _localMat = mat4.create()
    protected _valid = false
    protected _scaleRecord = [1,1,1]
    protected _position = [0,0,0]
    protected _drawingShape = AriaRenderEnumDrawingShape.TRIANGLE

    constructor(name:string){
        super(name)
        mat4.identity(this._localMat)
    }
    getRenderShape(): AriaRenderEnumDrawingShape {
        return this._drawingShape
    }
    getLocalPosition(): number[] {
        return [this._position[0],this._position[1],this._position[2]]
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

        this._position[0] += x * this._scaleRecord[0]
        this._position[1] += y * this._scaleRecord[1]
        this._position[2] += z * this._scaleRecord[2]
        mat4.translate(this._localMat,this._localMat,tvec)
    }
    public localTranslateAbsolute(x:number,y:number,z:number){
        const tvec = vec3.create()
        tvec[0] = x / this._scaleRecord[0]
        tvec[1] = y / this._scaleRecord[1]
        tvec[2] = z / this._scaleRecord[2]

        this._position[0] += x 
        this._position[1] += y 
        this._position[2] += z 
        mat4.translate(this._localMat,this._localMat,tvec)
    }

    public localPosition(x:number,y:number,z:number){
        this.localTranslateAbsolute(x-this._position[0],y-this._position[1],z-this._position[2])
    }

    public localScale(r:number|number[]){
        const tvec = vec3.create()
        if(typeof r == 'number'){
            tvec[0] = r
            tvec[1] = r
            tvec[2] = r
            this._scaleRecord[0] *= r
            this._scaleRecord[1] *= r
            this._scaleRecord[2] *= r
            
        }else{
            tvec[0] = r[0]
            tvec[1] = r[1]
            tvec[2] = r[2]
            this._scaleRecord[0] *= r[0]
            this._scaleRecord[1] *= r[1]
            this._scaleRecord[2] *= r[2]
            
        }
        mat4.scale(this._localMat,this._localMat,tvec)
    }
    exportToShader(renderer: IAriaRendererCore): void {
        if(!this._valid){
            this._logError("exportToShader: Method not implemented")
        }
        renderer.defineUniform("uLocal",AriaShaderUniformTp.ASU_MAT4,this._localMat)
    }
    getVertexNumber(): number{
        this._logError("getVertexNumber: Method not implemented")
        return 0
    }
}