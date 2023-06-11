import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCube } from "../components/geometry/primary/AriaComCube";
import { AriaComExtPlaneGeometry } from "../components/geometry/primary/AriaComExtPlaneGeometry";
import { AriaComPlaneGeometry } from "../components/geometry/primary/AriaComPlaneGeometry";
import { AriaComSphere } from "../components/geometry/primary/AriaComSphere";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComDirectionalLight } from "../components/light/AriaComDirectionalLight";
import { AriaComPointLight } from "../components/light/AriaComPointLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaPhyParticleForceRegistry } from "../components/physics/particle_force/AriaPhyParticleForceRegistry";
import { AriaPhySpringMassCloth } from "../components/physics/cloth/AriaPhySpringMassCloth";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";
import { AriaPhyParticleGravityGenerator } from "../components/physics/particle_force/AriaPhyParticleGravityGenerator";
import { AriaVoxel } from "../components/voxel/AriaVoxel";
import { AriaComVoxelCubeVisualizer } from "../components/geometry/voxelvis/AriaComVoxelCubeVisualizer";
import { AriaComMaterial } from "../components/material/AriaComMaterial";
import { AriaComCanvas } from "../components/canvas/AriaComCanvas";
import { AriaComGLTFLoader } from "../components/loader/AriaComGLTFLoader";
import { AriaComShaderLoader } from "../components/loader/AriaComShaderLoader";
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial";
import { AriaShaderUniformTp } from "../core/graphics/AriaShaderOps";
import { AriaComVoxelizer } from "../components/voxel/voxelization/AriaComVoxelizer";
import { AriaAuxiliaryOps } from "../core/AriaAuxiliaryOps";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";

export class AriaStageVoxelKlee extends AriaStage{
    constructor(){
        super("AriaStage/VoxelKlee")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        
        const panel = new AriaComParamPanel()
        panel.addTitle("Voxelization (Non-optimized)")
        panel.addFPSMeter("FPS")
        panel.addStatusBar("Status")
        panel.initInteraction()

        //Resources
        panel.setStatus("Loading model")
        const kleeModel = await(new AriaComGLTFLoader()).load(renderer.getEngine(),"./models/klee2/untitled.gltf")

        const toTrivialTriangles = (ind:Uint16Array,faces:Float32Array)=>{
            this._logInfo("Converting voxels")
            let res:number[][] = []
            for(let i=0;i<ind.length;i+=1){
                res.push([faces[ind[i]*3+0],faces[ind[i]*3+1],faces[ind[i]*3+2]])
            }
            return res
        }

        //Scene
        const camera = new AriaComCamera()
        const scene = (new AriaComScene())

        const meshes:AriaComMesh[] = []
        const materials: AriaComMaterial[] = []

        const light = new AriaComPointLight()
        light.setLightColor(15,15,15,1)
        light.setLightPosition(0,90,50)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.35,0.35,0.35,1)
    
        for(let i=0;i<kleeModel.geometries.length; i++){
            const material = new AriaSimpleMaterial()
            material.setColor(1,1,1,1)

            panel.setStatus("Building voxels - "+i)

            const triangleData = toTrivialTriangles(kleeModel.bufData[i].elements,kleeModel.bufData[i].position)
            const splitTriData = AriaAuxiliaryOps.splitArray(triangleData,180)
            for(let k=0;k<splitTriData.length;k++){
                const voxel = new AriaComVoxelizer(splitTriData[k],0.5,2)
                const voxelvis = new AriaComVoxelCubeVisualizer(voxel)
                //voxelvis.localScale(0.5)

                const mesh = new AriaComMesh(material, voxelvis)
                meshes.push(mesh)
                materials.push(material)
                scene.addChild(mesh)
            }
        }
        scene.addComponent(light)
        scene.addComponent(ambient)


        panel.setStatus("Done")
        camera.initInteraction()
        camera.disableInteraction()
        camera.roleStep = 0.3
        camera.setPos(0,100,100)

        
        const renderCall = ()=>{
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}