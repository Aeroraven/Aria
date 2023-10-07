import { AriaComponent } from "../../../core/AriaComponent";
import { AriaVec3, AriaVec3C } from "../../../core/arithmetic/AriaVector";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";

export class AriaPhyParticleContact extends AriaComponent{
    protected _particles:AriaPhyParticle[] = []
    protected _restitution:number = 1.0
    protected _contactNormal:AriaVec3 = AriaVec3.create()
    protected _penetration:number = 0.0

    constructor(pa:AriaPhyParticle,pb:AriaPhyParticle|undefined,contactNormal:AriaVec3C,rest:number,penetration:number){
        super("AriaPhy/ParticleContact")
        this._particles.push(pa)
        if(pb)this._particles.push(pb)
        this._contactNormal.fromArray(contactNormal)
        this._restitution = rest
        this._penetration = penetration
    }

    protected get pa():AriaPhyParticle{
        return this._particles[0]
    }

    protected get pb():AriaPhyParticle|null{
        if(this._particles.length>1){
            return this._particles[1]
        }
        return null
    }

    public resolve(duration:number){
        this.resolveVelocity(duration)
        this.resolveInterpenetration(duration)
    }
    public resolveInterpenetration(duration:number){
        if(this._penetration<=0){
            return
        }
        let totalInvMass = this.pa.inverseMass
        if(this.pb){
            totalInvMass += this.pb.inverseMass
        }
        let movePerInvMass = this._contactNormal.mul(-this._penetration/totalInvMass)
        //Apply Penetration Resolution
        this.pa.position.addScaled_(movePerInvMass,this.pa.inverseMass)
        if(this.pb){
            this.pb.position.addScaled_(movePerInvMass,-this.pb.inverseMass)
        }
    }
    public resolveVelocity(duration:number){
        let sepVelocity = this.calcSepVelocity()
        if(sepVelocity>0){
            return
        }
        //Acceleration Interference
        let relativeAccl = this.pa.totalAcceleration
        if(this.pb){
            relativeAccl.sub_(this.pb.totalAcceleration)
        }
        let accCausedSepVelocity = relativeAccl.dot(this._contactNormal)*duration

        //After Collision
        let afterSepVelocity = -sepVelocity * this._restitution
        if(accCausedSepVelocity<0){
            afterSepVelocity += accCausedSepVelocity * this._restitution
            if(afterSepVelocity<0){
                afterSepVelocity = 0
            }
        }

        let deltaSepVelocity = afterSepVelocity - sepVelocity
        let totalInvMass = this.pa.inverseMass
        if(this.pb){
            totalInvMass += this.pb.inverseMass
        }
        let impluse = deltaSepVelocity / totalInvMass
        let implPerInvMass = this._contactNormal.mul_(impluse)
        //Apply Impluse
        this.pa.velocity.addScaled_(implPerInvMass,this.pa.inverseMass)
        if(this.pb){
            this.pb.velocity.addScaled_(implPerInvMass,-this.pb.inverseMass)
        }

    }
    private calcSepVelocity():number{
        let relVelocity = AriaVec3.create()
        relVelocity.fromArray(this.pa.velocity)
        if(this.pb){
            relVelocity.sub_(this.pb.velocity)
        }
        return relVelocity.dot(this._contactNormal)
    }
}