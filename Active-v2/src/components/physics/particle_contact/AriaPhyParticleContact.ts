import { AriaComponent } from "../../../core/AriaComponent";
import { AriaVec3, AriaVec3C } from "../../../core/arithmetic/AriaVector";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";

export class AriaPhyParticleContact extends AriaComponent{
    protected _particles:AriaPhyParticle[] = []
    protected _restitution:number = 1.0
    protected _contactNormal:AriaVec3 = AriaVec3.create()

    constructor(pa:AriaPhyParticle,pb:AriaPhyParticle|undefined,contactNormal:AriaVec3C,rest:number){
        super("AriaPhy/ParticleContact")
        this._particles.push(pa)
        if(pb)this._particles.push(pb)
        this._contactNormal.fromArray(contactNormal)
        this._restitution = rest
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
        return this.resolveVelocity(duration)
    }
    public resolveVelocity(duration:number){
        let sepVelocity = this.calcSepVelocity()
        
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