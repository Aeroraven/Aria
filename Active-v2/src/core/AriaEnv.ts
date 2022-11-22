import { AriaEnvCore } from "./AriaEnvCore";
import { AriaObject } from "./base/AriaObject";

export class AriaEnv extends AriaObject{
    constructor(){
        super("AriaEnv")
    }
    
    public static get env() {
        return AriaEnvCore.getInstance().getEnv()
    }

    public static get canvas() {
        return AriaEnvCore.getInstance().getCanvas()
    }
    
}