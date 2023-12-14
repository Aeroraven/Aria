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

    public rotLb:number = -180.0
    public rotRb:number = 180.0;

    public rotAxis:number[] = [0.0,0.0,0.0]
    public pSize:number = 2.0

    public disturbX:number = 0.0
    public disturbY:number = 0.0

    public outputNormal:number=0.0
    public preNormal:number[] = [0.0,0.0,0.0]
    public sortVertex:number = 1.0
    public fccn:number = 1.0

    public useUniformDist:number = 1.0

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
            let a = Math.random()
            let b = Math.random()
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

        renderer.defineUniform("disturbX",tpf,this.disturbX)
        renderer.defineUniform("disturbY",tpf,this.disturbY)
        renderer.defineUniform("outputNormal",tpf,this.outputNormal)
        renderer.defineUniform("preNormal",tp,this.preNormal)

        renderer.defineUniform("sortVertex",tpf,this.sortVertex)
        renderer.defineUniform("useUniformDist",tpf,this.useUniformDist)
        renderer.defineUniform("forceCounterClockwiseNormal",tpf,this.fccn)

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
        //panel.addTitle("矩形内角投影")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        
        panel.addDynamicText("&nbsp;",()=>"")
        panel.addDynamicText("可视化控制",()=>"",true)
        /*
        panel.addDynamicText("Deactivate Camera",()=>"Key ESC")
        panel.addDynamicText("Activate Camera",()=>"Key ESC")
        panel.addDynamicText("Camera Position X",()=>"Key A/D (After Activation)")
        
        panel.addDynamicText("Camera Position Z",()=>"Key W/S (After Activation)")
        panel.addDynamicText("Camera Position Y",()=>"Key Q/E (After Activation)")
        panel.addDynamicText("Camera Rotation",()=>"Mouse (After Activation)")*/

        panel.addSlidebar("可视化结果旋转 Y",0.0,2000.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotY = x/2000.0*Math.PI*2.0;
            }
            
        },0.0)
        panel.addSlidebar("点大小",1.0,15.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].pSize = x;
            }
        },2.0)
        panel.addSelector("Color Map",[
            {key:"pos",value:"位置"},
            {key:"norm",value:"法线"},
            {key:"norma",value:"法线,取绝对值"},
            {key:"normz",value:"Z轴与法线的点积"},
            {key:"ndiff",value:"NDIFF"},
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
            }else if(x=="ndiff"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useDot=0.8;
                }
            }
            else{
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useDot=0.6
                }
            }
            
        })

        panel.addDynamicText("&nbsp;",()=>"")
        panel.addDynamicText("实验参数",()=>"",true)
        panel.addSlidebar("矩形中心平移 X",-100.0,100.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].tX=x;
            }
        },0.0) 
        panel.addSlidebar("矩形中心平移 Y",-100.0,100.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].tY=x;
            }
        },0.0)
        panel.addSlidebar("矩形中心平移 Z",2.0,100.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].tZ=x;
            }
        },5.0)
        panel.addSlidebar("焦距 (x100)",1.0,10000.0,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].focal=x/100;
            }
        },100.0)
        panel.addSlidebar("旋转角度下限",-180,180,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotLb=x;
            }
        },-180)
        panel.addSlidebar("旋转角度上限",-180,180,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotRb=x;
            }
        },180)
        
        panel.addSlidebar("转轴 X (x100)",0,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotAxis[0]=x/100;
            }
        },0)
        panel.addSlidebar("转轴 Y (x100)",0,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotAxis[1]=x/100;
            }
        },0)
        panel.addSlidebar("转轴 Z (x100)",0,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].rotAxis[2]=x/100;
            }
        },0)
        panel.addSlidebar("扰动 X",-1000,1000,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].disturbX=x/1000;
            }
        },0)
        panel.addSlidebar("扰动 Y",-1000,1000,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].disturbY=x/1000;
            }
        },0)
        panel.addSlidebar("预设法向 X (x100)",-100,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].preNormal[0]=x/100;
            }
        },0)
        panel.addSlidebar("预设法向 Y (x100)",-100,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].preNormal[1]=x/100;
            }
        },0)
        panel.addSlidebar("预设法向 Z (x100)",-100,100,(x)=>{
            for(let i=0;i<pointCollection.length;i++){
                pointCollection[i].preNormal[2]=x/100;
            }
        },0)
        panel.addSelector("顶点表示",[
            {key:"normal",value:"法向量"},
            {key:"angle",value:"内角值"},
        ],"angle",(x)=>{
            if(x=="normal"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].outputNormal=1.0;
                }
            }else{
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].outputNormal=0.0;
                }
            }
        })

        panel.addSelector("排序顶点",[
            {key:"ena",value:"启用"},
            {key:"dis",value:"禁用"},
        ],"ena",(x)=>{
            if(x=="ena"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].sortVertex=1.0;
                }
            }else{
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].sortVertex=0.0;
                }
            }
        })

        panel.addSelector("随机算法",[
            {key:"rso3",value:"均匀随机SO(3) [部分选项失效]"},
            {key:"raxs",value:"均匀转轴与均匀转角(非均匀SO(3))"},
            
        ],"rso3",(x)=>{
            if(x=="rso3"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useUniformDist=1.0;
                }
            }else{
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].useUniformDist=0.0;
                }
            }
        })

        panel.addSelector("法向方向规范化",[
            {key:"ena",value:"强制按逆时针排序"},
            {key:"dis",value:"保持原定点顺序"},
            
        ],"ena",(x)=>{
            if(x=="ena"){
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].fccn=1.0;
                }
            }else{
                for(let i=0;i<pointCollection.length;i++){
                    pointCollection[i].fccn=0.0;
                }
            }
        })
        camera.disableInteraction()
        const drawCall = ()=>{
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}