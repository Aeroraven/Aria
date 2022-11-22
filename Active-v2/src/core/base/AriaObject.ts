import { AriaLogger, AriaLogLevel } from "./AriaLogger"

export class AriaObject{
    protected name:string = ""
    private doneMixin = false
    constructor(name:string){
        this.name = name
    }

    protected _logInfo(info:string){
        AriaLogger.getInstance().log(info,AriaLogLevel.INFO, this.name)
    }
    protected _logWarn(info:string){
        AriaLogger.getInstance().log(info,AriaLogLevel.WARN, this.name)
    }
    protected _logError(info:string){
        AriaLogger.getInstance().log(info,AriaLogLevel.ERROR, this.name)
    }
    protected _mixin(derivedCtor: any, baseCtors: any[]){{
        if(this.doneMixin==false){
            if(this instanceof derivedCtor){
                this.__applyMixins(derivedCtor, baseCtors)
                this.doneMixin = true
            }else{
                this._logError("Mixin cannot be performed on unrelated classes")
            }
        }else{
            this._logWarn("Further mixins are temporarily not supported. Operation is ignored.")
        }
    }}
    private __applyMixins(derivedCtor: any, baseCtors: any[]) {
        baseCtors.forEach(baseCtor => {
            Object.getOwnPropertyNames(baseCtor.prototype).forEach(name => {
                derivedCtor.prototype[name] = baseCtor.prototype[name];
            })
        });
    }
}