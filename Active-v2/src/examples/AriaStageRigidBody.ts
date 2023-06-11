import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCube } from "../components/geometry/primary/AriaComCube";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComDirectionalLight } from "../components/light/AriaComDirectionalLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";

export class AriaStageRigidBody extends AriaStage{
    constructor(){
        super("AriaStage/RigidBody")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const cubeMatrial = new AriaSimpleMaterial()
        cubeMatrial.setColor(1,0.5,0,1)
        const cubeGeometry = new AriaComCube()
        const cubeMesh = new AriaComMesh(cubeMatrial,cubeGeometry)

        const light = new AriaComDirectionalLight()
        light.setLightPosition(0,-1,0)
        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.1,0.1,0.1,1)
        
        const camera = new AriaComCamera()
        camera.setPos(0,0.2,3.5)
        const scene = new AriaComScene()
        scene.addChild(cubeMesh)
        scene.addComponent(light)
        scene.addComponent(ambient)

        const panel = new AriaComParamPanel()
        panel.addFPSMeter("FPS")
        
        const renderCall = ()=>{
            cubeGeometry.localRotateX(0.02)
            cubeGeometry.localRotateY(0.03)
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
        
    }
}