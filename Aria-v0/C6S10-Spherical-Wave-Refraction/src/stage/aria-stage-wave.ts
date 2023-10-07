import { AriaComBuffers } from "../components/core/aria-com-buffers";
import { AriaComMesh } from "../components/core/aria-com-mesh";
import { AriaComScene } from "../components/core/aria-com-mesh-composite";
import { AriaComTimestamp } from "../components/effects/aria-com-timestamp";
import { AriaComRect } from "../components/geometry/aria-com-rect";
import { AriaComSineSurface } from "../components/geometry/aria-com-sine-surface";
import { AriaComLightSet } from "../components/light/aria-com-light-set";
import { AriaComSimplePBR } from "../components/material/aria-com-simple-pbr";
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaStage } from "./aria-stage-base";

export class AriaStageWave extends AriaStage{
    renderEnt:()=>void

    constructor(){
        super()
        this.renderEnt = ()=>{}
    }

    public async prepare(gl: WebGL2RenderingContext): Promise<any> {
        const camera = new AriaCamera()
        const assets = await AriaAssetLoader.getInstance(gl)
        AriaPageIndicator.getInstance().updateLoadingTip("Preparing")

        //Extensions
        const extCheck = (x:unknown)=>{return (x==null)};
        if(extCheck(gl.getExtension('EXT_color_buffer_float'))){
            alert("Floating buffer is not enabled / EXT_color_buffer_float")
        }

        //Viewport
        gl.viewport(0,0,window.innerWidth,window.innerHeight)
        gl.disable(gl.CULL_FACE)

        //Stencil Test
        gl.enable(gl.STENCIL_TEST)
        gl.stencilOp(gl.KEEP,gl.KEEP,gl.REPLACE)
        gl.stencilFunc(gl.ALWAYS,1,0xff)
        gl.stencilMask(0xff)

        //Camera
        camera.registerInteractionEvent()

        //Scene
        const mainScene = (<AriaComScene>AriaComScene.create(gl))

        //Light
        const dirLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([0,-1,0],[30,30,30])

        //Material
        const pbrMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAlbedo(0.0,0.15,1.0)
            .setMetallic(0.9)
            .setRoughness(0.2)
            .setAO(1.0)

        //Animation
        const shaderTimer  = (<AriaComTimestamp>AriaComTimestamp.create(gl))


        //Mesh
        const postOrgRect = (<AriaComSineSurface>AriaComSineSurface.create(gl))
            .setDensity(40)
            .translate(-5,-4,-2)
            .scale(10)
        const postOrgBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(postOrgRect)
        const postOrgMesh =   (<AriaComMesh>AriaComMesh.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("gerstner-wave"))
            .setMaterial(pbrMaterial)
            .setBuffer(postOrgBuffer)
            .setLight(dirLight)
            .addAttachments("timer",shaderTimer)
        mainScene.addObject(postOrgMesh)

        AriaPageIndicator.getInstance().done()
        let last = Date.now()
        let turns = 0

        function clearScene(gl:WebGL2RenderingContext){ 
            gl.clearColor(1,1,1,1);
            gl.enable(gl.DEPTH_TEST);
            gl.depthFunc(gl.LEQUAL);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
        } 

        const renderCall = ()=>{
            turns++;
            if(turns%5==0){
                let fps = 1000/(Date.now()-last)*5;
                last = Date.now()
                AriaPageIndicator.getInstance().updateFPS(fps)
                turns = 0;
            }

            clearScene(gl)
            mainScene.render()
        }
        this.renderEnt = renderCall
    }

    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }

}