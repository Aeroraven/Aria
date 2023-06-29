import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComCube } from "../components/geometry/primary/AriaComCube";
import { AriaComAmbientLight } from "../components/light/AriaComAmbientLight";
import { AriaComDirectionalLight } from "../components/light/AriaComDirectionalLight";
import { AriaComFractalNoise } from "../components/math/noise/AriaComFractalNoise";
import { AriaComImprovedNoise } from "../components/math/noise/AriaComImprovedNoise";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComData3DTexture } from "../components/texture/AriaComData3DTexture";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaCubicVolumeMaterial } from "../presets/materials/AriaCubicVolumeMaterial";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
import { AriaStage } from "./AriaStage";

export class AriaStageVolumetricCloud extends AriaStage{
    constructor(){
        super("AriaStage/VolCloud")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        const TL = 100
        const SCL = 0.01
        
        const cubeMatrial = new AriaCubicVolumeMaterial()

        const volTexture = new AriaComData3DTexture(TL,TL,TL)
        const noise = new AriaComFractalNoise(new AriaComImprovedNoise(),5)
        for(let i=0;i<TL;i++){
            for(let j=0;j<TL;j++){
                for(let k=0;k<TL;k++){
                    let nx = noise.noise(i*SCL,j*SCL,k*SCL);
                    volTexture.setItem(i,j,k,Math.floor(255*nx));
                }
            }
        } 
        cubeMatrial.setVTexture(volTexture)
        
        const cubeGeometry = new AriaComCube()
        const cubeMesh = new AriaComMesh(cubeMatrial,cubeGeometry)

        const light = new AriaComDirectionalLight()
        light.setLightPosition(0,-1,0)
        const ambient = new AriaComAmbientLight()
        ambient.setLightColor(0.1,0.1,0.1,1)
        
        const camera = new AriaComCamera()
        camera.setPos(0,0.0,3.5)
        const scene = new AriaComScene()
        scene.addChild(cubeMesh)
        scene.addComponent(light)
        scene.addComponent(ambient)

        const panel = new AriaComParamPanel()
        panel.addTitle("Ray Marching 2")
        panel.addFPSMeter("FPS")
        panel.initInteraction()
        
        let t = 0
        const renderCall = ()=>{
            camera.setPos(2*Math.sin(t),0,2*Math.cos(t))
            camera.setLookatCenter(0,0,0)
            t+= 0.001
            renderer.renderScene(camera,scene)
            panel.reqAniFrame(renderCall)
        }
        renderCall()
        
    }
}