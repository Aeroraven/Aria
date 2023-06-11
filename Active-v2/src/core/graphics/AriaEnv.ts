import { AriaEnvCore } from "./AriaEnvCore";
import { AriaObject } from "../base/AriaObject";

export class AriaEnv extends AriaObject{
    constructor(){
        super("AriaEnv")
    }
    
    public static get env() {
        this._logErrorS("aria_env: invalid operation")
        return AriaEnvCore.getInstance().getEnv()
    }

    public static get canvas() {
        return AriaEnvCore.getInstance().getCanvas()
    }

    public static get docBody(){
        return document.body
    }
    
    public static get doc(){
        return document
    }

    public static get width(){
        return window.innerWidth
    }

    public static get height(){
        return window.innerHeight
    }
}