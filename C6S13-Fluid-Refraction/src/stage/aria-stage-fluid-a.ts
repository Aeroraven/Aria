import { AriaComBuffers } from "../components/core/aria-com-buffers";
import { AriaComMesh, AriaComMeshTextureType } from "../components/core/aria-com-mesh";
import { AriaComModel, AriaComScene } from "../components/core/aria-com-mesh-composite";
import { AriaComTexture } from "../components/core/aria-com-texture";
import { AriaComTimestamp } from "../components/effects/aria-com-timestamp";
import { AriaComCube } from "../components/geometry/aria-com-cube";
import { AriaComFluidSurface } from "../components/geometry/aria-com-fluid-surface";
import { AriaComGeometryAttribEnum } from "../components/geometry/aria-com-geometry";
import { AriaComMeshGeometry } from "../components/geometry/aria-com-mesh-geometry";
import { AriaComRect } from "../components/geometry/aria-com-rect";
import { AriaComSineSurface } from "../components/geometry/aria-com-sine-surface";
import { AriaComLightSet } from "../components/light/aria-com-light-set";
import { AriaComSimplePBR } from "../components/material/aria-com-simple-pbr";
import { AriaComPostprocessPass } from "../components/postprocess/aria-com-postprocess-pass";
import { AriaAssetLoader } from "../core/aria-asset-loader";
import { AriaCamera } from "../core/aria-camera";
import { AriaFramebuffer, AriaFramebufferOption } from "../core/aria-framebuffer";
import { AriaPageIndicator } from "../core/aria-page-indicator";
import { AriaLoaderGLTF } from "../loaders/aria-loader-gltf";
import { AriaStage } from "./aria-stage-base";

export class AriaStageFluidA extends AriaStage{
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
        camera.registerInteractionEvent();
        camera.camFront = new Float32Array([-0.24152889847755432, -0.8619058132171631, -0.44584986567497253]);
        camera.camLookAt = new Float32Array([0.8792697787284851, -0.41054654121398926, 0.24152882397174835, 0, 0, 0.5070682764053345, 0.8619058728218079, 0, -0.47632408142089844, -0.7578478455543518, 0.44584980607032776, 0, 1.6368223428726196, 3.1248891353607178, -7.541825294494629, 1])
        camera.camPos = new Float32Array( [1.6652722358703613, 4.915811538696289, 6.510369777679443])
        camera.disableInteraction()
        ;(<any>window).cam = camera

        //Env Map Buffer
        const fpMarchOpt = AriaFramebufferOption.create().setHdr(true).setScaler(0.9)
        const fpFramebufferOpt = AriaFramebufferOption.create().setHdr(true).setScaler(1)
        const fpEnvPos = new AriaFramebuffer(gl,fpFramebufferOpt)
        const fpEnvDiffuse = new AriaFramebuffer(gl,fpFramebufferOpt)
        const fpScene = new AriaFramebuffer(gl,fpMarchOpt)
        assets.addTexture("fluida/envpos",fpEnvPos.tex)
        assets.addTexture("fluida/envcol",fpEnvDiffuse.tex)
        assets.addTexture("fluida/lowq",fpScene.tex)

        //Scene
        const mainScene = (<AriaComScene>AriaComScene.create(gl))
        const objectScene = (<AriaComScene>AriaComScene.create(gl))

        //Light
        const dirLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([0,-1,0],[30,30,30])

        //Material
        const pbrMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
            .setAlbedo(0.0,0.15,1.0)
            .setMetallic(0.9)
            .setRoughness(0.2)
            .setAO(1.0)

