import { AriaComRTAbstractMaterial } from "./AriaComRTAbstractMaterial";
import defCode from "../../assets/pathtracer/shaders/material_refractive/defs.glsl"
import tracerCode from "../../assets/pathtracer/shaders/material_refractive/tracer.glsl"
export class AriaComRTRefractiveMaterial extends AriaComRTAbstractMaterial {
    constructor() {
        super();
        this._rename("AriaCom/RefractiveMaterial");
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
        return "RTM_Refractive";
    }
}