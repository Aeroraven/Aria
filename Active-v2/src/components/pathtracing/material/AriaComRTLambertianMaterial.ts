import { AriaComRTAbstractMaterial } from "./AriaComRTAbstractMaterial";

export class AriaComRTLambertianMaterial extends AriaComRTAbstractMaterial {
    constructor() {
        super();
        this._rename("AriaCom/RTAbstractMaterial");
    }
    public injectDefinition(): string {
        return "const int RTM_Lambertian=1;"
    }
    public injectRayCollisionCriterion(): string {
        return ""
    }
    public injectRaySpreadingCriterion(): string {
        return ""
    }
    public getIdentifier(): string {
        return "RTM_Lambertian";
    }
}