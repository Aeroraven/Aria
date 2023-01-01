
import { mat4,mat3 } from "../node_modules/gl-matrix-ts/dist/index";
import { AriaCamera } from "./core/aria-camera"
import { AriaFramebuffer, AriaFramebufferOption } from "./core/aria-framebuffer";
import {initShaderProgram, loadFile, loadImage,createTexture } from "./core/aria-base"
import { AriaShader } from "./core/aria-shader";
import { AriaBufferMap } from "./core/aria-buffer-map";
import { AriaTextureMap } from "./core/aria-texture-map";
import { AriaComRectangle } from "./components/legacy/aria-com-rectangle";
import { AriaComAfrican } from "./components/legacy/aria-com-african";
import { AriaComSimLightBox } from "./components/legacy/aria-com-sim-light-box";
import { AriaComSkybox } from "./components/legacy/aria-com-skybox";
import { AriaComAfricanInstancing } from "./components/legacy/aria-com-african-instancing";
import { AriaComFloor } from "./components/legacy/aria-com-floor";
import { AriaAssetLoader } from "./core/aria-asset-loader";
import { AriaComMesh, AriaComMeshTextureType } from "./components/core/aria-com-mesh";
import { AriaComBuffers } from "./components/core/aria-com-buffers";
import { AriaComCube } from "./components/geometry/aria-com-cube";
import { AriaComScene } from "./components/core/aria-com-mesh-composite";
import { AriaComTexture } from "./components/core/aria-com-texture";
import { AriaComRect } from "./components/geometry/aria-com-rect";
import { AriaComLightSet } from "./components/light/aria-com-light-set";
import { AriaComSimplePBR } from "./components/material/aria-com-simple-pbr";

function clearScene(gl:WebGL2RenderingContext){ 
    //Clear
    gl.clearColor(0,0,0,1);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
} 


async function main(){
    const camera = new AriaCamera()
    const canvas = <HTMLCanvasElement>(document.getElementById("webgl_displayer"));
    canvas.width = window.innerWidth
    canvas.height = window.innerHeight
    const gl = <WebGL2RenderingContext>canvas.getContext("webgl2", { stencil: true });

    //Load Asset
    const assets = await AriaAssetLoader.getInstance(gl)

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

    //Skybox Shader
    const skyCom = new AriaComSkybox(gl)
    await skyCom.loadTexFromFolder("./skybox/")
    skyCom.initBuffer()

    //FrameBuffer
    const framebufOpt = AriaFramebufferOption.create().setHdr(true)
    const framebuf = new AriaFramebuffer(gl,framebufOpt)
    assets.addTexture("tex3",framebuf.tex)
    assets.addTexture("texSky",skyCom.cubeTex)

    //Scene
    const mainScene = (<AriaComScene>AriaComScene.create(gl))
    const postScene = (<AriaComScene>AriaComScene.create(gl))

    //Light
    const pointLight = (<AriaComLightSet>AriaComLightSet.create(gl))
        .addLight([1,1,0],[15,0,0])

    //Material
    const pbrMaterial = (<AriaComSimplePBR>AriaComSimplePBR.create(gl))
        .setAlbedo(1.0,0.0,0.0)
        .setMetallic(1.0)
        .setRoughness(0.3)
        .setAO(1.0)
    
    //Light box
    const lightGeometry = (<AriaComCube>AriaComCube.create(gl))
        .scale(0.2)
        .translate(1.0,1.0,3.0)
    const lightBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
        .addGeometry(lightGeometry)
    const lightMesh = (<AriaComMesh>AriaComMesh.create(gl))
        .setCamera(camera)
        .setShader(assets.getShader("light"))
        .setBuffer(lightBuffer)
    mainScene.addObject(lightMesh)

    //Boxes
    for(let i=0;i<5;i++){
        let rg = 3
        const boxTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("wall/diffuse"))
        const boxGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(0.5)
            .rotateX(Math.random()*2.0*Math.PI)
            .rotateY(Math.random()*2.0*Math.PI)
            .rotateZ(Math.random()*2.0*Math.PI)
            .translate(Math.random()*rg*2-rg,Math.random()*rg*2-rg,Math.random()*rg*2-rg)
        const boxBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(boxGeometry)
        const boxMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("plaincube-pbr"))
            .setBuffer(boxBuffer)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,boxTexture)
            .setLight(pointLight)
            .setMaterial(pbrMaterial)
        mainScene.addObject(boxMesh)
    }
    
    //Post Tex
    const postTex = (<AriaComTexture>AriaComTexture.create(gl))
        .setTex(assets.getTexture("tex3"))
    const postRect = (<AriaComRect>AriaComRect.create(gl))
    const postBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
        .addGeometry(postRect)
    const postMesh =   (<AriaComMesh>AriaComMesh.create(gl))
        .setCamera(camera)
        .setShader(assets.getShader("postproc"))
        .setBuffer(postBuffer)
        .setTexture(AriaComMeshTextureType.acmtDiffuse,postTex)
    postScene.addObject(postMesh)

    //Render
    function render(){
        //First pass
        framebuf.bind()
        clearScene(gl)
        mainScene.render()
        framebuf.unbind()

        //Postprocessing
        clearScene(gl)
        postScene.render()
        requestAnimationFrame(render)
    }
    requestAnimationFrame(render)
}



main()

