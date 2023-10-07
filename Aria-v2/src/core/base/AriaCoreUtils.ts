import { AriaObject } from "./AriaObject";
import { AriaBaseConstant } from "./AriaBaseConstant"

class AriaCoreUtilMath{
    static isEqual(a:number,b:number){
        return (Math.abs(a-b)<AriaBaseConstant.NUM_EPS)
    }
    static clampEps(a:number){
        return (Math.abs(a)<AriaBaseConstant.NUM_EPS)?0:a
    }
}

export class AriaCoreUtil extends AriaObject{
    public static readonly Math = AriaCoreUtilMath

    constructor(){
        super("AriaCoreUtil")
    }

}