import { AriaVec3, AriaVec3C } from "../../../core/arithmetic/AriaVector";
import { AriaCoreUtil } from "../../../core/base/AriaCoreUtils";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";
import { AriaPhyParticleForceGenerator } from "./AriaPhyParticleForceGenerator";

export class AriaPhyParticleBasicSpringGenerator extends AriaPhyParticleForceGenerator{
    private _otherEnd:AriaVec3|AriaPhyParticle = AriaVec3.create()
    private _restLength = 0
    private _stringConstant = 1
    private _otherEndCaller = ()=>{return AriaVec3.create()}

    constructor(end:AriaVec3C|AriaPhyParticle,restLength:number,stringConstant:number){
        super()
        this._rename("AriaPhy/ParticleBasicSpringGenerator")
        if(end instanceof AriaPhyParticle){
            this._otherEnd = end
            this._otherEndCaller = ()=>{
                return end.position
            }
        }else{
            this._otherEnd = AriaVec3.create()
            this._otherEnd.fromArray(end)

            this._otherEndCaller = ()=>{
                return <AriaVec3>this._otherEnd
            }
        }
        
        this._restLength = restLength
        this._stringConstant = stringConstant
    }

    public updateForce(particle: AriaPhyParticle, duration: number): void {
        let springLengthVec = particle.position.sub(this._otherEndCaller())
        let springLength = springLengthVec.len()
        springLengthVec.normalize_().mul_(-1)
        springLengthVec.mul_(AriaCoreUtil.Math.clampEps((springLength-this._restLength))*this._stringConstant)
        particle.addForce(springLengthVec)
    }
}