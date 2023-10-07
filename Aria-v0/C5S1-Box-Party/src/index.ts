
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

    //Post Shader
    const postbuf = AriaComRectangle.initBuffer(gl)

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

    //Boxes
    for(let i=0;i<10;i++){
        const boxTexture = (<AriaComTexture>AriaComTexture.create(gl))
            .setTex(assets.getTexture("wall/diffuse"))
        const boxGeometry = (<AriaComCube>AriaComCube.create(gl))
            .scale(1.0)
            .rotateX(Math.random()*2.0*Math.PI)
            .rotateY(Math.random()*2.0*Math.PI)
            .rotateZ(Math.random()*2.0*Math.PI)
            .translate(Math.random()*6-3,Math.random()*6-3,Math.random()*6-3)
        const boxBuffer = (<AriaComBuffers>AriaComBuffers.create(gl))
            .addGeometry(boxGeometry)
        const boxMesh = (<AriaComMesh>AriaComMesh.create(gl))
            .setCamera(camera)
            .setShader(assets.getShader("plaincube"))
            .setBuffer(boxBuffer)
            .setTexture(AriaComMeshTextureType.acmtDiffuse,boxTexture)
        mainScene.addObject(boxMesh)
    }
    
    //Render
    function render(){
        //First pass
        framebuf.bind()
        clearScene(gl)
        mainScene.render()
        framebuf.unbind()

        //Postprocessing
        clearScene(gl)
        AriaComRectangle.draw(gl,assets.getShader("postproc"),postbuf,assets.getTexStruct(),camera)
        requestAnimationFrame(render)
    }
    requestAnimationFrame(render)
}



main()

