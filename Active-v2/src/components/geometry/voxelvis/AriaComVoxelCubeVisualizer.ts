import { AriaShaderOps } from "../../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { AriaComEBO } from "../../base/AriaComEBO";
import { AriaComVAO } from "../../base/AriaComVAO";
import { AriaVoxel } from "../../voxel/AriaVoxel";
import { AriaComGeometry, AriaGeometryVars } from "../base/AriaComGeometry";

export class AriaComVoxelCubeVisualizer extends AriaComGeometry{
    private posBuf: AriaComVAO
    private normBuf: AriaComVAO
    private eleBuf: AriaComEBO
    private numVs = 0

    constructor(voxel:AriaVoxel,negateNorm=false){
        super("AriaCom/CubeVisualizer")
        this.posBuf = new AriaComVAO()
        this.normBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()

        let voxels = voxel.getAllVoxels()
        const nbfst = []
        const eleData = []
        const posData = []
        const mag = voxel.getMag()
        const nbfsf = negateNorm?-1:1
        const nbfs = [
            [0,0,-1],
            [0,0,1],
            [-1,0,0],
            [1,0,0],
            [0,-1,0],
            [0,1,0]
        ]
        const posf = [
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
        ]
        for(let k=0;k<voxels.length;k++){
            //Norms
            for(let i=0;i<6;i++){
                for(let j=0;j<6;j++){
                    nbfst.push(nbfs[i][0]*nbfsf)
                    nbfst.push(nbfs[i][1]*nbfsf)
                    nbfst.push(nbfs[i][2]*nbfsf)
                    
                }
            }
            //Position
            for(let i=0;i<posf.length;i+=3){
                posData.push(posf[i]+voxels[k].pos[0])
                posData.push(posf[i+1]+voxels[k].pos[1])
                posData.push(posf[i+2]+voxels[k].pos[2])
            }
            //Element
            for(let i=0;i<36;i++){
                eleData.push(i+k*36)
            }
        }
        this.numVs = voxels.length * 36
        this.posBuf.setData(posData)
        this.eleBuf.setData(eleData)
        this.normBuf.setData(nbfst)
        this._valid = true
    }
    public exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        renderer.defineAttribute(AriaGeometryVars.AGV_POSITION, this.posBuf)
        renderer.defineAttribute(AriaGeometryVars.AGV_NORMAL,this.normBuf)
        this.eleBuf.bind()
    }

    public  getVertexNumber(): number{
        return this.numVs
    }
}