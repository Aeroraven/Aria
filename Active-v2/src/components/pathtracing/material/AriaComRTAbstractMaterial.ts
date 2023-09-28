import { AriaVec3, AriaVec4, AriaVec4C } from "../../../core/arithmetic/AriaVector";
import { AriaComRTFragShaderInjector } from "../AriaComRTFragShaderInjector";
import { IAriaComRTShaderIdentifier } from "../base/IAriaComRTShaderIdentifier";
import { IAriaComRTAbstractMaterial } from "./IAriaComRTAbstractMaterial";

export abstract class AriaComRTAbstractMaterial extends AriaComRTFragShaderInjector implements IAriaComRTAbstractMaterial,IAriaComRTShaderIdentifier{
    private emissionColor:AriaVec4
    private materialColor:AriaVec4
    constructor(){
        super()
        this._rename("AriaCom/RTAbstractMaterial")
        this.emissionColor = AriaVec4.create()
        this.materialColor = AriaVec4.create()
    }
    getIdentifier(): string {
        throw new Error("Method not implemented.");
    }
    injectMaterialUpdates(){
        return this.getArgs().ACRTS_UPDATE_MATERIAL_COLOR+"="+this.emitVec4(this.materialColor) +";\n"
        + this.getArgs().ACRTS_UPDATE_EMISSION_COLOR+"="+this.emitVec4(this.emissionColor)+";\n"
    }
    setEmission(x:AriaVec4C){
        this.emissionColor.fromArray(x)
    }
    setMaterial(x:AriaVec4C){
        this.materialColor.fromArray(x)
    }
}