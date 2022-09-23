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

export class AriaStageGLTF extends AriaStage{
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
        camera.camPos[1] = 80
        camera.camPos[2] = 100

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

        //Main Scene:
        const lightProjPos = [-15,60,-20]
        const stageLight = (<AriaComLightSet>AriaComLightSet.create(gl))
            .addDirectionalLight([1,0,0],[1,1,1],lightProjPos)
        
        const kleeLoader = new AriaLoaderGLTF(gl)
        await kleeLoader.loadModel("./models/klee2/untitled.gltf")
        const kleeScene = (<AriaComScene>AriaComScene.create(gl))
        const kleeModel = (<AriaComModel>AriaComModel.create(gl))
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

        const renderFunc = ()=>{
            //FPS Update
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
            clearScene(gl)
            kleeScene.render()
        }

        this.renderEnt = renderFunc
    
    }
    
    public async render(gl: WebGL2RenderingContext): Promise<any> {
        this.renderEnt()
    }
}