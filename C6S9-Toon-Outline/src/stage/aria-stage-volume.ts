import { AriaComBuffers } from "../components/core/aria-com-buffers";
import { AriaComMesh, AriaComMeshTextureType } from "../components/core/aria-com-mesh";
import { AriaComScene } from "../components/core/aria-com-mesh-composite";
import { AriaComTexture } from "../components/core/aria-com-texture";
import { AriaComCube } from "../components/geometry/aria-com-cube";
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaFramebuffer, AriaFramebufferOption } from "../core/aria-framebuffer";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaStage } from "./aria-stage-base";

export class AriaStageVolume extends AriaStage{
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

        //First Pass: Obtaining Backend Position (Endpoint of Ray) 
        const fpScene = (<AriaComScene>AriaComScene.create(gl))
        const fpFramebufferOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const fpFramebuffer = new AriaFramebuffer(gl,fpFramebufferOpt)
        assets.addTexture("scene/volume-first",fpFramebuffer.tex)

        const bboxMeshGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(1)
            .translate(0,0,0)
        const bboxMeshBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(bboxMeshGeometry)
        const bboxMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("volume-render/first"))
            .setCamera(camera)
            .setBuffer(bboxMeshBuffer)
        fpScene.addObject(bboxMesh)
        
        function clearScene(gl:WebGL2RenderingContext){ 
            gl.clearColor(0,0,0,1);
            gl.enable(gl.DEPTH_TEST);
            gl.depthFunc(gl.LEQUAL);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
        } 

        //Second Pass: Ray Marching
        const spBackTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("scene/volume-first"))
        const spOpacityTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("bonsai/volume"))
        const spScene = (<AriaComScene>AriaComScene.create(gl))
        const bboxMeshAfter = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("volume-render/second"))
            .setCamera(camera)
            .setBuffer(bboxMeshBuffer)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,spOpacityTex)
            .setTexture(AriaComMeshTextureType.acmtSpecular,spBackTex)
        spScene.addObject(bboxMeshAfter);

        AriaPageIndicator.getInstance().done()
        AriaPageIndicator.getInstance().setSubTitle("Bonsai Model by Leandro Barbagallo. <br/>"
            +"https://github.com/lebarba/WebGLVolumeRendering/tree/5de6351a17d53645919232804dded6de7a060c61/Web/bonsai.raw.png")
        let last = Date.now()
        let turns = 0
        let t=0;

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
            t+=0.01
            let camPosZ = 2*Math.sin(t)
            let camPosX = 2*Math.cos(t)
            let camPosY = 0;
            camera.camPos[0] = camPosX
            camera.camPos[1] = camPosY
            camera.camPos[2] = camPosZ

            let w = Math.sqrt(camPosX*camPosX+camPosY*camPosY+camPosZ*camPosZ)

            camera.camFront[0] = -camPosX/w
            camera.camFront[1] = -camPosY/w
            camera.camFront[2] = -camPosZ/w

            camera.movePos(0,0,0)

            //First Pass Render
            fpFramebuffer.bind()
            clearScene(gl)
            fpScene.render()
            fpFramebuffer.unbind()

            //Second Pass Render
            clearScene(gl)
            spScene.render()
        }

        this.renderEnt = renderFunc
    
    }
    
    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }
}