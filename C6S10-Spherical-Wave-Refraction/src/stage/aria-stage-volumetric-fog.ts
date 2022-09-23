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
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaFramebuffer, AriaFramebufferOption } from "../core/aria-framebuffer";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaLoaderGLTF } from "../loaders/aria-loader-gltf";
import { AriaStage } from "./aria-stage-base";

export class AriaStageVolumetricFog extends AriaStage{
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

        //Overall Light
        const lightProjPos = [-220,220,0]//[-30,100,-20]
        const stageLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([1,-1,0],[10,10,10],lightProjPos)

        //===========Render Pass F: Depth Pass=======
        const depthFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(2)
        const depthFramebuf = new AriaFramebuffer(gl,depthFramebufOpt)
        assets.addTexture("klee/depth", depthFramebuf.tex)

        //===========Render Pass A&B: Main Scene & Dirshadow baking=======

        //Shadow Frame
        const orgFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const orgFramebuf = new AriaFramebuffer(gl,orgFramebufOpt)
        assets.addTexture("klee/original", orgFramebuf.tex)

        //Shadow Frame
        const shadowFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(6)
        const shadowFramebuf = new AriaFramebuffer(gl,shadowFramebufOpt)
        assets.addTexture("klee/shadow", shadowFramebuf.tex)

        //Main Scene:
        const kleeScene = (<AriaComScene>AriaComScene.create(gl))
        const kleeModel = (<AriaComModel>AriaComModel.create(gl))
        
        
        //Floor
        const floorShadowTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/shadow"))
        const floorGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(160)
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
            .setShader(assets.getShader("klee/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,floorShadowTex)
            .setLightProjPos(lightProjPos)
        kleeScene.addObject(floorMesh)

        //Cube
        const cubeShadowTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/shadow"))
        const cubeGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scalex(20,40,200)
            .translate(-70,20,-50)
        const cubeMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAO(1)
            .setRoughness(0.5)
            .setAlbedo(1,0,0)
            .setMetallic(0.5)
        const cubeBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(cubeGeometry)
        const cubeMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setLight(stageLight)
            .setBuffer(cubeBuffer)
            .setCamera(camera)
            .setMaterial(cubeMaterial)
            .setShader(assets.getShader("klee/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,cubeShadowTex)
            .setLightProjPos(lightProjPos)
        kleeScene.addObject(cubeMesh)

        //Cube2
        const bcubeShadowTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/shadow"))
        const bcubeGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scalex(20,200,200)
            .translate(-70,230,-50)
        const bcubeMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAO(1)
            .setRoughness(0.5)
            .setAlbedo(1,0,0)
            .setMetallic(0.5)
        const bcubeBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(bcubeGeometry)
        const bcubeMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setLight(stageLight)
            .setBuffer(bcubeBuffer)
            .setCamera(camera)
            .setMaterial(bcubeMaterial)
            .setShader(assets.getShader("klee/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,bcubeShadowTex)
            .setLightProjPos(lightProjPos)
        kleeScene.addObject(bcubeMesh)

        //Back
        const bfloorGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(160)
            .translate(0,80,-80)
            .rotateX(0.5*Math.PI)
            .setTopOnly(true)
        const bfloorMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAO(1)
            .setRoughness(0.5)
            .setAlbedo(1,1,1)
            .setMetallic(0.5)
        const bfloorBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(bfloorGeometry)
        const bfloorMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setLight(stageLight)
            .setBuffer(bfloorBuffer)
            .setCamera(camera)
            .setMaterial(bfloorMaterial)
            .setShader(assets.getShader("klee/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,floorShadowTex)
            .setLightProjPos(lightProjPos)
        kleeScene.addObject(bfloorMesh)

        //Klee
        const kleeLoader = new AriaLoaderGLTF(gl)
        await kleeLoader.loadModel("./models/klee2/untitled.gltf")
        const kleeMeshes:AriaComMesh[] = []
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
                .setShader(assets.getShader("klee/scene"))
                .setBuffer(kleeBuffer)
                .setTexture(AriaComMeshTextureType.acmtDiffuse,kleeBaseTex)
                .setTexture(AriaComMeshTextureType.acmtSpecular,kleeShadowTex)
                .setLight(stageLight)
                .setDepthShader(assets.getShader("klee/shadow"))
                .setLightProjPos(lightProjPos)
            kleeMeshes.push(kleeMesh)
            kleeModel.addObject(kleeMesh)
        }
        kleeScene.addObject(kleeModel)

        //RayMarching
        const mainFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const mainFramebuf = new AriaFramebuffer(gl,mainFramebufOpt)
        assets.addTexture("klee/final", mainFramebuf.tex)
        const mainScene = (<AriaComScene>AriaComScene.create(gl))

        //Mesh
        const postTexA = (<AriaComTexture>AriaComTexture.create(gl))
                .setTex(assets.getTexture("klee/depth"))
        const postTexB = (<AriaComTexture>AriaComTexture.create(gl))
                .setTex(assets.getTexture("klee/shadow"))
        const postTexC = (<AriaComTexture>AriaComTexture.create(gl))
                .setTex(assets.getTexture("klee/original"))
        const postOrgRect = (<AriaComRect>AriaComRect.create(gl))
        const postOrgBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(postOrgRect)
        const postOrgMesh =   (<AriaComMesh>AriaComMesh.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("klee/vlight"))
            .setBuffer(postOrgBuffer)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,postTexA)
            .setTexture(AriaComMeshTextureType.acmtSpecular,postTexB)
            .setTexture(AriaComMeshTextureType.acmtNormal,postTexC)
            .setLight(stageLight)
        mainScene.addObject(postOrgMesh)

        //Gaussian H Pass
        const ghFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const ghFramebuf = new AriaFramebuffer(gl,ghFramebufOpt)
        assets.addTexture("klee/gh", ghFramebuf.tex)
        const ghTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/final"))
        const ghScene = (<AriaComScene>AriaComScene.create(gl))
        const ghGeometry = (<AriaComRect>AriaComRect.create(gl))
        const ghBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(ghGeometry)
        const ghMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("gaussian-horizontal"))
            .setBuffer(ghBuffer)
            .setCamera(camera)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,ghTexture)
        ghScene.addObject(ghMesh)

        //Gaussian V Pass
        const gvFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const gvFramebuf = new AriaFramebuffer(gl,gvFramebufOpt)
        assets.addTexture("klee/gv", gvFramebuf.tex)
        const gvTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/gh"))
        const gvScene = (<AriaComScene>AriaComScene.create(gl))
        const gvGeometry = (<AriaComRect>AriaComRect.create(gl))
        const gvBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(gvGeometry)
        const gvMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("gaussian-vertical"))
            .setBuffer(gvBuffer)
            .setCamera(camera)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,gvTexture)
        gvScene.addObject(gvMesh)

        //Mixing
        const mxFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const mxFramebuf = new AriaFramebuffer(gl,mxFramebufOpt)
        assets.addTexture("klee/mix", mxFramebuf.tex)
        const mxTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/gv"))
        const mxScene = (<AriaComScene>AriaComScene.create(gl))
        const mxGeometry = (<AriaComRect>AriaComRect.create(gl))
        const mxBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(mxGeometry)
        const mxMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("klee/mix"))
            .setBuffer(mxBuffer)
            .setCamera(camera)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,mxTexture)
            .setTexture(AriaComMeshTextureType.acmtSpecular,postTexC)
        mxScene.addObject(mxMesh)

