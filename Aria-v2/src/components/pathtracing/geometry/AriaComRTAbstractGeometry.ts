import { AriaComRTFragShaderInjector } from "../AriaComRTFragShaderInjector";
import { IAriaComRTAbstractGeometry } from "./IAriaComRTAbstractGeometry";

export abstract class AriaComRTAbstractGeometry extends AriaComRTFragShaderInjector implements IAriaComRTAbstractGeometry{
    constructor(){
        super()
        this._rename("AriaCom/RTAbstractGeometry")
    }
    public injectRaySpreadingCriterion(): string {
        return ""
    }
}