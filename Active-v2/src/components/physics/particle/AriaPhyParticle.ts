import { AriaComponent } from "../../../core/AriaComponent";
import { AriaVec3, AriaVec3C } from "../../../core/arithmetic/AriaVector";

export enum AriaPhyParticleIntegrator{
    APP_INTEGRATOR_EUCLID = "eculid",
    APP_INTEGRATOR_VERLET = "verlet"
}

export class AriaPhyParticle extends AriaComponent{
    protected _lastPosition:AriaVec3 = AriaVec3.create()
    protected _position:AriaVec3 = AriaVec3.create()
    protected _velocity:AriaVec3 = AriaVec3.create()
    protected _acceleration:AriaVec3 = AriaVec3.create()
    protected _damping:number = 0.995
    protected _mass:number = 1
    public _forceAccum:AriaVec3 = AriaVec3.create()
    private _integrator:string = AriaPhyParticleIntegrator.APP_INTEGRATOR_VERLET

    constructor(){
        super("AriaPhy/Particle")
    }

    public integrate(duration:number){
        if(this._integrator==AriaPhyParticleIntegrator.APP_INTEGRATOR_EUCLID){
            this._lastPosition.fromArray(this.position)
            this.position.addScaled_(this.velocity,duration)

            let accu = AriaVec3.create()
            accu.fromArray(this.acceleration)
            accu.addScaled_(this._forceAccum,this.inverseMass)

            
            this.position.addScaled_(accu,duration*duration)

            this.velocity.addScaled_(accu,duration)
            this.velocity.mul_(Math.pow((1-this.damping),duration))
            this.clearForceAccum()
        }else if(this._integrator==AriaPhyParticleIntegrator.APP_INTEGRATOR_VERLET){
            let lastPos = AriaVec3.create()
            lastPos.fromArray(this._lastPosition)
            this._lastPosition.fromArray(this.position)

            let accu = AriaVec3.create()
            accu.fromArray(this.acceleration)
            accu.addScaled_(this._forceAccum,this.inverseMass)

            this.position.addScaled_(this._lastPosition.sub(lastPos),(1-this.damping))
            this.position.addScaled_(accu,duration*duration)

            this.velocity.fromArray(this.position.sub(this._lastPosition).div(duration))
            this.clearForceAccum()
        }else{
            this._logError("aria.phy.particle: integrator not supported yet")
        }
        
    }

    public addForce(force:AriaVec3){
        this._forceAccum.add_(force)
    }
    public clearForceAccum(){
        this._forceAccum.fromArray([0,0,0])
    }
    public setIntegrator(x:AriaPhyParticleIntegrator){
        this._integrator = x
    }
    public get lastPosition(){
        return this._lastPosition
    }
    public get position(){
        return this._position
    }
    public set position(x:AriaVec3){
        this._lastPosition.fromArray(x)
        this._position.fromArray(x)
    }

    public set positionAdjusted(x:AriaVec3){
        this._position.fromArray(x)
    }

    public get velocity(){
        return this._velocity
    }
    public set velocity(x:AriaVec3){
        this._velocity.fromArray(x)
    }

    public get acceleration(){
        return this._acceleration
    }
    public get totalAcceleration(){
        let ret = AriaVec3.create()
        ret.fromArray(this.acceleration)
        ret.addScaled_(this._forceAccum,this.inverseMass)
        return ret

    }
    public set acceleration(x:AriaVec3){
        this._acceleration.fromArray(x)
    }

    public get mass(){
        return this._mass
    }
    public set mass(x:number){
        this._mass = x
    }
    
    public get damping(){
        return this._damping
    }
    public set damping(x:number){
        this._damping = x
    }

    public get inverseMass(){
        return 1/this._mass
    }
    
}