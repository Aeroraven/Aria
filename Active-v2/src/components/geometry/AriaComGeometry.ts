import { AriaComponent } from "../../core/AriaComponent";
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
    constructor(name:string){
        super(name)
    }
    exportToShader(): void {
        this._logError("exportToShader: Method not implemented")
    }
    getVertexNumber(): number{
        this._logError("getVertexNumber: Method not implemented")
        return 0
    }
}