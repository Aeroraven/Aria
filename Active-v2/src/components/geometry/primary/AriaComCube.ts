import { AriaShaderOps } from "../../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { AriaComEBO } from "../../base/AriaComEBO";
import { AriaComVAO } from "../../base/AriaComVAO";
import { AriaComGeometry, AriaGeometryVars } from "../base/AriaComGeometry";

export class AriaComCube extends AriaComGeometry{
    private posBuf: AriaComVAO
    private normBuf: AriaComVAO
    private eleBuf: AriaComEBO

    constructor(negateNorm=false){
        super("AriaCom/Cube")
        this.posBuf = new AriaComVAO()
        this.normBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()

        const nbfsf = negateNorm?-1:1
        const nbfs = [
            [0,0,-1],
            [0,0,1],
            [-1,0,0],
            [1,0,0],
            [0,-1,0],
            [0,1,0]
        ]
        const nbfst = []
        for(let i=0;i<6;i++){
            for(let j=0;j<6;j++){
                nbfst.push(nbfs[i][0]*nbfsf)
                nbfst.push(nbfs[i][1]*nbfsf)
                nbfst.push(nbfs[i][2]*nbfsf)
                
            }
        }
        this.normBuf.setData(nbfst)

        this.posBuf.setData([
           //Back
           -0.5, -0.5, -0.5,   
           0.5, -0.5, -0.5,   
           0.5,  0.5, -0.5,   
           0.5,  0.5, -0.5,   
          -0.5,  0.5, -0.5,   
          -0.5, -0.5, -0.5,   
           
          //Front
          -0.5, -0.5,  0.5,   
           0.5, -0.5,  0.5,   
           0.5,  0.5,  0.5,   
           0.5,  0.5,  0.5,   
          -0.5,  0.5,  0.5,   
          -0.5, -0.5,  0.5,   
           
          //Left
          -0.5,  0.5,  0.5,   
          -0.5,  0.5, -0.5,   
          -0.5, -0.5, -0.5,   
          -0.5, -0.5, -0.5,   
          -0.5, -0.5,  0.5,   
          -0.5,  0.5,  0.5,   
           

          //Right
           0.5,  0.5,  0.5,   
           0.5,  0.5, -0.5,   
           0.5, -0.5, -0.5,   
           0.5, -0.5, -0.5,   
           0.5, -0.5,  0.5,   
           0.5,  0.5,  0.5,   
           
           //Bottom
          -0.5, -0.5, -0.5,   
           0.5, -0.5, -0.5,   
           0.5, -0.5,  0.5,   
           0.5, -0.5,  0.5,   
          -0.5, -0.5,  0.5,   
          -0.5, -0.5, -0.5,   
           
          //Top
          -0.5,  0.5, -0.5,   
           0.5,  0.5, -0.5,   
           0.5,  0.5,  0.5,   
           0.5,  0.5,  0.5,   
          -0.5,  0.5,  0.5,   
          -0.5,  0.5, -0.5,   
        ])
        const eleData = []
        for(let i=0;i<36;i++){
            eleData.push(i)
        }
        this.eleBuf.setData(eleData)
        this._valid = true
    }
    exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        renderer.defineAttribute(AriaGeometryVars.AGV_POSITION, this.posBuf)
        renderer.defineAttribute(AriaGeometryVars.AGV_NORMAL,this.normBuf)
        this.eleBuf.bind()
    }

    public  getVertexNumber(): number{
        return 36
    }
}