import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCubeMapLoader } from "../components/loader/AriaComCubemapLoader";
import { AriaComSkyBox } from "../components/mesh/AriaComSkyBox";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaStage } from "./AriaStage";

export class AriaStageSkyBox extends AriaStage{
    constructor(){
        super("AriaStage/SkyBox")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const cubemap = await (new AriaComCubeMapLoader()).load(renderer.getEngine(),"./textures/skybox")
        const skybox = new AriaComSkyBox()
        skybox.setTexture(cubemap)
        const scene = new AriaComScene()
        const camera = new AriaComCamera()
        camera.setPos(0,0,0)
        camera.initInteraction()
        //scene.addComponent(camera)
        scene.addChild(skybox)

        const panel =  new AriaComParamPanel()
        panel.addTitle("Skybox")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        const renderCall = ()=>{
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}