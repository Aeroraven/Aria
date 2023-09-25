import { AriaComRTFragShaderInjector } from "../AriaComRTFragShaderInjector";
import { IAriaComRTShaderIdentifier } from "../base/IAriaComRTShaderIdentifier";
import { IAriaComRTAbstractMaterial } from "./IAriaComRTAbstractMaterial";

export abstract class AriaComRTAbstractMaterial extends AriaComRTFragShaderInjector implements IAriaComRTAbstractMaterial,IAriaComRTShaderIdentifier{
    constructor(){
        super()
        this._rename("AriaCom/RTAbstractMaterial")
    }
    getIdentifier(): string {
        throw new Error("Method not implemented.");
    }
}