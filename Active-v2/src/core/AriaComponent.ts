import { AriaComManager } from "./AriaComManager";
import { AriaLogger, AriaLogLevel } from "./base/AriaLogger";
import { AriaObject } from "./base/AriaObject";

export class AriaComponent extends AriaObject{
    private aId:number = -1

    constructor(name:string){
        super(name)
        this.aId = AriaComManager.getInst().registerComponent(this)
    }
 
    public get id():number{
        if(this.aId<0){
            this._logError("Component is not registered")
        }
        return this.aId
    }

    protected override _logInfo(info:string){
        AriaLogger.getInstance().log(info,AriaLogLevel.INFO, this.name+"[#"+this.id+"]")
    }
    protected override _logWarn(info:string){
        AriaLogger.getInstance().log(info,AriaLogLevel.WARN, this.name+"[#"+this.id+"]")
    }
    protected override _logError(info:string){
        AriaLogger.getInstance().log(info,AriaLogLevel.ERROR, this.name+"[#"+this.id+"]")
    }
    protected _rename(name:string){
        this.name = name
    }
}