import { AriaComponent } from "../../../core/AriaComponent";
import { AriaComExtPlaneGeometry } from "../../geometry/primary/AriaComExtPlaneGeometry";
import { AriaPhyVirtualGeometry } from "../base/AriaPhyVirtualGeometry";
import { AriaPhyParticleForceRegistry } from "../particle_force/AriaPhyParticleForceRegistry"
import { AriaPhyParticleBasicSpringGenerator } from "../particle_force/AriaPhyParticleBasicSpringGenerator"
import { AriaPhyParticle, AriaPhyParticleIntegrator } from "../particle/AriaPhyParticle";
import { AriaVec3 } from "../../../core/arithmetic/AriaVector";
import { AriaPhyParticleForceGenerator } from "../particle_force/AriaPhyParticleForceGenerator";

interface AriaPhySpringMassClothSpringPair {
    s:AriaPhyParticleBasicSpringGenerator
    r:AriaPhyParticle
}

export class AriaPhySpringMassCloth extends AriaComponent{
    private _clothEntity:AriaComExtPlaneGeometry
    private _springK:number = 0
    private _springRest:number = 0
    private _springRestO:number = 0
    private _particleStore:AriaPhyVirtualGeometry[] = []
    private _particle:AriaPhyParticle[] = []
    private _forceReg: AriaPhyParticleForceRegistry
    private _cx = 0
    private _cz = 0
    public _springs: AriaPhyParticleBasicSpringGenerator[][] = []
    private _springMass = 0
    private _integrator:AriaPhyParticleIntegrator

    private _enableScaler = true
    private _scaler = 1

