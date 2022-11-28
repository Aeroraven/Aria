import { mat4 } from "gl-matrix-ts";
import { AriaShaderOps } from "../../core/AriaShaderOps";
import { AriaComEBO } from "../base/AriaComEBO";
import { AriaComVAO } from "../base/AriaComVAO";
import { AriaComGeometry, AriaGeometryVars } from "./AriaComGeometry";

export class AriaComSphere extends AriaComGeometry{
    private posBuf: AriaComVAO
    private texBuf: AriaComVAO
    private eleBuf: AriaComEBO
    private normBuf: AriaComVAO

    private cx: number = 0
    private cy: number = 0
    private cz: number = 0
    private sc: number = 1
    private de: number = 40
    private orgv: number[] = []
    private elv:number[] = []
    private colv:number[] = []
    private texv:number[] = []
    private norv:number[] = []
    private rot: Float32Array = mat4.create()

    constructor(){
        super("AriaCom/Sphere")
        this.posBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()
        this.texBuf = new AriaComVAO()
        this.normBuf = new AriaComVAO()

        this.setDensity(this.de)
        this.generateBuffer()
        this._valid = true
    }

    private setDensity(nb:number){
        this.de = nb
        this.orgv = []
        this.elv = []
        this.texv = []
        this.norv = []
        let idx = 0;
        
        let deltaChange = 2*Math.PI/nb;

        const getCoord = (da:number,dc:number)=>{
            let y = this.sc * Math.sin(dc) + this.cy
            let xz = this.sc * Math.cos(dc)
            let x = xz * Math.cos(da) + this.cx
            let z = xz * Math.sin(da) + this.cz
            return [x,y,z]
        }
        const getCoordN = (da:number,dc:number)=>{
            let y = Math.sin(dc)
            let xz = Math.cos(dc)
            let x = xz * Math.cos(da) 
            let z = xz * Math.sin(da) 
            return [x,y,z]
        }
        const getCoordX = (di:number, dj:number)=>{
            return getCoord(di*deltaChange,dj*deltaChange)
        }

        const getCoordXN = (di:number, dj:number)=>{
            return getCoordN(di*deltaChange,dj*deltaChange)
        }

        const appendVec3 = (x:number[],ap:number[])=>{
            x.push(ap[0],ap[1],ap[2])
            return x
        }

        for(let i=0;i<this.de;i++){
            for(let j=0;j<this.de;j++){

                let lta = getCoordX(i,j)
                let ltb = getCoordX(i+1,j)
                let ltc = getCoordX(i+1,j+1)
                let ltd = getCoordX(i,j+1)

                let ltan = getCoordXN(i,j)
                let ltbn = getCoordXN(i+1,j)
                let ltcn = getCoordXN(i+1,j+1)
                let ltdn = getCoordXN(i,j+1)

                this.orgv.push(lta[0],lta[1],lta[2])
                this.orgv.push(ltb[0],ltb[1],ltb[2])
                this.orgv.push(ltc[0],ltc[1],ltc[2])
                this.texv.push(0,0)
                this.texv.push(1,0)
                this.texv.push(1,1)
                this.norv.push(ltan[0],ltan[1],ltan[2])
                this.norv.push(ltbn[0],ltbn[1],ltbn[2])
                this.norv.push(ltcn[0],ltcn[1],ltcn[2])

                
                this.orgv.push(ltc[0],ltc[1],ltc[2])
                this.orgv.push(ltd[0],ltd[1],ltd[2])
                this.orgv.push(lta[0],lta[1],lta[2])
                this.texv.push(1,1)
                this.texv.push(0,1)
                this.texv.push(0,0)
                this.norv.push(ltcn[0],ltcn[1],ltcn[2])
                this.norv.push(ltdn[0],ltdn[1],ltdn[2])
                this.norv.push(ltan[0],ltan[1],ltan[2])
            }
        }
        for(let i=0;i<this.de*this.de*6;i++){
            this.elv.push(idx)
            idx++;
        }
        
        return this
    }

    private generateBuffer(){
        this.posBuf.setData(this.orgv)
        this.normBuf.setData(this.norv)
        this.eleBuf.setData(this.elv)
    }

    public exportToShader(): void {
        super.exportToShader()
        AriaShaderOps.defineAttribute(AriaGeometryVars.AGV_POSITION, this.posBuf)
        AriaShaderOps.defineAttribute(AriaGeometryVars.AGV_NORMAL, this.normBuf)
        this.eleBuf.bind()
    }

    public  getVertexNumber(): number{
        return this.de*this.de*6
    }
}