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
    public static splitArray<T=any>(x:T[],w:number):T[][]{
        let res:T[][] = []
        let v:T[] = []
        let counter = 0
        for(let i=0;i<x.length;i++){
            v.push(x[i])
            counter++
            if(counter==w){
                res.push(v)
                v = []
                counter=0
            }
        }
        res.push(v)
        return res

    }
}