import { AriaComponent } from "../../../core/AriaComponent";
import { AriaArithmetic } from "../../../core/arithmetic/AriaArithmetic";
import { IAriaNoiseGenerator } from "../interfaces/IAriaNoiseGenerator";

export class AriaComImprovedNoise extends AriaComponent implements IAriaNoiseGenerator{
    private hashTable:number[]
    private hashLength:number
    private hashMap:Map<number,number[]> = new Map()
    private readonly directions = [[0,0,0],[1,0,0],[0,1,0],[1,1,0],[0,0,1],[1,0,1],[0,1,1],[1,1,1]]
    private readonly gradDirs = [ [1.0, 1.0, 1.0],[-1.0, 1.0,1.0], [-1.0, -1.0,1.0],[1.0, -1.0, 1.0],
        [1.0, 1.0, -1.0],[-1.0, 1.0,-1.0], [-1.0, -1.0,-1.0],[1.0, -1.0, -1.0]]
    private readonly perlinTable = [151,160,137,91,90,15,
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180]

    constructor(hashLength:number=256,usePredefined=true){
        super("AriaCom/ImprovedNoise")
        if(usePredefined){
            if(hashLength!=256){
                this._logError("aria_com.improved_noise: hash length != 256")
            }
            this.hashLength = hashLength
            this.hashTable = this.perlinTable
        }else{
            this.hashLength = hashLength
            this.hashTable = [...Array(hashLength).keys()].sort(()=>(Math.random()-0.5))
        }
    }
    private fade(x:number){
        let x2 = x*x
        let x3 = x2*x
        return x3*(6*x2-15*x+10)
    }
    private lerp(value:number,s:number,t:number){
        return s+this.fade(value)*(t-s)
    }
    private grad(ix:number,iy:number,iz:number){
        let ind = ix+iy+iz+ix*iy+iy*iz+iz*ix
        if(this.hashMap.has(ind)==false){
            let pa = Math.random()*(Math.PI*2)
            let pb = Math.random()*(Math.PI*2)
            this.hashMap.set(ind,[Math.cos(pa)*Math.cos(pb),Math.cos(pa)*Math.sin(pb),Math.sin(pa)])
        }
        return this.hashMap.get(ind)!
    }
    public noise(x:number,y:number,z:number){
        let [iX,iY,iZ] = [Math.floor(x),Math.floor(y),Math.floor(z)]
        let [hX,hY,hZ] = [iX%this.hashLength,iY%this.hashLength,iZ%this.hashLength]
        let [fX,fY,fZ] = [x-iX,y-iY,z-iZ]
        let scales:number[] = []
        let [p,q] = [this.hashTable,this.hashLength]
        this.directions.forEach((el)=>{
            let t = [fX-el[0],fY-el[1],fZ-el[2]]
            let g = this.grad(p[(hX+el[0])%q],p[(hY+el[1])%q],p[(hZ+el[2])%q])
            scales.push(AriaArithmetic.dot(t,g))
        })
        let lerpX=[...Array(4).keys()].map((i)=>(this.lerp(fX,scales[i*2],scales[i*2+1])))
        let lerpY=[...Array(2).keys()].map((i)=>(this.lerp(fY,lerpX[i*2],lerpX[i*2+1])))
        let lerpZ=this.lerp(fZ,lerpY[0],lerpY[1])
        return lerpZ
    }
}