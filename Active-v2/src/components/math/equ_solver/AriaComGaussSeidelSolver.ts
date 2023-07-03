import { AriaComponent } from "../../../core/AriaComponent";
import { AriaMatrix } from "../../../core/arithmetic/AriaMatrix";
import { AriaVector } from "../../../core/arithmetic/AriaVector";

export class AriaComGaussSeidelSolver extends AriaComponent{
    constructor(){
        super("AriaCom/GaussSeidelSolver")
    }
    public solve(a:AriaMatrix,b:AriaVector,iters:number,sorCoef:number=1.0){
        //Solves Ax=b
        let r = a.getRows()
        let x = [AriaVector.create(r,1),AriaVector.create(r,1)]
        let o = 0
        while(iters--){
            o ^= 1
            for(let i=0;i<r;i++){
                let s=0
                for(let j=0;j<r;j++){
                    if(i!=j){
                        s+=a.at(i,j)*x[o].at(j)
                    }
                }
                x[o^1].setVal(i,sorCoef*(b.at(i)-s)/a.at(i,i)+(1-sorCoef)*(x[o].at(i)))
            }
            //this._logInfo(x[o^1].toArray()+"_")
        }
        //this._logError("X")
        return x[o^1]
    }
}