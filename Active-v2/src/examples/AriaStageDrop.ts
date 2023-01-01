import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComSphere } from "../components/geometry/AriaComSphere";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComPointLight } from "../components/light/AriaComPointLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaPhyParticleForceRegistry } from "../components/physics/particle_force/AriaPhyParticleForceRegistry";
import { AriaPhyParticleGravityGenerator } from "../components/physics/particle_force/AriaPhyParticleGravityGenerator";
import { AriaPhyParticleSyncer } from "../components/physics/particle/AriaPhyParticleSyncer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";
import { AriaPhyParticle } from "../components/physics/particle/AriaPhyParticle";
import { AriaPhyParticleBasicSpringGenerator } from "../components/physics/particle_force/AriaPhyParticleBasicSpringGenerator";
import { AriaVec3 } from "../core/arithmetic/AriaVector";

export class AriaStageDrop extends AriaStage{
    constructor(){
        super("AriaStage/Drop")
    }

    public async entry(): Promise<void> {
        //Materials
        const sphereMaterial = new AriaSimpleMaterial()
        sphereMaterial.setColor(1,0,0,1)

        //Lightings
        const light = new AriaComPointLight()
        light.setLightColor(2,2,2,2)
        light.setLightPosition(0,0,0)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.3,0.3,0.3,1)

        //Geometry
        const sphere = new AriaComSphere()
        sphere.localScale(0.1)
        sphere.localTranslateAbsolute(0,3.5,-3.5)
        const mesh = new AriaComMesh(sphereMaterial,sphere)

        //Controllers
        const camera = new AriaComCamera()
        camera.setPos(0,0,0)
        camera.initInteraction()

        const scene = new AriaComScene()
        scene.addChild(mesh)
        scene.addComponent(light)
        scene.addComponent(ambient)

        const panel = new AriaComParamPanel()
        panel.addTitle("Dropping Ball")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        //Physics
        const spAnno = new AriaPhyParticleSyncer(sphere)
        spAnno.particle.position.fromArray([0,2,-5.5])
        spAnno.particle.damping = 0.5

        const springEnd = AriaVec3.create()
        springEnd.fromArray([0,-0.5,-5.5])

        const spForceReg = new AriaPhyParticleForceRegistry()
        const spGravity = new AriaPhyParticleGravityGenerator([0,-0.05,0])
        spForceReg.add(spAnno.particle,spGravity)
        const spSpringForce = new AriaPhyParticleBasicSpringGenerator(springEnd,2.5,5)
        spForceReg.add(spAnno.particle,spSpringForce)
        
        //Render Call
        const drawCall = ()=>{
            spForceReg.autoUpdate()
            spAnno.update()
            camera.initiateRender(scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}