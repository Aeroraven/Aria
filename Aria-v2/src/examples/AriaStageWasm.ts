import { AriaAlwRuntime } from "../core/runtime/AriaAlwRuntime";
import { AriaStage } from "./AriaStage";

export class AriaStageWasm extends AriaStage{
    constructor(){
        super("AriaStage/Wasm")
    }
    public async entry(): Promise<void> {
        let runtime = AriaAlwRuntime.instance
        await runtime.initRuntime()
        console.log(runtime.runtime)

    }
}

