import { AriaComRTAbstractMaterial } from "./AriaComRTAbstractMaterial";
import defCode from "../../assets/pathtracer/shaders/material_lambertian/defs.glsl"
import tracerCode from "../../assets/pathtracer/shaders/material_lambertian/tracer.glsl"
export class AriaComRTLambertianMaterial extends AriaComRTAbstractMaterial {
    constructor() {
        super();
        this._rename("AriaCom/RTAbstractMaterial");
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
        return "RTM_Lambertian";
    }
}