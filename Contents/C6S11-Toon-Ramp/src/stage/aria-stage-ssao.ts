import { AriaComBuffers } from "../components/core/aria-com-buffers";
import { AriaComMesh, AriaComMeshTextureType } from "../components/core/aria-com-mesh";
import { AriaComModel, AriaComScene } from "../components/core/aria-com-mesh-composite";
import { AriaComTexture } from "../components/core/aria-com-texture";
import { AriaComGeometryInstancing } from "../components/effects/aria-com-geometry-instancing";
import { AriaComCube } from "../components/geometry/aria-com-cube";
import { AriaComGeometryAttribEnum } from "../components/geometry/aria-com-geometry";
import { AriaComMeshGeometry } from "../components/geometry/aria-com-mesh-geometry";
import { AriaComRect } from "../components/geometry/aria-com-rect";
import { AriaComLightSet } from "../components/light/aria-com-light-set";
import { AriaComSimplePBR } from "../components/material/aria-com-simple-pbr";
import { AriaComPostprocessPass } from "../components/postprocess/aria-com-postprocess-pass";
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaFramebuffer, AriaFramebufferOption } from "../core/aria-framebuffer";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaLoaderGLTF } from "../loaders/aria-loader-gltf";
import { AriaStage } from "./aria-stage-base";

export class AriaStageSSAO extends AriaStage{
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
        camera.camPos[0] = 0
        camera.camPos[1] = 60
        camera.camPos[2] = 150
        camera.roleStep = 0.5
        camera.camFront[0] = 0
        camera.camFront[1] = 0
        camera.camFront[2] = -1
        camera.movePos(0,0,0)
        camera.disableInteraction()

        //Begin Scene Def

        

        //Shadow Frame
        const shadowFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(4)
        const shadowFramebuf = new AriaFramebuffer(gl,shadowFramebufOpt)
        assets.addTexture("klee/shadow", shadowFramebuf.tex)

        const normalFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(2)
        const normalFramebuf = new AriaFramebuffer(gl,normalFramebufOpt)
        assets.addTexture("klee/normal", normalFramebuf.tex)

        const posFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(2)
        const posFramebuf = new AriaFramebuffer(gl,posFramebufOpt)
        assets.addTexture("klee/depth", posFramebuf.tex)

        const ssaoFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(2)
        const ssaoFramebuf = new AriaFramebuffer(gl,ssaoFramebufOpt)
        assets.addTexture("klee/ssao", ssaoFramebuf.tex)

        const diffFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(2)
        const diffFramebuf = new AriaFramebuffer(gl,diffFramebufOpt)
        assets.addTexture("klee/diff", diffFramebuf.tex)

        const deferFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(2)
        const deferFramebuf = new AriaFramebuffer(gl,deferFramebufOpt)
        assets.addTexture("klee/defer", deferFramebuf.tex)


        //Main Scene:
        const lightProjPos = [-50,70,0]//[-30,100,-20]
        const kleeScene = (<AriaComScene>AriaComScene.create(gl))
        const kleeModel = (<AriaComModel>AriaComModel.create(gl))
        const stageLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([1,-1,0],[10,10,10],lightProjPos)

        //Textures
        const normalTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/normal"))
        const posTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/depth"))
        const shadowTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/shadow"))
        const ssaoTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/ssao"))
        const diffTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/diff"))
        const deferTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/defer"))
        //Floor
        
