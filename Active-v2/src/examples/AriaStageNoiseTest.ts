import { AriaComImprovedNoise } from "../components/math/noise/AriaComImprovedNoise";
import { AriaStage } from "./AriaStage";

export class AriaStageNoiseTest extends AriaStage{
    constructor(){
        super("AriaStage/RigidBody")
    }
    public async entry(): Promise<void> {
        let w = new AriaComImprovedNoise();
        (<any>window).pl = w
    }
}