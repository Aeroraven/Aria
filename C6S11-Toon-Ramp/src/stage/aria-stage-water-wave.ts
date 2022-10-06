import { AriaComBuffers } from "../components/core/aria-com-buffers";
import { AriaComMesh, AriaComMeshTextureType } from "../components/core/aria-com-mesh";
import { AriaComModel, AriaComScene } from "../components/core/aria-com-mesh-composite";
import { AriaComTexture } from "../components/core/aria-com-texture";
import { AriaComTimestamp } from "../components/effects/aria-com-timestamp";
import { AriaComCube } from "../components/geometry/aria-com-cube";
import { AriaComGeometryAttribEnum } from "../components/geometry/aria-com-geometry";
import { AriaComMeshGeometry } from "../components/geometry/aria-com-mesh-geometry";
import { AriaComRect } from "../components/geometry/aria-com-rect";
import { AriaComLightSet } from "../components/light/aria-com-light-set";
import { AriaComSimplePBR } from "../components/material/aria-com-simple-pbr";
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaFramebufferOption, AriaFramebuffer } from "../core/aria-framebuffer";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaLoaderGLTF } from "../loaders/aria-loader-gltf";
import { AriaStage } from "./aria-stage-base";

export class AriaStageWaterwave extends AriaStage{
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

        //==============Texture Rendering===================
        //Shadow Frame
        const shadowFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(4)
        const shadowFramebuf = new AriaFramebuffer(gl,shadowFramebufOpt)
        assets.addTexture("klee/shadow", shadowFramebuf.tex)

        const kleeFramebufOpt = AriaFramebufferOption.create().setHdr(true).setScaler(4)
        const kleeFramebuf = new AriaFramebuffer(gl,kleeFramebufOpt)
        assets.addTexture("klee/ftex", kleeFramebuf.tex)

        //Main Scene:
        const lightProjPos = [-50,70,0]//[-30,100,-20]
        const kleeScene = (<AriaComScene>AriaComScene.create(gl))
        const kleeModel = (<AriaComModel>AriaComModel.create(gl))
        const stageLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([1,-1,0],[10,10,10],lightProjPos)

        const floorShadowTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/shadow"))
        const kleeTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("klee/ftex"))
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
            .setShader(assets.getShader("klee/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,floorShadowTex)
            .setLightProjPos(lightProjPos)
        kleeScene.addObject(floorMesh)


        //Klee
        const kleeLoader = new AriaLoaderGLTF(gl)
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
                .setShader(assets.getShader("klee/scene"))
                .setBuffer(kleeBuffer)
                .setTexture(AriaComMeshTextureType.acmtDiffuse,kleeBaseTex)
                .setTexture(AriaComMeshTextureType.acmtSpecular,kleeShadowTex)
                .setLight(stageLight)
                .setDepthShader(assets.getShader("klee/shadow"))
                .setLightProjPos(lightProjPos)
            kleeModel.addObject(kleeMesh)
        }
        kleeScene.addObject(kleeModel)

        //===========End of Scene Def=================

        //Scene
        const mainScene = (<AriaComScene>AriaComScene.create(gl))

        //Animation Component
        const shaderTimer  = (<AriaComTimestamp>AriaComTimestamp.create(gl))

        //Mesh
        const postOrgRect = (<AriaComRect>AriaComRect.create(gl))
        const postOrgBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(postOrgRect)
        const postOrgMesh =   (<AriaComMesh>AriaComMesh.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("water-wave"))
            .setBuffer(postOrgBuffer)
            .addAttachments("timer",shaderTimer)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,kleeTex)
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

            //Shadow baking
            shadowFramebuf.bind()
            clearScene(gl)
            kleeScene.renderLightDepthMapS(0)
            shadowFramebuf.unbind()

            //Main Render
            kleeFramebuf.bind()
            clearScene(gl)
            kleeScene.render()
            kleeFramebuf.unbind()

            clearScene(gl)
            mainScene.render()
        }
        this.renderEnt = renderCall
    }

    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }

}