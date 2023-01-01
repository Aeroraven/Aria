import { AriaVec3, AriaVec3C } from "../../../core/arithmetic/AriaVector";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";
import { AriaPhyParticleForceGenerator } from "./AriaPhyParticleForceGenerator";

export class AriaPhyParticleBasicSpringGenerator extends AriaPhyParticleForceGenerator{
    private _otherEnd:AriaVec3 = AriaVec3.create()
    private _restLength = 0
    private _stringConstant = 1

    constructor(end:AriaVec3C,restLength:number,stringConstant:number){
        super()
        this._rename("AriaPhy/ParticleBasicSpringGenerator")
        this._otherEnd.fromArray(end)
        this._restLength = restLength
        this._stringConstant = stringConstant
    }

    public updateForce(particle: AriaPhyParticle, duration: number): void {
        let stringLengthVec = particle.position.sub(this._otherEnd)
        let stringLength = stringLengthVec.len()
        
        let forceDir = stringLengthVec.normalize_().mul_(-1)
        let magnitude = Math.abs(stringLength-this._restLength)*this._stringConstant
        forceDir.mul_(magnitude)
        particle.addForce(forceDir)
    }
}