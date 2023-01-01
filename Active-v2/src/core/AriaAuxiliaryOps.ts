import { AriaObjArray, AriaObjArrayIterCallable } from "./base/AriaBaseDefs";
import { AriaObject } from "./base/AriaObject";

export class AriaAuxiliaryOps extends AriaObject{
    private constructor(){
        super("AriaAuxiliaryOps")
    }
    public static iterateObjArray<T=any>(x:AriaObjArray<T>,callable:AriaObjArrayIterCallable<T>){
        if (x instanceof Array){
            x.forEach((p)=>{
                callable(p)
            })
        }else{
            callable(x)
        }
    }
}