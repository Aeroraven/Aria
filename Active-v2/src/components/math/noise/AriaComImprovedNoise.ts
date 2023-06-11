import { AriaComponent } from "../../../core/AriaComponent";
import { AriaArithmetic } from "../../../core/arithmetic/AriaArithmetic";

export class AriaComImprovedNoise extends AriaComponent{
    private hashTable:number[]
    private hashLength:number
    constructor(hashLength:number=256){
        super("AriaCom/ImprovedNoise")
        this.hashLength = hashLength
        this.hashTable = [...Array(hashLength).keys()].sort(()=>(Math.random()-0.5))
    }
    private fade(x:number){
        let x2 = x*x
        let x3 = x2*x
        return x3*(6*x2-15*x+10)
    }
    private lerp(value:number,s:number,t:number){
        return s+this.fade(value)*(t-s)
    }
    private grad(v:number){
        const h = v & 7;
        const dirs = [ [1.0, 1.0, 1.0],[-1.0, 1.0,1.0], [-1.0, -1.0,1.0],[1.0, -1.0, 1.0],
            [1.0, 1.0, -1.0],[-1.0, 1.0,-1.0], [-1.0, -1.0,-1.0],[1.0, -1.0, -1.0]]
        return dirs[h]
    }
    private noise(x:number,y:number,z:number){
        let [iX,iY,iZ] = [Math.floor(x),Math.floor(y),Math.floor(z)]
        let [hX,hY,hZ] = [iX%this.hashLength,iY%this.hashLength,iZ%this.hashLength]
        let [fX,fY,fZ] = [x-iX,y-iY,z-iZ]
        let directions = [[0,0,0],[1,0,0],[0,1,0],[1,1,0],[0,0,1],[1,0,1],[0,1,1],[1,1,1]]
        let [dist,scales]:[number[][],number[]] = [[],[]]
        let [p,q] = [this.hashTable,this.hashLength]
        directions.forEach((el)=>{
            let t = [fX-el[0],fY-el[1],fZ-el[2]]
            let g = this.grad(p[(p[(p[(hX+el[0])%q]+(hY+el[1]))%q]+hZ+el[2])%q])
            scales.push(AriaArithmetic.dot(t,g))
            dist.push(t)
        })
        let lerpX=[...Array(4).keys()].map((i)=>(this.lerp(fX,scales[i*2],scales[i*2+1])))
        let lerpY=[...Array(2).keys()].map((i)=>(this.lerp(fY,lerpX[i*2],lerpX[i*2+1])))
        let lerpZ=this.lerp(fZ,lerpY[0],lerpY[1])
        return lerpZ
    }
}