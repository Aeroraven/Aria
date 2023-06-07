import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCube } from "../components/geometry/primary/AriaComCube";
import { AriaComExtPlaneGeometry } from "../components/geometry/primary/AriaComExtPlaneGeometry";
import { AriaComPlaneGeometry } from "../components/geometry/primary/AriaComPlaneGeometry";
import { AriaComSphere } from "../components/geometry/primary/AriaComSphere";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComDirectionalLight } from "../components/light/AriaComDirectionalLight";
import { AriaComPointLight } from "../components/light/AriaComPointLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";

export class AriaStageSurface extends AriaStage{
    constructor(){
        super("AriaStage/Surface")
    }
    public async entry(): Promise<void> {
        const material = new AriaSimpleMaterial()
        material.setColor(1,1,0,1)

    
        const geometry = new AriaComExtPlaneGeometry(40,40)
        for(let i=0;i<40;i++){
            for(let j=0;j<40;j++){
                geometry.setLoc(i,j,[i/40,-(i-20)*(i-20)/400*(j-20)*(j-20)/400*1.5,j/40])
            }
        }
        geometry.localTranslateAbsolute(0,0,0)

        const light = new AriaComDirectionalLight()
        light.setLightColor(1,1,1,1)
        light.setLightPosition(0,-1,0)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.0,0.0,0.0,1)

        const mesh = new AriaComMesh(material,geometry)

        //Controllers
        const camera = new AriaComCamera()
        camera.setPos(0,0,1.5)
        camera.initInteraction()

        const scene = new AriaComScene()
        scene.addChild(mesh)
        scene.addComponent(light)
        scene.addComponent(ambient)

        const panel = new AriaComParamPanel()
        panel.addTitle("Curved Surface")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        const renderCall = ()=>{
            camera.initiateRender(scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}