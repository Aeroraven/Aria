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

export class AriaStageVoxel extends AriaStage{
    constructor(){
        super("AriaStage/Voxel")
    }
    public async entry(): Promise<void> {
        const material = new AriaSimpleMaterial()
        material.setColor(1,1,0,1)

        const voxel = new AriaVoxel()
        voxel.setVoxel(0,0,0,{tsdf:1,valid:true})
        voxel.setVoxel(1,0,0,{tsdf:1,valid:true})
        voxel.setVoxel(1,1,-1,{tsdf:1,valid:true})
    
        const geometry = new AriaComVoxelCubeVisualizer(voxel)

        const light = new AriaComPointLight()
        light.setLightColor(1,1,1,1)
        light.setLightPosition(1,1,1)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.35,0.35,0.35,1)

        const mesh = new AriaComMesh(material,geometry)

        //Controllers
        const camera = new AriaComCamera()
        camera.setPos(0,0,4.5)
        camera.initInteraction()

        const scene = new AriaComScene()
        scene.addChild(mesh)
        scene.addComponent(light)
        scene.addComponent(ambient)


        const panel = new AriaComParamPanel()
        panel.addTitle("Voxel Visualization C")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        
        const renderCall = ()=>{
            camera.initiateRender(scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}