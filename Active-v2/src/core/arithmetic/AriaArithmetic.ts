import { AriaObject } from "../base/AriaObject";

export class AriaArithmetic extends AriaObject{
    constructor(){
        super("Aria/Arithmetic")
    }
    public static cross2(x1:number,y1:number,x2:number,y2:number){
        return x1*y2-x2*y1
    }
    public static cross(a:number[],b:number[]):number[]{
        let dx = a[1]*b[2]-a[2]*b[1]
        let dy = a[2]*b[0]-a[0]*b[2]
        let dz = a[0]*b[1]-a[1]*b[0]
        return [dx,dy,dz]
    }
    public static average(a:number[][],dim:number):number[]{
        let x = new Array<number>(dim)
        for(let i=0;i<dim;i++){
            x[i]=0
            for(let j=0;j<a.length;j++){
                x[i]+=a[j][i]
            }
            x[i]/=a.length
        }
        return x
    }
}