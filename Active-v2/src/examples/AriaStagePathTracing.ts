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

export class AriaStagePathTracing extends AriaStage{
    constructor(){
        super("AriaStage/PathTracing")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const material = new AriaComRTLambertianMaterial()
        const ball = new AriaComRTSphere("ball",AriaVec3.create([0,0,12]),1,material)
        const plane = new AriaComRTPlane("plane",AriaVec3.create([0,-2,0]),AriaVec3.create([0,1,0]),material)
        const processor = new AriaComRTFragShaderProcessor()
        
        processor.add(ball)
        processor.add(plane)
        const result = processor.generateCodes()
        console.log(result)
        const tracer = new AriaComRTRenderPass(processor,45,45,1,window.innerWidth/window.innerHeight)        

        const panel = new AriaComParamPanel()
        panel.addTitle("Demo")
        panel.addFPSMeter("FPS")

        panel.initInteraction()

        const drawCall = ()=>{
            renderer.renderSimple(tracer)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}