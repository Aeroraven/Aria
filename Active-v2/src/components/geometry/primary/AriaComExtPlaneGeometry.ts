import { AriaArithmetic } from "../../../core/arithmetic/AriaArithmetic";
import { AriaShaderOps } from "../../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { AriaComEBO } from "../../base/AriaComEBO";
import { AriaComVAO } from "../../base/AriaComVAO";
import { AriaComGeometry, AriaGeometryVars } from "../base/AriaComGeometry";

export class AriaComExtPlaneGeometry extends AriaComGeometry{
    private posBuf: AriaComVAO
    private normBuf: AriaComVAO
    private eleBuf: AriaComEBO
    private compiledStat:boolean = false

    private cx:number = 0
    private cz:number = 0

    private vy:number[] = []
    private vx:number[] = []
    private vz:number[] = []

    constructor(dx:number,dz:number){
        super("AriaCom/PlaneGeometry")
        this.posBuf = new AriaComVAO()
        this.normBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()
        this.cx = dx
        this.cz = dz

        this.vy = new Array<number>(dx*dz)
        this.vz = new Array<number>(dx*dz)
        this.vy = new Array<number>(dx*dz)

        this._valid = true
    }
    public getDensityX(){
        return this.cx
    }

    public getDensityZ(){
        return this.cz
    }

    public getLoc(x:number,z:number){
        return [this.vx[this.cz*x+z],this.vy[this.cz*x+z],this.vz[this.cz*x+z]]
    }
    public setLoc(x:number,z:number,v:number[]){
        this.vx[this.cz*x+z]=v[0]
        this.vy[this.cz*x+z]=v[1]
        this.vz[this.cz*x+z]=v[2]
        this.compiledStat = false
        
    }
    private getLocID(x:number,z:number){
        return this.cz*x+z
    }
    public compile(){
        let pos:number[] = []
        for(let i=0;i<this.cz*this.cx;i++){
            let px = Math.floor(i/this.cz)
            let pz = i%this.cz
            pos.push(this.getLoc(px,pz)[0],this.getLoc(px,pz)[1],this.getLoc(px,pz)[2])
        }
        let norm:number[] = []
        for(let i=0;i<this.cz*this.cx;i++){
            let adjNorms:number[][] = []
            let px = Math.floor(i/this.cz)
            let pz = i%this.cz
            if(px-1>=0){
                if(pz-1>=0){
                    adjNorms.push(
                        AriaArithmetic.cross(
                            [this.getLoc(px,pz)[0]-this.getLoc(px-1,pz)[0],this.getLoc(px,pz)[1]-this.getLoc(px-1,pz)[1],this.getLoc(px,pz)[2]-this.getLoc(px-1,pz)[2]],
                            [this.getLoc(px,pz-1)[0]-this.getLoc(px,pz)[0],this.getLoc(px,pz-1)[1]-this.getLoc(px,pz)[1],this.getLoc(px,pz-1)[2]-this.getLoc(px,pz)[2]],
                        )
                    )
                }
                if(pz+1<this.cz){
                    adjNorms.push(
                        AriaArithmetic.cross(
                            [this.getLoc(px,pz)[0]-this.getLoc(px,pz+1)[0],this.getLoc(px,pz)[1]-this.getLoc(px,pz+1)[1],this.getLoc(px,pz)[2]-this.getLoc(px,pz+1)[2]],
                            [this.getLoc(px-1,pz)[0]-this.getLoc(px,pz)[0],this.getLoc(px-1,pz)[1]-this.getLoc(px,pz)[1],this.getLoc(px-1,pz)[2]-this.getLoc(px,pz)[2]],
                        )
                    )
                }
            }
            if(px+1<this.cx){
                if(pz-1>=0){
                    adjNorms.push(
                        AriaArithmetic.cross(
                            [this.getLoc(px,pz)[0]-this.getLoc(px,pz-1)[0],this.getLoc(px,pz)[1]-this.getLoc(px,pz-1)[1],this.getLoc(px,pz)[2]-this.getLoc(px,pz-1)[2]],
                            [this.getLoc(px+1,pz)[0]-this.getLoc(px,pz)[0],this.getLoc(px+1,pz)[1]-this.getLoc(px,pz)[1],this.getLoc(px+1,pz)[2]-this.getLoc(px,pz)[2]],
                        )
                    )
                }
                if(pz+1<this.cz){
                    adjNorms.push(
                        AriaArithmetic.cross(
                            [this.getLoc(px,pz)[0]-this.getLoc(px+1,pz)[0],this.getLoc(px,pz)[1]-this.getLoc(px+1,pz)[1],this.getLoc(px,pz)[2]-this.getLoc(px+1,pz)[2]],
                            [this.getLoc(px,pz+1)[0]-this.getLoc(px,pz)[0],this.getLoc(px,pz+1)[1]-this.getLoc(px,pz)[1],this.getLoc(px,pz+1)[2]-this.getLoc(px,pz)[2]],
                        )
                    )
                }
            }

            let avgNorm = AriaArithmetic.average(adjNorms,3)
            norm.push(avgNorm[0],avgNorm[1],avgNorm[2])
        }
        let ele:number[]=[]
        for(let i=0;i<this.cx-1;i++){
            for(let j=0;j<this.cz-1;j++){
                ele.push(this.getLocID(i,j),this.getLocID(i+1,j),this.getLocID(i+1,j+1))
                ele.push(this.getLocID(i+1,j+1),this.getLocID(i,j+1),this.getLocID(i,j))
            }
        }
        this.posBuf = new AriaComVAO()
        this.posBuf.setData(pos)
        this.normBuf = new AriaComVAO()
        this.normBuf.setData(norm)
        this.eleBuf = new AriaComEBO()
        this.eleBuf.setData(ele)
    }
    public getVertexNumber(): number {
        return (this.cx-1)*(this.cz-1)*6
    }
    public exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        if(this.compiledStat==false){
            this.compile()
            this.compiledStat=true
        }
        renderer.defineAttribute(AriaGeometryVars.AGV_POSITION, this.posBuf)
        renderer.defineAttribute(AriaGeometryVars.AGV_NORMAL, this.normBuf)
        this.eleBuf.bind(renderer)
    }
}