import { AriaShaderOps } from "../../core/AriaShaderOps";
import { AriaComEBO } from "../base/AriaComEBO";
import { AriaComVAO } from "../base/AriaComVAO";
import { AriaComGeometry, AriaGeometryVars } from "./AriaComGeometry";

export class AriaComTestGeometry extends AriaComGeometry{
    private posBuf: AriaComVAO
    private eleBuf: AriaComEBO

    constructor(){
        super("AriaCom/TestGeometry")
        this.posBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()

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

    public exportToShader(): void {
        super.exportToShader()
        AriaShaderOps.defineAttribute(AriaGeometryVars.AGV_POSITION, this.posBuf)
        this.eleBuf.bind()
    }

    public  getVertexNumber(): number{
        return 36
    }
}