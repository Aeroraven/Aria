import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComExtPlaneGeometry } from "../components/geometry/primary/AriaComExtPlaneGeometry";
import { AriaComSphere } from "../components/geometry/primary/AriaComSphere";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComDirectionalLight } from "../components/light/AriaComDirectionalLight";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaPhyParticleForceRegistry } from "../components/physics/particle_force/AriaPhyParticleForceRegistry";
import { AriaPhySpringMassCloth } from "../components/physics/cloth/AriaPhySpringMassCloth";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";
import { AriaPhyParticleGravityGenerator } from "../components/physics/particle_force/AriaPhyParticleGravityGenerator";
import { AriaPhySMCSphereBlock } from "../components/physics/cloth_helper/AriaPhySMCSphereBlock";
import { AriaPhyParticleIntegrator } from "../components/physics/particle/AriaPhyParticle";
import { AriaNormalMaterial } from "../presets/materials/AriaNormalMaterial";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaSingleColorMaterial } from "../presets/materials/AriaSingleColorMaterial";

export class AriaStageSpringMassCloth extends AriaStage{
    constructor(){
        super("AriaStage/SpringMassCloth")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const material = new AriaNormalMaterial()
        const sphereMaterial = new AriaSimpleMaterial()
        sphereMaterial.setColor(0.8,0.5,0.5,1)
        
        const anchors = 40
        const scales = 1.5
        const elevation = 0.3
    
        const geometry = new AriaComExtPlaneGeometry(anchors,anchors)
        const blockGeometry = new AriaComSphere()
        blockGeometry.localTranslate(0,-0.6,0)
        blockGeometry.localScale(0.39)

        for(let i=0;i<anchors;i++){
            for(let j=0;j<anchors;j++){
                geometry.setLoc(i,j,[scales*((i+0.609)/anchors-0.5),elevation,scales*((j+0.609)/anchors-0.5)])
            }
        }
        geometry.localTranslateAbsolute(0,0,0)

        const light = new AriaComDirectionalLight()
        light.setLightColor(1,1,1,1)
        light.setLightPosition(1,-1,0)

        const light2 = new AriaComDirectionalLight()
        light2.setLightColor(1,1,1,1)
        light2.setLightPosition(-1,1,0)

        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.35,0.35,0.35,1)

        const mesh = new AriaComMesh(material,geometry)
        const blockMesh = new AriaComMesh(sphereMaterial,blockGeometry)

        //Controllers
        const camera = new AriaComCamera()
        camera.setPos(0,-0.2,2.5)
        camera.initInteraction()
        camera.disableInteraction()

        const scene = new AriaComScene()
        scene.addChild(mesh)
        scene.addChild(blockMesh)

        scene.addComponent(light)
        scene.addComponent(ambient)

        //Blocks
        const sphereBlock = new AriaPhySMCSphereBlock([0,-0.6,0],0.4)

        //Physics
        const forceReg = new AriaPhyParticleForceRegistry()
        const cloth = new AriaPhySpringMassCloth(forceReg,geometry,600, 1/anchors*scales,
            1,0.05,AriaPhyParticleIntegrator.APP_INTEGRATOR_VERLET)
        const delta = 0.02
        const deltaSteps = 4
        const gravity = new AriaPhyParticleGravityGenerator([0,-0.2,0])
        for(let i=0;i<anchors;i++){
            for(let j=0;j<anchors;j++){
                cloth.addForce(i,j,gravity)
            }
        }


        const panel = new AriaComParamPanel()
        panel.addTitle("Cloth")
        panel.addFPSMeter("FPS")
        panel.initInteraction()
        
        const renderCall = ()=>{
            for(let T=0;T<deltaSteps;T++){
                forceReg.update(delta)
                cloth.getParticle(0,0).clearForceAccum()
                cloth.getParticle(anchors-1,0).clearForceAccum()
                //for(let i=0;i<anchors;i++){
                //    cloth.getParticle(0,i).clearForceAccum()
                //    cloth.getParticle(anchors-1,i).clearForceAccum()
                //    cloth.getParticle(i,anchors-1).clearForceAccum()
                //    cloth.getParticle(i,0).clearForceAccum()
                //}
                cloth.integrateParticles(delta)
                sphereBlock.updateAll(cloth)
            }
        
            cloth.sync()
            //light.generateShadowMap(renderer.getEngine(),scene)
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}