        const floorGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(200)
            .translate(0,5,0)
            .setTopOnly(true)
        const floorMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAO(1)
            .setRoughness(0.5)
            .setAlbedo(1,1,1)
            .setMetallic(0.5)
        const floorBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(floorGeometry)
        const floorMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setLight(stageLight)
            .setBuffer(floorBuffer)
            .setCamera(camera)
            .setMaterial(floorMaterial)
            .setShader(assets.getShader("klee-ssao/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,shadowTex)
            .setTexture(AriaComMeshTextureType.acmtSpecular,ssaoTex)        
            .setLightProjPos(lightProjPos)
        kleeScene.addObject(floorMesh)


        //Klee
        const kleeLoader = new AriaLoaderGLTF(gl)
        const kleeMeshes:AriaComMesh[] = []
        await kleeLoader.loadModel("./models/klee2/untitled.gltf")

        for(let i=0;i<kleeLoader.getTotalMeshes();i++){
            const kleeShadowTex = (<AriaComTexture>AriaComTexture.create(gl))
                .setTex(assets.getTexture("klee/shadow"))
            const kleeBaseTex = (<AriaComTexture>AriaComTexture.create(gl))
                .setTex(kleeLoader.getBaseMaterialTexture(i))
            const kleeModelRaw = (<AriaComMeshGeometry>AriaComMeshGeometry.create(gl))
            const kleeBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
                .addGeometry(kleeModelRaw)
            const w = kleeLoader.getPosBuffer(i)
            const n = kleeLoader.getNormalBuffer(i)
            const t = kleeLoader.getTexBuffer(i)
            kleeModelRaw.setBuffer(AriaComGeometryAttribEnum.acgaPosition,w.buffer,w.type,w.size)
                .setBuffer(AriaComGeometryAttribEnum.acgaNormal,n.buffer,n.type,n.size)
                .setBuffer(AriaComGeometryAttribEnum.acgaTextureVertex,t.buffer,t.type,t.size)
                .setBuffer(AriaComGeometryAttribEnum.acgaElementBuffer,kleeLoader.getElementBuffer(i),undefined,undefined)
                .setNumVertices(kleeLoader.getElements(i))
            const kleeMesh = (<AriaComMesh>AriaComMesh.create(gl))
                .setCamera(camera)
                .setShader(assets.getShader("klee-ssao/scene"))
                .setBuffer(kleeBuffer)
                .setTexture(AriaComMeshTextureType.acmtDiffuse,kleeBaseTex)
                .setTexture(AriaComMeshTextureType.acmtSpecular,shadowTex)    
                .setTexture(AriaComMeshTextureType.acmtNormal,ssaoTex)    
                .setLight(stageLight)
                .setDepthShader(assets.getShader("klee/shadow"))
                .setLightProjPos(lightProjPos)
            kleeMeshes.push(kleeMesh)
            kleeModel.addObject(kleeMesh)
        }
        kleeScene.addObject(kleeModel)

        //SSAO
        const ssaoPass = (<AriaComPostprocessPass>AriaComPostprocessPass.create(gl))
            .setShader(assets.getShader("klee-ssao/occl"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,posTex)
            .setTexture(AriaComMeshTextureType.acmtSpecular,normalTex)    
            .setCamera(camera)

        //Defer
        const deferPass = (<AriaComPostprocessPass>AriaComPostprocessPass.create(gl))
            .setShader(assets.getShader("klee-ssao/postmix"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,diffTex)
            .setTexture(AriaComMeshTextureType.acmtSpecular,ssaoTex)    
            .setCamera(camera)

        //FXAA
        const fxaaPass = (<AriaComPostprocessPass>AriaComPostprocessPass.create(gl))
            .setShader(assets.getShader("fxaa-all"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,deferTex)
            .setCamera(camera)
        

        //End of Scene Def

        AriaPageIndicator.getInstance().done()
        AriaPageIndicator.getInstance().setSubTitle("Klee (Genshin Impact) Model by Sketchfab/INSTICT. <br/>"+
            " Licensed Under CC-BY. Converted By Sktechfab, Re-exported using Blender <br/>"
            +"https://sketchfab.com/3d-models/klee-fbx-eb4ba3c0d92349cfaf7361e4e55604f0")
        
        let last = Date.now()
        let turns = 0
        let t=0;

        function clearScene(gl:WebGL2RenderingContext){ 
            gl.clearColor(0,0,0,1);
            gl.enable(gl.DEPTH_TEST);
            gl.depthFunc(gl.LEQUAL);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
        } 
        
        const deferPosSetup = ()=>{
            kleeMeshes.forEach((el)=>{
                el.setShader(assets.getShader("klee-ssao/depth"))
            })
            floorMesh.setShader(assets.getShader("klee-ssao/depth"))
        }

        const deferNormalSetup = ()=>{
            kleeMeshes.forEach((el)=>{
                el.setShader(assets.getShader("klee-ssao/normal"))
            })
            floorMesh.setShader(assets.getShader("klee-ssao/normal"))
        }

        const deferUnload = ()=>{
            kleeMeshes.forEach((el)=>{
                el.setShader(assets.getShader("klee-ssao/scene"))
            })
            floorMesh.setShader(assets.getShader("klee-ssao/floor"))
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
    
            //Position Tex
            posFramebuf.bind()
            deferPosSetup()
            clearScene(gl)
            kleeScene.render()
            deferUnload()
            posFramebuf.unbind()

            //Normal Tex
            normalFramebuf.bind()
            deferNormalSetup()
            clearScene(gl)
            kleeScene.render()
            deferUnload()
            normalFramebuf.unbind()

            //SSAO
            ssaoFramebuf.bind()
            clearScene(gl)
            ssaoPass.render()
            ssaoFramebuf.unbind()

            //Shadow Mapping
            shadowFramebuf.bind()
            clearScene(gl)
            kleeScene.renderLightDepthMapS(0)
            shadowFramebuf.unbind()
            
            //Diffuse
            diffFramebuf.bind()
            clearScene(gl)
            kleeScene.render()
            diffFramebuf.unbind()

            //Deferred
            deferFramebuf.bind()
            clearScene(gl)
            deferPass.render()
            deferFramebuf.unbind()

            //FXAA
            clearScene(gl)
            fxaaPass.render()
            
        }
        this.renderEnt = renderFunc
    
    }
    
    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }
}