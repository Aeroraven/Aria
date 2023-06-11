import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComGLTFLoader } from "../components/loader/AriaComGLTFLoader";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComMaterial } from "../components/material/AriaComMaterial";
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaShaderUniformTp } from "../core/graphics/AriaShaderOps";
import { AriaPostFxFXAA } from "../presets/postprocess/AriaPostFxFXAA";
import { AriaPostFxMLAA } from "../presets/postprocess/AriaPostFxMLAA";
import { AriaPostFxInvertColor } from "../presets/postprocess/AriaPostFxInvertColor";
import { AriaStage } from "./AriaStage";
import { AriaPostFxIdentity } from "../presets/postprocess/AriaPostFxIdentity";
import { IAriaRenderable } from "../components/base/interface/IAriaRenderable";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaPostFxSMAAThresholding } from "../presets/postprocess/AriaPostFxSMAAThresholding";
import { AriaPostFxMLAAThresholding } from "../presets/postprocess/AriaPostFxMLAAThresholding";
import { AriaPostFxSMAA } from "../presets/postprocess/AriaPostFxSMAA";
import { AriaPostFxSMAABlendResult } from "../presets/postprocess/AriaPostFxSMAABlendResult";
import { AriaPostFxMLAABlendResult } from "../presets/postprocess/AriaPostFxMLAABlendResult";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";

export class AriaStageAntialiasing extends AriaStage{
    constructor(){
        super("AriaStage/Antialiasing")
    }
    async entry(){
        //Renderer
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        //Resources
        const shaderSource = await (new AriaComShaderLoader()).loadFolder("./shaders/01-hello-world")
        const kleeModel = await(new AriaComGLTFLoader()).load(renderer.getEngine(),"./models/klee2/untitled.gltf")
    
        //Scene
        const canvasOrg = new AriaComCanvas(1)
        const canvas = new AriaComCanvas(1)
        const camera = new AriaComCamera()
        const scene = (new AriaComScene())

        const meshes:AriaComMesh[] = []
        const materials: AriaComMaterial[] = []
    
        for(let i=0;i<kleeModel.geometries.length-1; i++){
            const material = new AriaComShaderMaterial(shaderSource)
            material.addParam("uBackground",AriaShaderUniformTp.ASU_TEX2D, kleeModel.textures[i])
            const mesh = new AriaComMesh(material, kleeModel.geometries[i])
            meshes.push(mesh)
            materials.push(material)
            scene.addChild(mesh)
        }
        
        //Postprocess

        const postIdentity = new AriaPostFxIdentity()
        postIdentity.addInput(canvas)

        const postFXAA = new AriaPostFxFXAA()
        postFXAA.addInput(canvasOrg)

        const postMLAA = new AriaPostFxMLAA()
        postMLAA.addInput(canvasOrg)

        const postSMAA = new AriaPostFxSMAA()
        postSMAA.addInput(canvasOrg)

        const postNOAA = new AriaPostFxIdentity()
        postNOAA.addInput(canvasOrg)

        const postSMAATh = new AriaPostFxSMAAThresholding()
        postSMAATh.addInput(canvasOrg)

        const postMLAATh = new AriaPostFxMLAAThresholding()
        postMLAATh.addInput(canvasOrg)

        const postSMAABl = new AriaPostFxSMAABlendResult()
        postSMAABl.addInput(canvasOrg)

        const postMLAABl = new AriaPostFxMLAABlendResult()
        postMLAABl.addInput(canvasOrg)

        //Activate Components
        camera.initInteraction()
        camera.disableInteraction()
        camera.setPos(0,100,100)

        //Render Schemes
        let activeAAScheme:IAriaRenderable = postMLAA

        //Panel
        let panel = new AriaComParamPanel()
        panel.addTitle("Antialiasing")
        panel.addFPSMeter("FPS")
        panel.addSelector("Algorithm",[
            {key:"mlaa",value:"Morphological Antialiasing (MLAA)"},
            {key:"fxaa",value:"Fast Approximation Antialiasing (FXAA)"},
            {key:"smaa",value:"Subpixel Morphological Antialiasing (SMAA) [Incomplete]"},
            {key:"smaa_th",value:"SMAA / Thresholding"},
            {key:"mlaa_th",value:"MLAA / Thresholding"},
            {key:"smaa_bl",value:"SMAA / Blending"},
            {key:"mlaa_bl",value:"MLAA / Blending"},
            {key:"noaa",value:"No Antialiasing"}
        ],"mlaa",(x)=>{
            if(x=="mlaa"){
                activeAAScheme = postMLAA
            }else if(x=="fxaa"){
                activeAAScheme = postFXAA
            }else if(x=="smaa"){
                activeAAScheme = postSMAA
            }else if(x=="smaa_th"){
                activeAAScheme = postSMAATh
            }else if(x=="mlaa_th"){
                activeAAScheme = postMLAATh
            }else if(x=="smaa_bl"){
                activeAAScheme = postSMAABl
            }else if(x=="mlaa_bl"){
                activeAAScheme = postMLAABl
            }else{
                activeAAScheme = postNOAA
            }
        })
        panel.initInteraction()
    
        //Render
        const renderCall = ()=>{
            renderer.renderScene(camera,scene)
            /*
            renderer.renderComposite(canvasOrg,()=>{
                renderer.renderScene(camera,scene)
            })
            renderer.renderComposite(canvas,()=>{
                renderer.renderSimple(activeAAScheme)
            })
            renderer.renderSimple(postIdentity)*/
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
    
}
