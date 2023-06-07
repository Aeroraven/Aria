import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComSphere } from "../components/geometry/primary/AriaComSphere";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComPointLight } from "../components/light/AriaComPointLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaPhyParticleSyncer } from "../components/physics/particle/AriaPhyParticleSyncer";
import { AriaPhyParticleContact } from "../components/physics/particle_contact/AriaPhyParticleContact";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComStopwatch } from "../components/timer/AriaComStopwatch";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";

export class AriaStageParticleHoriContact extends AriaStage{
    constructor(){
        super("AriaStage/ParticleHoriContact")
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
        const sphereA = new AriaComSphere()
        sphereA.localScale(0.1)
        sphereA.localTranslateAbsolute(-1.25,0,-2.5)
        const meshA = new AriaComMesh(sphereMaterial,sphereA)

        const sphereB = new AriaComSphere()
        sphereB.localScale(0.1)
        sphereB.localTranslateAbsolute(1.25,0,-2.5)
        const meshB = new AriaComMesh(sphereMaterial,sphereB)


        //Controllers
        const camera = new AriaComCamera()
        camera.setPos(0,0,0)
        camera.initInteraction()

        const scene = new AriaComScene()
        scene.addChild(meshA)
        scene.addChild(meshB)
        scene.addComponent(light)
        scene.addComponent(ambient)

        const panel = new AriaComParamPanel()
        panel.addTitle("Horizontal Particle Collision")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        //Phy Wrapper
        const spAnnoA = new AriaPhyParticleSyncer(sphereA)
        spAnnoA.particle.mass = 2
        spAnnoA.particle.position.fromArray([-1.25,0,-2.5])
        spAnnoA.particle.velocity.fromArray([0.4,0,0])

        const spAnnoB = new AriaPhyParticleSyncer(sphereB)
        spAnnoB.particle.mass = 1
        spAnnoB.particle.position.fromArray([1.25,0,-2.5])
        spAnnoB.particle.velocity.fromArray([-0.4,0,0])

        const timer = new AriaComStopwatch()
        timer.setScaler(0.001)

        let timerInterval = timer.recordAndGetDuration()

        const contactJudge = ()=>{
            if(spAnnoA.particle.position.sub(spAnnoB.particle.position).len()<=0.2){
                let contact = new AriaPhyParticleContact(spAnnoA.particle,spAnnoB.particle,spAnnoA.particle.position.sub(spAnnoB.particle.position).normalize_(),1.0,0.0)
                contact.resolve(timerInterval)
            }
        }

        let drawCall = ()=>{
            timerInterval = timer.recordAndGetDuration()
            contactJudge()
            spAnnoA.update()
            spAnnoB.update()
            camera.initiateRender(scene)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}