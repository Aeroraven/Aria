import { AriaComponent } from "./AriaComponent";
import { AriaObject } from "./base/AriaObject";

export class AriaComManager extends AriaObject{
    public static inst:AriaComManager|null = null
    private componentList:AriaComponent[] = []
    public static getInst(){
        if(this.inst == null){
            this.inst = new AriaComManager()
        }
        return <AriaComManager>this.inst
    }
    constructor(){
        super("AriaComManager")
    }
    public registerComponent(c:AriaComponent){
        this.componentList.push(c)
        return this.componentList.length-1
    }
}