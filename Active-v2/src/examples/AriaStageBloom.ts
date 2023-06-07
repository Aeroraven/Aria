import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComGLTFLoader } from "../components/loader/AriaComGLTFLoader";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComMaterial } from "../components/material/AriaComMaterial";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaShaderUniformTp } from "../core/graphics/AriaShaderOps";
import { AriaStage } from "./AriaStage";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSingleColorMaterial } from "../presets/materials/AriaSingleColorMaterial";
import { AriaPostFxSimpleBloom } from "../presets/postprocess/AriaPostFxSimpleBloom";
import { AriaComCube } from "../components/geometry/primary/AriaComCube";

export class AriaStageBloom extends AriaStage{
    constructor(){
        super("AriaStage/Bloom")
    }
    async entry(){
        //Resources
        const shaderSource = await (new AriaComShaderLoader()).loadFolder("./shaders/01-hello-world")
        const kleeModel = await(new AriaComGLTFLoader()).load("./models/klee2/untitled.gltf")
    
        //Scene
        const canvasOrg = new AriaComCanvas(1)
        const camera = new AriaComCamera()
        const scene = (new AriaComScene()).addComponent(camera)

        const meshes:AriaComMesh[] = []
        const materials: AriaComMaterial[] = []
        const geometry = new AriaComCube()

        for(let i=0;i<4; i++){
            //const material = new AriaComShaderMaterial(shaderSource)
            const material = new AriaSingleColorMaterial()
            material.setColor(0.5,0.6,1,1)

            material.addParam("uBackground",AriaShaderUniformTp.ASU_TEX2D,kleeModel.textures[i])
            const mesh = new AriaComMesh(material, geometry)
            meshes.push(mesh)
            materials.push(material)
            scene.addChild(mesh)
        }
        
        //Postprocess
        const postBlurA = new AriaPostFxSimpleBloom()
        postBlurA.setIteration(6)
        postBlurA.addInput(canvasOrg)

        
        //Activate Components
        camera.initInteraction()
        camera.setPos(-3,3,3)
        camera.camFront[0] = 1
        camera.camFront[1] = -1

        //Panel
        let panel = new AriaComParamPanel()
        panel.addTitle("Bloom Effect")
        panel.addFPSMeter("FPS")
        panel.initInteraction()
    
        //Render
        const renderCall = ()=>{
            canvasOrg.compose(()=>{
                scene.render()
            })
            postBlurA.render()
           
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
    
}
