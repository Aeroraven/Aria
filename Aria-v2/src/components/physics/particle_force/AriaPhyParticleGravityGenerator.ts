import { AriaVec3 } from "../../../core/arithmetic/AriaVector";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";
import { AriaPhyParticleForceGenerator } from "./AriaPhyParticleForceGenerator";

export class AriaPhyParticleGravityGenerator extends AriaPhyParticleForceGenerator{
    private _gravity:AriaVec3
    constructor(gravity_accl:AriaVec3|number[]){
        super()
        this._rename("AriaPhy/ParticleGravityGenerator")
        this._gravity = AriaVec3.create()
        this._gravity.fromArray(gravity_accl)
    }
    public updateForce(particle: AriaPhyParticle, duration: number): void {
        particle.addForce(this._gravity.mul(particle.mass))
    }
}