import { mat4 } from "gl-matrix-ts";
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

class CoordinateAxis extends AriaComGeometry{
    private posBuf: AriaComVAO
    private eleBuf: AriaComEBO
    private colBuf: AriaComVAO
    constructor(){
        super("CoordinateAxis")
        this._drawingShape = AriaRenderEnumDrawingShape.LINE;
        this.posBuf = new AriaComVAO()
        this.eleBuf = new AriaComEBO()
        this.posBuf.setData([
            -1,0,0,
            1,0,0,
            0,-1,0,
            0,1,0,
            0,0,-1,
            0,0,1,
        ])
        this.colBuf = new AriaComVAO()
        this.colBuf.setData([
            1,0,0,
            1,0,0,
            0,1,0,
            0,1,0,
            0,0,1,
            0,0,1,
        ])
        this.eleBuf.setData([
            0,1,2,3,4,5
        ])
        this._valid = true
    }
    getVertexNumber(): number {
        return 6
    }
    exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        renderer.defineAttribute(AriaGeometryVars.AGV_POSITION,this.posBuf,3)
        renderer.defineAttribute("aCol",this.colBuf,3)
        this.eleBuf.bind(renderer)
    }

}
class SamplingCandidateCollection extends AriaComGeometry{
    private _numCands:number
    private posBuf: AriaComVAO
    private eleBuf: AriaComEBO
    public tZ:number = 5.0
    public tX:number = 0.0
    public focal:number = 1.0
    public tY:number = 0.0
    public useDot:number = 0.3
    public localMat:Float32Array = mat4.create()
    public rotY:number = 0.0

    public rotLb:number = -89.0
    public rotRb:number = 89.0;

    public rotAxis:number[] = [0.0,0.0,0.0]
    public pSize:number = 2.0

    constructor(){
        super("SamplingCandidateCollection")
        this._drawingShape = AriaRenderEnumDrawingShape.POINT;
        this._numCands = 100000
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
            p.push(a,b,Math.random())

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
        renderer.defineUniform("useDot",tpf,this.useDot)
        renderer.defineUniform("uRef",tp,[0.0,0.0,1.0])
        renderer.defineUniform("uRotLb",tpf,this.rotLb)
        renderer.defineUniform("uRotRb",tpf,this.rotRb)
        renderer.defineUniform("pSize",tpf,this.pSize)
        
        renderer.defineUniform("uRotAxis",tp,this.rotAxis)
        //Create RotY
        const rotY = mat4.create()
        mat4.identity(this.localMat)
        mat4.rotateY(this.localMat,this.localMat,this.rotY)
        renderer.defineUniform("uLocalMat",AriaShaderUniformTp.ASU_MAT4,this.localMat)

        this.eleBuf.bind(renderer)
    }
}

export class AriaStageInternalAngleReprojection extends AriaStage{
    constructor(){
        super("AriaStage/InternalAngleReprojection")
    }
    public async entry(): Promise<void> {
        const renderer = new AriaWGL2Renderer("webgl_displayer")

        const axis = new CoordinateAxis()

        const loader = new AriaComShaderLoader()
        const shader = await loader.load("./shaders/iap/shader.vert","./shaders/iap/shader.frag")
        const shaderAxis = await loader.load("./shaders/iapLine/shader.vert","./shaders/iapLine/shader.frag")
        const axisMaterial = new AriaComShaderMaterial(shaderAxis)

        const procedure = new AriaComShaderMaterial(shader)
        const scene = new AriaComScene()
        const pointCollection:SamplingCandidateCollection[] = []
        for(let i=0;i<64;i++){
            const p = new SamplingCandidateCollection()
            p.prepare()
            pointCollection.push(p)

            const collection = new AriaComMesh(procedure,p)
            const axisMesh = new AriaComMesh(axisMaterial,axis)


            scene.addChild(collection)
            scene.addChild(axisMesh)
        }
        
        
        const camera = new AriaComCamera()
        camera.setPos(0,0,2)
        camera.initInteraction()



        const panel = new AriaComParamPanel(200,250)
        panel.addTitle("Internal Angle Projection")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        
        panel.addDynamicText("&nbsp;",()=>"")
        panel.addDynamicText("Visualization Controls",()=>"",true)
        panel.addDynamicText("Deactivate Camera",()=>"Key ESC")
        panel.addDynamicText("Activate Camera",()=>"Key ESC")
        panel.addDynamicText("Camera Position X",()=>"Key A/D (After Activation)")
        
        panel.addDynamicText("Camera Position Z",()=>"Key W/S (After Activation)")
        panel.addDynamicText("Camera Position Y",()=>"Key Q/E (After Activation)")
        panel.addDynamicText("Camera Rotation",()=>"Mouse (After Activation)")
        panel.addSlidebar("Vis Rotation Y",0.0,2000.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotY = x/2000.0*Math.PI*2.0;
            }
            
        },0.0)
        panel.addSlidebar("Point Size",1.0,15.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].pSize = x;
            }
        },2.0)
        panel.addSelector("Color Map",[
            {key:"pos",value:"Position"},
            {key:"norm",value:"Normal"},
            {key:"norma",value:"Abs(Normal)"},
            {key:"normz",value:"Dot(Normal,Z)"},
        ],"pos",(x)=>{
            if(x=="norm"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useDot=0.0;
                }
            }else if(x=="norma"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useDot=1.0;
                }
            }else if(x=="pos"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useDot=0.3;
                }
            }
            else{
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useDot=0.6
                }
            }
        })

        panel.addDynamicText("&nbsp;",()=>"")
        panel.addDynamicText("Parameters (Experiment)",()=>"",true)
        panel.addSlidebar("Translation X",-100.0,100.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].tX=x;
            }
        },0.0) 
        panel.addSlidebar("Translation Y",-100.0,100.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].tY=x;
            }
        },0.0)
        panel.addSlidebar("Translation Z",2.0,100.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].tZ=x;
            }
        },5.0)
        panel.addSlidebar("Focal Length (x100)",1.0,10000.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].focal=x/100;
            }
        },100.0)
        panel.addSlidebar("Rotation Angle Lower Bound",-89,89,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotLb=x;
            }
        },-89)
        panel.addSlidebar("Rotation Angle Upper Bound",-89,89,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotRb=x;
            }
        },89)
        
        panel.addSlidebar("Rotation Axis X (x100)",0,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotAxis[0]=x/100;
            }
        },0)
        panel.addSlidebar("Rotation Axis Y (x100)",0,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotAxis[1]=x/100;
            }
        },0)
        panel.addSlidebar("Rotation Axis Z (x100)",0,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotAxis[2]=x/100;
            }
        },0)

        camera.disableInteraction()
        const drawCall = ()=>{
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}