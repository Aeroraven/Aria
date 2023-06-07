import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCubicCanvas } from "../components/canvas/AriaComCubicCanvas";
import { AriaComCube } from "../components/geometry/primary/AriaComCube";
import { AriaComSphere } from "../components/geometry/primary/AriaComSphere";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComPointLight } from "../components/light/AriaComPointLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComSkyBox } from "../components/mesh/AriaComSkyBox";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaDepthMaterial } from "../components/material/AriaDepthMaterial";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaSingleColorMaterial } from "../presets/materials/AriaSingleColorMaterial";
import { AriaPostFxIdentity } from "../presets/postprocess/AriaPostFxIdentity";
import { AriaStage } from "./AriaStage";

export class AriaStagePointLight extends AriaStage{
    constructor(){
        super("AriaStage/PointLight")
    }
    public async entry(): Promise<void> {
        const material = new AriaSimpleMaterial()
        material.setColor(1,1,0,1)

        const sphereMaterial = new AriaSimpleMaterial()
        sphereMaterial.setColor(1,0,0,1)

        const geometry = new AriaComCube(true)
        geometry.localScale(10)
        const boundbox = new AriaComMesh(material,geometry)
    
        const light = new AriaComPointLight()
        light.setLightColor(2,2,2,2)
        light.setLightPosition(0,0,0)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.3,0.3,0.3,1)

        const camera = new AriaComCamera()
        camera.setPos(0,0,3.5)
        camera.initInteraction()


        const scene = new AriaComScene()
        scene.addChild(boundbox)
        scene.addComponent(light)
        scene.addComponent(ambient)

        for(let i=0;i<5;i++){
            const sphere = new AriaComSphere()
            sphere.localScale(0.1)
            sphere.localTranslate(Math.random()*15-7.5,Math.random()*15-7.5,-Math.random()*5-1.5)
            const mesh = new AriaComMesh(sphereMaterial,sphere)
            scene.addChild(mesh)
        }
    
        const panel = new AriaComParamPanel()
        panel.addTitle("Point Shadow")
        panel.addFPSMeter("FPS")

        panel.initInteraction()

        const drawCall = ()=>{
            
            light.generateShadowMap(scene)
            camera.initiateRender(scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}