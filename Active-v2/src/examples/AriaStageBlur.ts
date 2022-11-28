import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComGLTFLoader } from "../components/loader/AriaComGLTFLoader";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComMaterial } from "../components/material/AriaComMaterial";
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaShaderUniformTp } from "../core/AriaShaderOps";
import { AriaStage } from "./AriaStage";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaPostFxGaussianBlur } from "../presets/postprocess/AriaPostFxGaussianBlur";
import { AriaPostFxGaussianBloom } from "../presets/postprocess/AriaPostFxGaussianBloom";
import { AriaComPostPass } from "../components/postproc/AriaComPostPass";
import { AriaPostFxKawaseBlur } from "../presets/postprocess/AriaPostFxKawaseBlur";
import { AriaPostFxMipmapDownsample } from "../presets/postprocess/AriaPostFxMipmapDownsample";

export class AriaStageBlur extends AriaStage{
    constructor(){
        super("AriaStage/Blur")
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
    
        for(let i=0;i<kleeModel.geometries.length; i++){
            const material = new AriaComShaderMaterial(shaderSource)
            material.addParam("uBackground",AriaShaderUniformTp.ASU_TEX2D, kleeModel.textures[i])
            const mesh = new AriaComMesh(material, kleeModel.geometries[i])
            meshes.push(mesh)
            materials.push(material)
            scene.addChild(mesh)
        }
        
        //Postprocess
        const postBlurA = new AriaPostFxGaussianBlur()
        postBlurA.setSigma(5)
        postBlurA.addInput(canvasOrg)

        const postBlurKawase = new AriaPostFxKawaseBlur()
        postBlurKawase.setBlurRange(5)
        postBlurKawase.addInput(canvasOrg)

        const postBlurMipdown = new AriaPostFxMipmapDownsample()
        postBlurMipdown.setLod(3)
        postBlurMipdown.addInput(canvasOrg)

        //Active Blur
        let activeBlurPass:AriaComPostPass = postBlurA

        
        //Activate Components
        camera.initInteraction()
        camera.setPos(0,100,100)


        //Panel
        let panel = new AriaComParamPanel()
        panel.addTitle("Blurring")
        panel.addFPSMeter("FPS")
        panel.addSelector("Algorithm",[
            {key:"gaussian",value:"Gaussian Blur"},
            {key:"kawase", value:"Kawase Blur"},
            {key:"mipdown", value:"MipMap Downsample"}
        ],"gaussian",(x)=>{
            if(x=="gaussian"){
                activeBlurPass = postBlurA
            }else if(x=="kawase"){
                activeBlurPass = postBlurKawase
            }else{
                activeBlurPass = postBlurMipdown
            }
        })
        panel.initInteraction()
    
        //Render
        const renderCall = ()=>{
            canvasOrg.compose(()=>{
                scene.render()
            })
            activeBlurPass.render()
           
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
    
}
