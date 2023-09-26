import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaStage } from "./AriaStage";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComRTRenderPass } from "../components/pathtracing/AriaComRTRenderPass";
import { AriaComRTSphere } from "../components/pathtracing/geometry/AriaComRTSphere";
import { AriaVec3 } from "../core/arithmetic/AriaVector";
import { IAriaComRTAbstractMaterial } from "../components/pathtracing/material/IAriaComRTAbstractMaterial";
import { AriaComRTFragShaderProcessor } from "../components/pathtracing/AriaComRTFragShaderProcessor";
import { AriaComRTLambertianMaterial } from "../components/pathtracing/material/AriaComRTLambertianMaterial";
import { AriaComRTPlane } from "../components/pathtracing/geometry/AriaComRTPlane";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComRTPingPongTexturePass } from "../components/pathtracing/AriaComRTPingPongTexturePass";

export class AriaStagePathTracing extends AriaStage{
    constructor(){
        super("AriaStage/PathTracing")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const material = new AriaComRTLambertianMaterial()
        const ball = new AriaComRTSphere("ball",AriaVec3.create([0,0,12]),4,material)
        const plane = new AriaComRTPlane("plane",AriaVec3.create([0,-2,0]),AriaVec3.create([0,1,0]),material)
        const processor = new AriaComRTFragShaderProcessor()
        processor.add(ball)
        processor.add(plane)
        const tracer = new AriaComRTRenderPass(processor,45,45,1,window.innerWidth/window.innerHeight)
        
        const canvas1 = new AriaComCanvas()
        const canvas2 = new AriaComCanvas()
        canvas1.forceInit(renderer.getEngine())
        canvas2.forceInit(renderer.getEngine())

        const pingpong = new AriaComRTPingPongTexturePass(canvas1,canvas2,tracer)

        const panel = new AriaComParamPanel()
        panel.addTitle("Demo")
        panel.addFPSMeter("FPS")

        panel.initInteraction()

        const drawCall = ()=>{
            renderer.renderSimple(pingpong)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}