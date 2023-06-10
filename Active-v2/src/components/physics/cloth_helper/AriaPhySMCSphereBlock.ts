import { AriaComponent } from "../../../core/AriaComponent";
import { AriaArithmetic } from "../../../core/arithmetic/AriaArithmetic";
import { AriaCompGeo } from "../../../core/arithmetic/AriaCompGeo";
import { AriaVec3C, AriaVec3 } from "../../../core/arithmetic/AriaVector";
import { AriaPhySpringMassCloth } from "../cloth/AriaPhySpringMassCloth";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";
import { AriaPhyParticleForceGenerator } from "../particle_force/AriaPhyParticleForceGenerator";

export class AriaPhySMCSphereBlock extends AriaComponent{
    center: AriaVec3
    raidus: number
    accuMode:boolean = false
    constructor(center:AriaVec3C, raidus:number){
        super("AriaPhy/AriaPhySMCSphereBlock")
        this.center = (center instanceof AriaVec3) ? center : AriaVec3.create(center)
        this.raidus = raidus
    }
    public updateSingle(particle: AriaPhyParticle): void {
        let dis = particle.position.sub(this.center)
        if(dis.len()-this.raidus<-1e-6){
            let dir = dis.normalize().mul(this.raidus).add(this.center)
            if (this.accuMode == false){
                particle.position.fromArray(dir)
            }else{
                let last = particle.lastPosition
                let sols = AriaCompGeo.segmentIntersectsSphere(this.center.toArray(),this.raidus,last.toArray(),dir.toArray())
                if(sols.length==0){
                    this._logError("Invalid Position")
                }else{
                    let pt0 = AriaVec3.create(sols[0])
                    let pt1 = AriaVec3.create(sols[1])
                    let choice = (pt0.sub(last).len()<pt1.sub(last).len())?pt0:pt1
                    particle.position.fromArray(choice)
                }
            }
            
            //return 
            
        }
    }
    public updateAll(smc:AriaPhySpringMassCloth){
        let xancs = smc.getXAnchors()
        let zancs = smc.getZAnchors()
        for(let i=0;i<xancs;i++){
            for(let j=0;j<zancs;j++){
                this.updateSingle(smc.getParticle(i,j))
            }
        }
    }
}