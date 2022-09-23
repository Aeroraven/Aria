import { AriaComBuffers } from "../components/core/aria-com-buffers";
import { AriaComMesh, AriaComMeshTextureType } from "../components/core/aria-com-mesh";
import { AriaComScene } from "../components/core/aria-com-mesh-composite";
import { AriaComTexture } from "../components/core/aria-com-texture";
import { AriaComGeometryInstancing } from "../components/effects/aria-com-geometry-instancing";
import { AriaComCube } from "../components/geometry/aria-com-cube";
import { AriaComRect } from "../components/geometry/aria-com-rect";
import { AriaComLightSet } from "../components/light/aria-com-light-set";
import { AriaComSimplePBR } from "../components/material/aria-com-simple-pbr";
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaFramebuffer, AriaFramebufferOption } from "../core/aria-framebuffer";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaStage } from "./aria-stage-base";

export class AriaStageFXAA extends AriaStage{
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
        camera.camPos[0] = 1.5
        camera.camPos[1] = 1.5
        camera.camPos[2] = 1.5

        camera.camFront[0] = -1
        camera.camFront[1] = -1
        camera.camFront[2] = -1
        camera.movePos(0,0,0)
        camera.disableInteraction()

        //First Pass: Scene Paiting
        const fpScene = (<AriaComScene>AriaComScene.create(gl))
        const fpFramebufferOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const fpFramebuffer = new AriaFramebuffer(gl,fpFramebufferOpt)
        const fpGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(0.3)
            .translate(0,0,0)
            .rotateX(17/180*Math.PI)
            .rotateY(48/180*Math.PI)
            .rotateZ(27/180*Math.PI)
            
        const fpBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(fpGeometry)
        const fpLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([0,0,1],[10,10,10])
        const fpInstancing = (<AriaComGeometryInstancing>AriaComGeometryInstancing.create(gl))
            .setNumber(30)
            .generateUniform()
        const fpMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAO(1.0)
            .setAlbedo(1.0,0.1,0.1)
            .setMetallic(0.5)
            .setRoughness(0.5)
        const fpMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("fxaa/scene"))
            .setBuffer(fpBuffer)
            .setCamera(camera)
            .setMaterial(fpMaterial)
            .setLight(fpLight)
            .addAttachments("instancing",fpInstancing)
            .setNumInstances(30)
        
        fpScene.addObject(fpMesh)
        assets.addTexture("fxaa/first",fpFramebuffer.tex)
        
        //Second Pass : FXAA
        const spTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("fxaa/first"))
        const spScene = (<AriaComScene>AriaComScene.create(gl))
        const spGeometry = (<AriaComRect>AriaComRect.create(gl))
        const spBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(spGeometry)
        const spMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("fxaa/post"))
            .setBuffer(spBuffer)
            .setCamera(camera)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,spTexture)
        spScene.addObject(spMesh)

        //End of Scene Def

        AriaPageIndicator.getInstance().done()
        
        let last = Date.now()
        let turns = 0
        let t=0;

        function clearScene(gl:WebGL2RenderingContext){ 
            gl.clearColor(0,0,0,1);
            gl.enable(gl.DEPTH_TEST);
            gl.depthFunc(gl.LEQUAL);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
        } 

        const renderFunc = ()=>{
            //FPS Update
            turns++;
            if(turns%5==0){
                let fps = 1000/(Date.now()-last)*5;
                last = Date.now()
                AriaPageIndicator.getInstance().updateFPS(fps)
                turns = 0;
            }

            //Camera Move
            t+=0.0005
            let camPosZ = 5*Math.sin(t)
            let camPosX = 5*Math.cos(t)
            let camPosY = 0;
            camera.camPos[0] = camPosX
            camera.camPos[1] = camPosY
            camera.camPos[2] = camPosZ

            let w = Math.sqrt(camPosX*camPosX+camPosY*camPosY+camPosZ*camPosZ)

            camera.camFront[0] = -camPosX/w
            camera.camFront[1] = -camPosY/w
            camera.camFront[2] = -camPosZ/w

            camera.movePos(0,0,0)

            //First Pass
            fpFramebuffer.bind()
            clearScene(gl)
            fpScene.render()
            fpFramebuffer.unbind()

            //Second Pass
            clearScene(gl)
            spScene.render()
        }

        this.renderEnt = renderFunc
    
    }
    
    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }
}