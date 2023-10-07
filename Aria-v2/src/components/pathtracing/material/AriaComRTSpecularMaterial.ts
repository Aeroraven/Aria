import { AriaComRTAbstractMaterial } from "./AriaComRTAbstractMaterial";
import defCode from "../../assets/pathtracer/shaders/material_specular/defs.glsl"
import tracerCode from "../../assets/pathtracer/shaders//material_specular/tracer.glsl"
export class AriaComRTSpecularMaterial extends AriaComRTAbstractMaterial {
    constructor() {
        super();
        this._rename("AriaCom/SpecularMaterial");
    }
    public injectDefinition(): string {
        return defCode
    }
    public injectRayCollisionCriterion(): string {
        return ""
    }
    public injectRaySpreadingCriterion(): string {
        return tracerCode
    }
    public getIdentifier(): string {
        return "RTM_Specular";
    }
}