        //Env Textures
        const texEnvPos = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("fluida/envpos"))
        const texEnvColor = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("fluida/envcol"))
        const finalTex = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("fluida/lowq"))

        //Fluid Surface
        let d = 0.005
        let c = 0.1
        let t = 0.01
        let mu = 0.05
        
        const postOrgRect = (<AriaComFluidSurface>AriaComFluidSurface.create(gl))
            .translate(-5,-4,-2)
            .scale(20)
            .setDensity(80)
            .setFluidParams(c,d,t,mu)
            .initFluid()

        const postOrgBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(postOrgRect)
        const postOrgMesh =  (<AriaComMesh>AriaComMesh.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("fluid/surface"))
            .setMaterial(pbrMaterial)
            .setBuffer(postOrgBuffer)
            .setLight(dirLight)
            .setTexture(AriaComMeshTextureType.acmtSpecular,texEnvPos)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,texEnvColor)
        mainScene.addObject(postOrgMesh)


        //Object
        const kleeLoader = new AriaLoaderGLTF(gl)
        const kleeModel = (<AriaComModel>AriaComModel.create(gl))
        const kleeMeshes:AriaComMesh[] = []

        await kleeLoader.loadModel("./models/klee2/untitled.gltf")

        for(let i=0;i<kleeLoader.getTotalMeshes();i++){
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
                .setShader(assets.getShader("fluid/scene"))
                .setBuffer(kleeBuffer)
                .setTexture(AriaComMeshTextureType.acmtDiffuse,kleeBaseTex)
                .setDepthShader(assets.getShader("klee/shadow"))
            kleeModel.addObject(kleeMesh)
            kleeMeshes.push(kleeMesh)
        }
        const floorGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(200)
            .translate(0,-7,0)
            .setTopOnly(true)
        const floorBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(floorGeometry)
        const floorMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setBuffer(floorBuffer)
            .setCamera(camera)
            .setShader(assets.getShader("fluid/floor"))
            .setDepthShader(assets.getShader("klee/shadow"))
        objectScene.addObject(kleeModel)
        objectScene.addObject(floorMesh)
        
        //Post Pass
        const fxaaPass = (<AriaComPostprocessPass>AriaComPostprocessPass.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("fxaa-all"))
            .setTexture(AriaComMeshTextureType.acmtDiffuse,finalTex)

        //Done
        AriaPageIndicator.getInstance().done()
        let last = Date.now()
        let turns = 0

        function clearScene(gl:WebGL2RenderingContext){ 
            gl.clearColor(1,1,1,1);
            gl.enable(gl.DEPTH_TEST);
            gl.depthFunc(gl.LEQUAL);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
        } 

        //Disturbance
        postOrgRect.addPointInterference(40,40,0.01)
        postOrgRect.addPointInterference(20,60,0.01)

        //Shader Changes
        const setDepthRender = ()=>{
            floorMesh.setShader(assets.getShader("fluid/depth"))
            kleeMeshes.forEach((el)=>{
                el.setShader(assets.getShader("fluid/kdepth"))
            })
        }

        const setDepthRenderRecover = ()=>{
            floorMesh.setShader(assets.getShader("fluid/floor"))
            kleeMeshes.forEach((el)=>{
                el.setShader(assets.getShader("fluid/scene"))
            })
        }

        //Render Call
        const renderCall = ()=>{
            turns++;
            if(turns%5==0){
                let fps = 1000/(Date.now()-last)*5;
                last = Date.now()
                AriaPageIndicator.getInstance().updateFPS(fps)
                turns = 0;
            }

            //Render

            //Pass 1 - Env Position
            fpEnvPos.bind()
            clearScene(gl)
            setDepthRender()
            objectScene.render()
            setDepthRenderRecover()
            fpEnvPos.unbind()

            //Pass 2 - Env Color
            fpEnvDiffuse.bind()
            clearScene(gl)
            objectScene.render()
            fpEnvDiffuse.unbind()

            //Pass 3 - Overall Scene
            fpScene.bind()
            clearScene(gl)
            mainScene.render()
            objectScene.render()
            fpScene.unbind()

            //Pass 4 - Final
            clearScene(gl)
            fxaaPass.render()
        }
        this.renderEnt = renderCall
    }

    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }

}