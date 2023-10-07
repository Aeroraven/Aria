import { AriaComponent } from "../../../core/AriaComponent";
import { IAriaDynamicGeometry } from "../../base/interface/IAriaDynamicGeometry";
import { AriaComStopwatch } from "../../timer/AriaComStopwatch";
import { AriaPhyParticle } from "./AriaPhyParticle";

export class AriaPhyParticleSyncer extends AriaComponent{
    protected _particle:AriaPhyParticle = new AriaPhyParticle()
    protected _geometry:IAriaDynamicGeometry
    protected _timer:AriaComStopwatch = new AriaComStopwatch()

    constructor(g:IAriaDynamicGeometry){
        super("AriaPhy/PartcileSyncer")
        this._geometry = g
        this._timer.setScaler(0.001)
    }

    public get particle(){
        return this._particle
    }

    public update(){
        let interval = this._timer.recordAndGetDuration()
        this._particle.integrate(interval)
        this._geometry.localPosition(this.particle.position.at(0),this.particle.position.at(1),this.particle.position.at(2))
    }
}