import { AriaComEBO } from "../components/base/AriaComEBO";
import { AriaComVAO } from "../components/base/AriaComVAO";
import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComGeometry, AriaGeometryVars } from "../components/geometry/base/AriaComGeometry";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaShaderUniformTp } from "../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../core/interface/IAriaRendererCore";
import { AriaRenderEnumDrawingShape } from "../core/renderer/AriaRendererEnums";
import { AriaStage } from "./AriaStage";

class SamplingCandidateCollection extends AriaComGeometry{
    private _numCands:number
    private posBuf: AriaComVAO
    private eleBuf: AriaComEBO
    public tZ:number = 5.0
    public tX:number = 0.0
    public focal:number = 1.0
    public tY:number = 0.0
    constructor(){
        super("SamplingCandidateCollection")
        this._drawingShape = AriaRenderEnumDrawingShape.POINT;
        this._numCands = 1000000
        this.posBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()
    }

    prepare(){
        let p=[],e=[]
        for(let i=0;i<this._numCands;i++){
            let a = Math.random()*Math.PI*2.0
            let b = Math.random()*Math.PI*2.0


            let ub = 89
            let lb = -89
            let u = Math.random()*(ub-lb)+lb
            let ru = u/180.0*Math.PI
            p.push(a,b,ru)

            e.push(i)
        }
        this.posBuf.setData(p)
        this.eleBuf.setData(e)
        this._valid = true
    }
    getVertexNumber(): number {
        return this._numCands
    }
    exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        renderer.defineAttribute(AriaGeometryVars.AGV_POSITION,this.posBuf,3)
        const tp = AriaShaderUniformTp.ASU_VEC3
        const tpf = AriaShaderUniformTp.ASU_VEC1
        
        renderer.defineUniform("uLT",tp,[-1.0,1.0,0.0])
        renderer.defineUniform("uRT",tp,[1.0,1.0,0.0])
        renderer.defineUniform("uLB",tp,[-1.0,-1.0,0.0])
        renderer.defineUniform("uRB",tp,[1.0,-1.0,0.0])
        renderer.defineUniform("uTranslation",tp,[this.tX,this.tY,this.tZ])
        renderer.defineUniform("uFocalX",tpf,this.focal)
        renderer.defineUniform("uFocalY",tpf,this.focal)
        renderer.defineUniform("uRef",tp,[0.0,0.0,1.0])

        this.eleBuf.bind(renderer)
    }
}

export class AriaStageIAP extends AriaStage{
    constructor(){
        super("AriaStage/IAP")
    }
    public async entry(): Promise<void> {
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        const points = new SamplingCandidateCollection()
        points.prepare()

        const loader = new AriaComShaderLoader()
        const shader = await loader.load("./shaders/iap/shader.vert","./shaders/iap/shader.frag")
        console.log(shader.vertex)
        const procedure = new AriaComShaderMaterial(shader)
        const collection = new AriaComMesh(procedure,points)
        const camera = new AriaComCamera()
        camera.setPos(0,0,2)
        camera.initInteraction()
        const scene = new AriaComScene()
        scene.addChild(collection)

        const panel = new AriaComParamPanel()
        panel.addTitle("IAP")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        panel.addSlidebar("Translation X",-20.0,20.0,(x)=>{
            points.tX = x;
        },0.0) 
        panel.addSlidebar("Translation Y",-20.0,20.0,(x)=>{
            points.tY = x;
        },0.0)
        panel.addSlidebar("Translation Z",5.0,20.0,(x)=>{
            points.tZ = x;
        },5.0)
        panel.addSlidebar("Focal Length",1.0,20.0,(x)=>{
            points.focal = x;
        },1.0)
        const drawCall = ()=>{
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}