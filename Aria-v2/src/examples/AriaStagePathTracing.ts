import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaStage } from "./AriaStage";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComRTRenderPass } from "../components/pathtracing/AriaComRTRenderPass";
import { AriaComRTSphere } from "../components/pathtracing/geometry/AriaComRTSphere";
import { AriaVec3 } from "../core/arithmetic/AriaVector";
import { AriaComRTFragShaderProcessor } from "../components/pathtracing/AriaComRTFragShaderProcessor";
import { AriaComRTLambertianMaterial } from "../components/pathtracing/material/AriaComRTLambertianMaterial";
import { AriaComRTPlane } from "../components/pathtracing/geometry/AriaComRTPlane";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComRTPingPongTexturePass } from "../components/pathtracing/AriaComRTPingPongTexturePass";
import { AriaPostFxFXAA } from "../presets/postprocess/AriaPostFxFXAA";
import { AriaComRTSpecularMaterial } from "../components/pathtracing/material/AriaComRTSpecularMaterial";
import { AriaPostFxIdentity } from "../presets/postprocess/AriaPostFxIdentity";
import { IAriaRenderable } from "../components/base/interface/IAriaRenderable";
import { AriaPostFxMLAA } from "../presets/postprocess/AriaPostFxMLAA";

export class AriaStagePathTracing extends AriaStage{
    constructor(){
        super("AriaStage/PathTracing")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        const processor = new AriaComRTFragShaderProcessor()
        

        //Raytracing
        const planeMaterial = new AriaComRTLambertianMaterial()
        planeMaterial.setMaterial([1.0,1.0,1.0,1.0])
        const plane = new AriaComRTPlane("plane",AriaVec3.create([0,-2,0]),AriaVec3.create([0,1,0]),planeMaterial)
        processor.add(plane)

        //Place Small Balls
        const balls = 30
        const radius = 1
        const xspan = 40
        const zspan = 40
        const zbase = 20
        const avaMaterials = [AriaComRTLambertianMaterial] //AriaComRTRefractiveMaterial
        
        const rgbGenerator = ()=>{
            return [Math.random()*0.5+0.5,Math.random()*0.5+0.5,Math.random()*0.5+0.5,1.0]
        }
        for(let i=0;i<balls;i++){
            let midx = Math.floor(Math.random()*avaMaterials.length)
            let material = new avaMaterials[midx]()
            if(midx==0){
                material.setMaterial(rgbGenerator())
            }else{
                material.setMaterial([1,1,1,1])
            }
            const ball = new AriaComRTSphere("ball",AriaVec3.create([(Math.random()-0.5)*xspan,-2+radius,zbase+(Math.random()-0.5)*zspan]),radius,material)
            processor.add(ball)
        }

        const specular = new AriaComRTSpecularMaterial()
        specular.setMaterial([0.95,0.95,0.95,1.0])
        const largeBall = new AriaComRTSphere("ball2",AriaVec3.create([3,0.5,12]),2.5,specular)
        processor.add(largeBall)

        //Texture Ping Pong
        const tracer = new AriaComRTRenderPass(processor,90,90,1,window.innerWidth/window.innerHeight)
        tracer.setOrigin([0,1,0])

        const canvas1 = new AriaComCanvas(1,true)
        const canvas2 = new AriaComCanvas(1,true)
        canvas1.forceInit(renderer.getEngine())
        canvas2.forceInit(renderer.getEngine())
        const pingpong = new AriaComRTPingPongTexturePass(canvas1,canvas2,tracer)

        //Debug
        console.log(processor.generateCodes())

        //Antialiasing
        const canvasDraw = new AriaComCanvas()
        const fxaa = new AriaPostFxFXAA()
        const noaa = new AriaPostFxIdentity()
        const mlaa = new AriaPostFxMLAA()

        fxaa.addInput(canvasDraw)
        noaa.addInput(canvasDraw)
        mlaa.addInput(canvasDraw)

        const panel = new AriaComParamPanel()
        let activeAAScheme:IAriaRenderable = mlaa

        panel.addTitle("Path Tracing")
        panel.addFPSMeter("FPS")
        panel.addSelector("Antialiasing",[
            {key:"mlaa",value:"Morphological Antialiasing (MLAA)"},
            {key:"fxaa",value:"Fast Approximation Antialiasing (FXAA)"},
            {key:"noaa",value:"No Antialiasing"}
        ],"mlaa",(x)=>{
            if(x=="mlaa"){
                activeAAScheme = mlaa
            }else if(x=="fxaa"){
                activeAAScheme = fxaa
            }else{
                activeAAScheme = noaa
            }
        })
        

        panel.initInteraction()

        const drawCall = ()=>{
            renderer.renderComposite(canvasDraw,()=>{
                renderer.renderSimple(pingpong)
            })
            renderer.renderSimple(activeAAScheme)

            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}