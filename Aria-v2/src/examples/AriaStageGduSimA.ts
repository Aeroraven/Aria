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
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaSimpleColorRampMaterial } from "../presets/materials/AriaSimpleColorRampMaterial";

export class AriaStageGduSimA extends AriaStage{
    constructor(){
        super("AriaStage/GduSimA")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const material = new AriaSimpleMaterial()
        material.setColor(1,1,0,1)

        const sphereMaterial = new AriaSimpleMaterial()
        sphereMaterial.setColor(1,0,0,1)

        const sphereMaterial_2 = new AriaSingleColorMaterial()
        sphereMaterial_2.setColor(1,0,0,1)

        const sphereMaterial_3 = new AriaSimpleColorRampMaterial()
        sphereMaterial_3.setColor(1,0,0,1)


        //const geometry = new AriaComCube(true)
        //geometry.localScale(10)
        
        //const boundbox = new AriaComMesh(material,geometry)
    
        const light = new AriaComPointLight()
        light.setLightColor(2,2,2,2)
        light.setLightPosition(1,1,1)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.3,0.3,0.3,1)

        const camera = new AriaComCamera()
        camera.setPos(0,0,3.5)
        camera.initInteraction()


        const scene = new AriaComScene()
        //scene.addChild(boundbox)
        scene.addComponent(light)
        scene.addComponent(ambient)

        for(let i=0;i<1;i++){
            const sphere = new AriaComSphere()
            sphere.localTranslate(-0.3,0,0)
            sphere.localScale(0.1)
            
            const mesh = new AriaComMesh(sphereMaterial,sphere)
            scene.addChild(mesh)

            const sphere_2 = new AriaComSphere()
            sphere.localTranslate(0.0,0,0)
            sphere_2.localScale(0.1)
            const mesh_2 = new AriaComMesh(sphereMaterial_2,sphere_2)
            scene.addChild(mesh_2)

            const sphere_3 = new AriaComSphere()
            sphere_3.localTranslate(0.3,0,0)
            sphere_3.localScale(0.1)
            const mesh_3 = new AriaComMesh(sphereMaterial_3,sphere_3)
            scene.addChild(mesh_3)
        }
    
        const panel = new AriaComParamPanel()
        panel.addTitle("Point Shadow")
        panel.addFPSMeter("FPS")

        panel.initInteraction()

        const drawCall = ()=>{
            light.generateShadowMap(renderer.getEngine(),scene,2)
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}