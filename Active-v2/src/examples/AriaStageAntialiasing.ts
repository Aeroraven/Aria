import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComGLTFLoader } from "../components/loader/AriaComGLTFLoader";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComMaterial } from "../components/material/AriaComMaterial";
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaShaderUniformTp } from "../core/AriaShaderOps";
import { AriaPostFxFXAA } from "../presets/postprocess/AriaPostFxFXAA";
import { AriaPostFxMLAA } from "../presets/postprocess/AriaPostFxMLAA";
import { AriaPostFxInvertColor } from "../presets/postprocess/AriaPostFxInvertColor";
import { AriaStage } from "./AriaStage";
import { AriaPostFxIdentity } from "../presets/postprocess/AriaPostFxIdentity";
import { IAriaRenderable } from "../components/base/interface/IAriaRenderable";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";

export class AriaStageAntialiasing extends AriaStage{
    constructor(){
        super("AriaStage/Antialiasing")
    }
    async entry(){
        //Resources
        const shaderSource = await (new AriaComShaderLoader()).loadFolder("./shaders/01-hello-world")
        const kleeModel = await(new AriaComGLTFLoader()).load("./models/klee2/untitled.gltf")
    
        //Scene
        const canvasOrg = new AriaComCanvas(1)
        const canvas = new AriaComCanvas(1)
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

        const postIdentity = new AriaPostFxIdentity()
        postIdentity.addInput(canvas)

        const postFXAA = new AriaPostFxFXAA()
        postFXAA.addInput(canvasOrg)

        const postMLAA = new AriaPostFxMLAA()
        postMLAA.addInput(canvasOrg)

        const postNOAA = new AriaPostFxIdentity()
        postNOAA.addInput(canvasOrg)

        
        //Activate Components
        camera.initInteraction()
        camera.setPos(0,100,100)

        //Render Schemes
        let activeAAScheme:IAriaRenderable = postMLAA

        //Panel
        let panel = new AriaComParamPanel()
        panel.addTitle("Antialiasing")
        panel.addSelector("Algorithm",[
            {key:"mlaa",value:"Morphological Antialiasing (MLAA)"},
            {key:"fxaa",value:"Fast Approximation Antialiasing (FXAA)"},
            {key:"noaa",value:"No Antialiasing"}
        ],"mlaa",(x)=>{
            if(x=="mlaa"){
                activeAAScheme = postMLAA
            }else if(x=="fxaa"){
                activeAAScheme = postFXAA
            }else{
                activeAAScheme = postNOAA
            }
        })
        panel.initInteraction()
    
        //Render
        const renderCall = ()=>{
            canvasOrg.compose(()=>{
                scene.render()
            })
            canvas.compose(()=>{
                activeAAScheme.render()
            })
            postIdentity.render()
           
            requestAnimationFrame(renderCall)
        }
        renderCall()
    }
    
}
