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

export class AriaStageSpringMassCloth extends AriaStage{
    constructor(){
        super("AriaStage/SpringMassCloth")
    }
    public async entry(): Promise<void> {
        const material = new AriaSimpleMaterial()
        material.setColor(1,1,0,1)
        const anchors = 20
    
        const geometry = new AriaComExtPlaneGeometry(anchors,anchors)
        for(let i=0;i<anchors;i++){
            for(let j=0;j<anchors;j++){
                geometry.setLoc(i,j,[i/anchors-0.5,0,j/anchors-0.5])
            }
        }
        geometry.localTranslateAbsolute(0,0,0)

        const light = new AriaComDirectionalLight()
        light.setLightColor(1,1,1,1)
        light.setLightPosition(0,-1,0)

        const light2 = new AriaComDirectionalLight()
        light2.setLightColor(1,1,1,1)
        light2.setLightPosition(-1,-1,0)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.35,0.35,0.35,1)

        const mesh = new AriaComMesh(material,geometry)

        //Controllers
        const camera = new AriaComCamera()
        camera.setPos(0,-0.5,2.5)
        camera.initInteraction()
        camera.disableInteraction()

        const scene = new AriaComScene()
        scene.addChild(mesh)
        scene.addComponent(light)
        //scene.addComponent(light2)
        scene.addComponent(ambient)

        //Physics
        const forceReg = new AriaPhyParticleForceRegistry()
        const cloth = new AriaPhySpringMassCloth(forceReg,geometry,210.0, 1/anchors ,1,0.70)
        const delta = 0.004
        const deltaSteps = 8
        const gravity = new AriaPhyParticleGravityGenerator([0,-0.2,0])
        for(let i=0;i<anchors;i++){
            for(let j=0;j<anchors;j++){
                cloth.addForce(i,j,gravity)
            }
        }


        const panel = new AriaComParamPanel()
        panel.addTitle("Mass Spring Cloth (Without Collision)")
        panel.addFPSMeter("FPS")
        panel.initInteraction()

        
        const renderCall = ()=>{
            for(let T=0;T<deltaSteps;T++){
                forceReg.update(delta)

                cloth.getParticle(0,0).clearForceAccum()
                cloth.getParticle(anchors-1,0).clearForceAccum()
                //cloth.getParticle(anchors/2,anchors/2).clearForceAccum()
                /*
                for(let i=0;i<anchors;i++){
                    //cloth.getParticle(anchors-1,i).clearForceAccum()
                    //cloth.getParticle(i,0).clearForceAccum()
                    //cloth.getParticle(0,i).clearForceAccum()
                    //cloth.getParticle(i,anchors-1).clearForceAccum()
                }*/
                cloth.integrateParticles(delta)
            }
        
            cloth.sync()

            camera.initiateRender(scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}