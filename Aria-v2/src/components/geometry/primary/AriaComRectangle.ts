import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { AriaComEBO } from "../../base/AriaComEBO";
import { AriaComVAO } from "../../base/AriaComVAO";
import { AriaComGeometry, AriaGeometryVars } from "../base/AriaComGeometry";

export class AriaComRectangle extends AriaComGeometry{
    private posBuf: AriaComVAO
    private texBuf: AriaComVAO
    private eleBuf: AriaComEBO

    constructor(){
        super("AriaCom/Rectangle")
        this.posBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()
        this.texBuf = new AriaComVAO()
        this.generateBuffer()
        this._valid = true
    }
    private generateBuffer(){
        this.posBuf.setData([
          //Front
          -1, -1,  -1,   
           1, -1,  -1,   
           1,  1,  -1,   
          -1,  1,  -1,     
        ])

        this.texBuf.setData([
            //Front
             0,  0,    
             1,  0,    
             1,  1,        
             0,  1,    
        ])

        const eleData = [0,1,2,2,3,0]
        this.eleBuf.setData(eleData)
    }
    public exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        renderer.defineAttribute(AriaGeometryVars.AGV_POSITION, this.posBuf)
        renderer.defineAttribute(AriaGeometryVars.AGV_TEXTURE_POSITION, this.texBuf, 2)
        this.eleBuf.bind(renderer)
    }

    public  getVertexNumber(): number{
        return 6
    }
}