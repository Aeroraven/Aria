import { AriaComponent } from "../../../core/AriaComponent";
import { IAriaNoiseGenerator } from "../interfaces/IAriaNoiseGenerator";

export class AriaComFractalNoise extends AriaComponent implements IAriaNoiseGenerator{
    its:number
    gen:IAriaNoiseGenerator
    constructor(baseGenerator:IAriaNoiseGenerator,iterations:number){
        super("AriaCom/FractalNoise")
        this.its = iterations
        this.gen = baseGenerator
    }
    public noise(x: number, y: number, z: number): number {
        let val=0,ampl=1,freq=1,csamp=0
        for(let i=0;i<this.its;i++){
            val+=this.gen.noise(freq*x,freq*y,freq*z)*ampl
            csamp+=ampl
            freq*=2
            ampl/=2
        }
        return val/csamp
    }
    
}