    constructor(forceReg:AriaPhyParticleForceRegistry, clothMesh:AriaComExtPlaneGeometry,springHookeCoef:number,
        springRestLength:number,springMass:number,particleDamping:number,integrator:AriaPhyParticleIntegrator=AriaPhyParticleIntegrator.APP_INTEGRATOR_VERLET){
        super("AriaPhy/SpringMassCloth")
        this._clothEntity = clothMesh
        this._springK = springHookeCoef
        this._springRest = springRestLength
        this._springRestO = springRestLength
        this._cx = clothMesh.getDensityX()
        this._cz = clothMesh.getDensityZ()
        this._springMass = springMass
        this._integrator = integrator
        for(let i=0;i<this._cx;i++){
            for(let j=0;j<this._cz;j++){
                let pos = clothMesh.getLoc(i,j)
                let store = new AriaPhyVirtualGeometry()
                store.localPosition(pos[0]/this._springRest,pos[1]/this._springRest,pos[2]/this._springRest)
                this._particleStore.push(store)

                let p = new AriaPhyParticle()
                p.setIntegrator(this._integrator)
                p.position = AriaVec3.create(pos)
                p.mass = springMass
                p.damping = particleDamping
                this._particle.push(p)

                this._springs.push([])
            }
        }
        
        this._forceReg = forceReg
        this._buildSprings()
        this._springRest = 1
    }
    public getXAnchors(){
        return this._cx
    }
    public getZAnchors(){
        return this._cz
    }
    public getParticle(x:number,z:number){
        return this._particle[x*this._cz+z]
    }
    public addForce(x:number,z:number,f:AriaPhyParticleForceGenerator){
        this._forceReg.add(this.getParticle(x,z),f)
    }
    public _getIdx(x:number,z:number){
        return x*this._cz+z
    }
    public integrateParticles(delta:number){
        for(let i=0;i<this._cx;i++){
            for(let j=0;j<this._cz;j++){
                this.getParticle(i,j).integrate(delta)
            }
        }
        return
        //Deformation Constraints
        for(let i=0;i<this._cx-1;i++){
            for(let j=0;j<this._cz-1;j++){
                let idx = this._getIdx(i,j)
                for(let k=0;k<this._springs[idx].length;k++){
                    if(this._springs[idx][k].getOtherEnd().sub(this.getParticle(i,j).position).len()>this._springs[idx][k].getRestLength()*1.1){
                        let dirs = this.getParticle(i,j).position.sub(this._springs[idx][k].getOtherEnd())
                        let delta = 0.5*(dirs.len()-this._springs[idx][k].getRestLength()*1.1)
                        this._springs[idx][k].getOtherEnd().add(dirs.mul(delta))
                        this.getParticle(i,j).position.sub(dirs.mul(delta))
                    }
                }
            }
        }
    }
    public sync(){
        for(let i=0;i<this._cx;i++){
            for(let j=0;j<this._cz;j++){
                let pos = this.getParticle(i,j).position.toArray()
                this._clothEntity.setLoc(i,j,[pos[0]*this._springRest,pos[1]*this._springRest,pos[2]*this._springRest])
            }
        }
    }
    private _buildSprings(){
        const sqrt2 = Math.sqrt(2.0)
        //Structural & Shearing Constraints
        for(let i=0;i<this._cx-1;i++){
            for(let j=0;j<this._cz-1;j++){
                let pc = this.getParticle(i,j)
                let pr = this.getParticle(i+1,j)
                let pb = this.getParticle(i,j+1)
                let prb = this.getParticle(i+1,j+1)

                //PC
                this._springs[this._getIdx(i,j)].push(new AriaPhyParticleBasicSpringGenerator(pr,this._springRest,this._springK))
                this._springs[this._getIdx(i,j)].push(new AriaPhyParticleBasicSpringGenerator(pb,this._springRest,this._springK))
                this._springs[this._getIdx(i,j)].push(new AriaPhyParticleBasicSpringGenerator(prb,this._springRest*sqrt2,this._springK))

                //PR
                this._springs[this._getIdx(i+1,j)].push(new AriaPhyParticleBasicSpringGenerator(pb,this._springRest*sqrt2,this._springK))
                this._springs[this._getIdx(i+1,j)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest,this._springK))

                //PB
                this._springs[this._getIdx(i,j+1)].push(new AriaPhyParticleBasicSpringGenerator(pr,this._springRest*sqrt2,this._springK))
                this._springs[this._getIdx(i,j+1)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest,this._springK))

                //PRB
                this._springs[this._getIdx(i+1,j+1)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest*sqrt2,this._springK))
            }
        }
        for(let i=0;i<this._cx-1;i++){
            let pc = this.getParticle(i,this._cz-1)
            let pr = this.getParticle(i+1,this._cz-1)
            this._springs[this._getIdx(i,this._cz-1)].push(new AriaPhyParticleBasicSpringGenerator(pr,this._springRest,this._springK))
            this._springs[this._getIdx(i+1,this._cz-1)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest,this._springK))
        }
        for(let i=0;i<this._cz-1;i++){
            let pc = this.getParticle(this._cx-1,i)
            let pr = this.getParticle(this._cx-1,i+1)
            this._springs[this._getIdx(this._cx-1,i)].push(new AriaPhyParticleBasicSpringGenerator(pr,this._springRest,this._springK))
            this._springs[this._getIdx(this._cx-1,i+1)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest,this._springK))
        }

        //Bending Constraints
        const bendK = this._springK * 0.4
        for(let i=0;i<this._cx-2;i++){
            for(let j=0;j<this._cz-2;j++){
                let pc = this.getParticle(i,j)
                let pr2 = this.getParticle(i+2,j)
                let pb2 = this.getParticle(i,j+2)

                //PC
                this._springs[this._getIdx(i,j)].push(new AriaPhyParticleBasicSpringGenerator(pr2,this._springRest*2,bendK))
                this._springs[this._getIdx(i,j)].push(new AriaPhyParticleBasicSpringGenerator(pb2,this._springRest*2,bendK))

                //PR2
                this._springs[this._getIdx(i+2,j)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest*2,bendK))

                //PB2
                this._springs[this._getIdx(i,j+2)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest*2,bendK))
            }
        }
        for(let i=0;i<this._cx-2;i++){
            let pc = this.getParticle(i,this._cz-1)
            let pr = this.getParticle(i+2,this._cz-1)
            this._springs[this._getIdx(i,this._cz-1)].push(new AriaPhyParticleBasicSpringGenerator(pr,this._springRest*2,bendK))
            this._springs[this._getIdx(i+2,this._cz-1)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest*2,bendK))
        }
        for(let i=0;i<this._cz-2;i++){
            let pc = this.getParticle(this._cx-1,i)
            let pr = this.getParticle(this._cx-1,i+2)
            this._springs[this._getIdx(this._cx-1,i)].push(new AriaPhyParticleBasicSpringGenerator(pr,this._springRest*2,bendK))
            this._springs[this._getIdx(this._cx-1,i+2)].push(new AriaPhyParticleBasicSpringGenerator(pc,this._springRest*2,bendK))
        }


        //Inject Spring
        for(let i=0;i<this._cx;i++){
            for(let j=0;j<this._cz;j++){
                for(let k=0;k<this._springs[this._getIdx(i,j)].length;k++){
                    this._forceReg.add(this._particle[this._getIdx(i,j)],this._springs[this._getIdx(i,j)][k])
                }
                
            }
        }
    }
}