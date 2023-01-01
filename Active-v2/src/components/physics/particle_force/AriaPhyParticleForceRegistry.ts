import { AriaComponent } from "../../../core/AriaComponent";
import { AriaComStopwatch } from "../../timer/AriaComStopwatch";
import { AriaPhyParticle } from "../particle/AriaPhyParticle";
import { AriaPhyParticleForceGenerator } from "./AriaPhyParticleForceGenerator";

interface IAriaPhyForceRegistryEntry{
    particle:AriaPhyParticle
    generator:AriaPhyParticleForceGenerator
}

export class AriaPhyParticleForceRegistry extends AriaComponent{
    private _list:IAriaPhyForceRegistryEntry[] = []
    private _timer:AriaComStopwatch = new AriaComStopwatch()
    constructor(){
        super("AriaPhy/ParticleForceRegistry")
        this._timer.recordAndGetDuration()
        this._timer.setScaler(0.001)
    }

    public add(particle:AriaPhyParticle,force:AriaPhyParticleForceGenerator){
        this._list.push({
            particle:particle,
            generator:force
        })
    }
    public clear(){
        this._list = []
    }
    public remove(particle:AriaPhyParticle,force:AriaPhyParticleForceGenerator){
        for(let i=0;i<this._list.length;i++){
            if(particle.id == this._list[i].particle.id && force.id == this._list[i].generator.id){
                this._list.splice(i,1)
                break
            }
        }
    }
    public update(duration:number){
        for(let i=0;i<this._list.length;i++){
            this._list[i].generator.updateForce(this._list[i].particle,duration)
        }
    }
    public autoUpdate(){
        let duration = this._timer.recordAndGetDuration()
        this.update(duration)
    }
}