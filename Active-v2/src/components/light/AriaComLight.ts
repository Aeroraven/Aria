import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps } from "../../core/AriaShaderOps";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";

export enum AriaLightShaderVars{
    ALSV_POS = "uLightPos",
    ALSV_COLOR = "uLightColor",
    ALSV_TYPE = "uLightType"
}

export class AriaComLight extends AriaComponent implements IAriaShaderEmitter{
    protected _shaderId:number = -1
    protected _validLight = false
    constructor(name:string){
        super(name)
    }
    exportToShader(): void {
        this._shaderId = AriaShaderOps.defineUniformCounter("uLightCount")
        if(this._validLight==false){
            this._logError("light: exportToShader not implemented")
        }
    }
}