        //FXAA Pass
        const spTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/mix"))
        const spScene = (<AriaComScene>AriaComScene.create(gl))
        const spGeometry = (<AriaComRect>AriaComRect.create(gl))
        const spBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(spGeometry)
        const spMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setShader(assets.getShader("fxaa-all"))
            .setBuffer(spBuffer)
            .setCamera(camera)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,spTexture)
        spScene.addObject(spMesh)

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

        const depthRenderPreq = ()=>{
            for(let i=0;i<kleeMeshes.length;i++){
                kleeMeshes[i].setShader(assets.getShader("klee/depth"))
            }
            floorMesh.setShader(assets.getShader("klee/depth"))
            bcubeMesh.setShader(assets.getShader("klee/depth"))
            cubeMesh.setShader(assets.getShader("klee/depth"))
            bfloorMesh.setShader(assets.getShader("klee/depth"))
        }

        const depthRenderRecv = ()=>{
            for(let i=0;i<kleeMeshes.length;i++){
                kleeMeshes[i].setShader(assets.getShader("klee/scene"))
            }
            floorMesh.setShader(assets.getShader("klee/floor"))
            cubeMesh.setShader(assets.getShader("klee/floor"))
            bcubeMesh.setShader(assets.getShader("klee/floor"))
            bfloorMesh.setShader(assets.getShader("klee/floor"))
        }
        const fpsSampleInterval = 10
        const renderFunc = ()=>{
            //FPS Update
            turns++;
            if(turns%fpsSampleInterval==0){
                let fps = 1000/(Date.now()-last)*fpsSampleInterval;
                last = Date.now()
                AriaPageIndicator.getInstance().updateFPS(fps)
                turns = 0;
            }
            //Shadow baking
            shadowFramebuf.bind()
            clearScene(gl)
            kleeScene.renderLightDepthMapS(0)
            shadowFramebuf.unbind()

            //Depth
            depthFramebuf.bind()
            depthRenderPreq()
            clearScene(gl)
            kleeScene.render()
            depthRenderRecv()
            depthFramebuf.unbind()

            //Original
            orgFramebuf.bind()
            clearScene(gl)
            kleeScene.render()
            orgFramebuf.unbind()

            //FP
            mainFramebuf.bind()
            clearScene(gl)
            mainScene.render()
            mainFramebuf.unbind()
            
            //Blurring Gaussian H
            ghFramebuf.bind()
            clearScene(gl)
            ghScene.render()
            ghFramebuf.unbind()

            //Blurring Gaussian V
            gvFramebuf.bind()
            clearScene(gl)
            gvScene.render()
            gvFramebuf.unbind()

            //Mix
            mxFramebuf.bind()
            clearScene(gl)
            mxScene.render()
            mxFramebuf.unbind()

            //FXAA
            clearScene(gl)
            spScene.render()

        }

        this.renderEnt = renderFunc
    
    }
    
    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }
}