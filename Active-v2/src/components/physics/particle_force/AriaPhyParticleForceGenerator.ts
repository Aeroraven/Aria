import { AriaComponent } from "../../../core/AriaComponent";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";

export class AriaPhyParticleForceGenerator extends AriaComponent{
    constructor(){
        super("AriaPhy/ParticleForceGenerator")
    }
    public updateForce(particle:AriaPhyParticle,duration:number){
        this._logError("aria.physics.particle_force_generator: updateForce: not implemented")
    }
}