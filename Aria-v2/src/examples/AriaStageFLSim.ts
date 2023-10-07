import { AriaComTexture } from "../components/base/AriaComTexture"
import { AriaPhyFluid2D } from "../components/physics/fluid_cpu/AriaPhyFluid2D"
import { AriaPhyFluidGrid2D } from "../components/physics/fluid_cpu/AriaPhyFluidGrid2D"
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer"
import { AriaComData2DTexture } from "../components/texture/AriaComData2DTexture"
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel"
import { AriaVec2 } from "../core/arithmetic/AriaVector"
import { AriaPostFxImageDisplay } from "../presets/postprocess/AriaPostFxImageDisplay"
import { AriaPostFxTesting } from "../presets/postprocess/AriaPostFxTestingPass"
import { AriaStage } from "./AriaStage"

export class AriaStageFLSim extends AriaStage{
    constructor(){
        super("AriaStage/FLSim")
    }
    public async entry(){
        const procedure = new AriaPostFxImageDisplay()
        const renderer = new AriaWGL2Renderer("webgl_displayer") 
        const panel = new AriaComParamPanel()

        const size = 40
        const initGrid = new AriaPhyFluidGrid2D(size,size)
        initGrid.setMass(Math.floor(size/2),Math.floor(size/2),25)
        for(let i=0;i<size/2;i++){
            initGrid.setVelocity(Math.floor(size/2),i,AriaVec2.create([0.01,0]))
        }
        

        const fluid = new AriaPhyFluid2D(initGrid)

        const renderCall = ()=>{
            //Prepare Fluid Simulation
            const disp = new AriaComData2DTexture(size,size)
            for(let i=0;i<size;i++){
                for(let j=0;j<size;j++){
                    disp.setItem(i,j,Math.min(255,fluid.getCurrentState().getMass(i,j)*255))
                }
            }
            fluid.proceed()
            procedure.addInputTexture(disp,"uSourceFrame")

            //Render 
            renderer.renderSimple(procedure)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}