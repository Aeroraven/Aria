import { AriaComponent } from "../../../core/AriaComponent";
import { AriaVec3 } from "../../../core/arithmetic/AriaVector";

export class AriaPhyParticle extends AriaComponent{
    protected _position:AriaVec3 = AriaVec3.create()
    protected _velocity:AriaVec3 = AriaVec3.create()
    protected _acceleration:AriaVec3 = AriaVec3.create()
    protected _damping:number = 0.995
    protected _mass:number = 1
    protected _forceAccum:AriaVec3 = AriaVec3.create()

    constructor(){
        super("AriaPhy/Particle")
    }

    public integrate(duration:number){
        this.position.addScaled_(this.velocity,duration)
        let accu = AriaVec3.create()
        accu.fromArray(this.acceleration)
        accu.addScaled_(this._forceAccum,this.inverseMass)
        this.velocity.addScaled_(accu,duration)
        this.velocity.mul_(Math.pow(this.damping,duration))
        this.clearForceAccum()
    }

    public addForce(force:AriaVec3){
        this._forceAccum.add_(force)
    }
    public clearForceAccum(){
        this._forceAccum.fromArray([0,0,0])
    }

    public get position(){
        return this._position
    }
    public set position(x:AriaVec3){
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