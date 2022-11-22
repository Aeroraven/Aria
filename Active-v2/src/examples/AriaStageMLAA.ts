import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComGLTFLoader } from "../components/loader/AriaComGLTFLoader";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComMaterial } from "../components/material/AriaComMaterial";
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaShaderUniformTp } from "../core/AriaShaderOps";
import { AriaDepthMaterial } from "../presets/materials/AriaDepthMaterial";
import { AriaPostFxMLAABlending } from "../presets/postprocess/AriaPostFxMLAABlending";
import { AriaPostFxMLAAThresholding } from "../presets/postprocess/AriaPostFxMLAAThresholding";
import { AriaPostFxMLAAMixing } from "../presets/postprocess/AriaPostFxMLAAMixing";
import { AriaPostFxIdentity } from "../presets/postprocess/AriaPostFxIdentity";
import { AriaPostFxFXAA } from "../presets/postprocess/AriaPostFxFXAA";
import { AriaPostFxGrayscale } from "../presets/postprocess/AriaPostFxGrayscale";
import { AriaPostFxMLAA } from "../presets/postprocess/AriaPostFxMLAA";
import { AriaPostFxInvertColor } from "../presets/postprocess/AriaPostFxInvertColor";

export class AriaStageMLAA{
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

        const postIdentity = new AriaPostFxInvertColor()
        postIdentity.addInput(canvas)

        const postFXAA = new AriaPostFxFXAA()
        postFXAA.addInput(canvasOrg)

        const postMLAA = new AriaPostFxMLAA()
        postMLAA.addInputW(canvasOrg)
    
        //Activate Components
        camera.initInteraction()
        camera.setPos(0,100,100)
    
        //Render
        const renderCall = ()=>{
            canvasOrg.compose(()=>{
                scene.render()
            })
            canvas.compose(()=>{
                postMLAA.render()
            })
            postIdentity.render()
           
            requestAnimationFrame(renderCall)
        }
        renderCall()
    }
    
}
