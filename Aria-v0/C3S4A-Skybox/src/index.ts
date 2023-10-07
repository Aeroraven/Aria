
import { mat4,mat3 } from "../node_modules/gl-matrix-ts/dist/index";
import { AriaCamera } from "./core/aria-camera"
import { AriaFramebuffer } from "./core/aria-framebuffer";
import {initShaderProgram, loadFile, loadImage,createTexture } from "./core/aria-base"
import { AriaShader } from "./core/aria-shader";
import { AriaBufferMap } from "./core/aria-buffer-map";
import { AriaTextureMap } from "./core/aria-texture-map";
import { AriaComRectangle } from "./components/aria-com-rectangle";
import { AriaComAfrican } from "./components/aria-com-african";
import { AriaComSimLightBox } from "./components/aria-com-sim-light-box";
import { AriaComSkybox } from "./components/aria-com-skybox";

function clearScene(gl:WebGL2RenderingContext){
    //Clear
    gl.clearColor(0,0,0,1);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
} 


async function loadTex(gl:WebGL2RenderingContext):Promise<AriaTextureMap>{
    const texImg = await loadImage("./african_diffuse.jpg")
    const tex = createTexture(gl,texImg);
    const specTexImg = await loadImage("./african_specular.jpg")
    const specTex = createTexture(gl,specTexImg)
    const postTexImg = await loadImage("./star.jpg")
    const postTex = createTexture(gl,postTexImg)

    const r = new AriaTextureMap()
    r.set("tex1",tex)
    r.set("tex2",specTex)
    r.set("tex3",postTex)

    return r
}

async function main(){
    const camera = new AriaCamera()
    const canvas = <HTMLCanvasElement>(document.getElementById("webgl_displayer"));
    canvas.width = window.innerWidth
    canvas.height = window.innerHeight
    const gl = <WebGL2RenderingContext>canvas.getContext("webgl2", { stencil: true });

    gl.viewport(0,0,window.innerWidth,window.innerHeight)
    gl.disable(gl.CULL_FACE)

    //Stencil Test
    gl.enable(gl.STENCIL_TEST)
    gl.stencilOp(gl.KEEP,gl.KEEP,gl.REPLACE)
    gl.stencilFunc(gl.ALWAYS,1,0xff)
    gl.stencilMask(0xff)
    //Mesh
    const african = await loadFile("./african.obj")

    //Camera
    camera.registerInteractionEvent()

    //Texture
    
    //Shader
    const vSrc = await loadFile("./african-vertex.glsl");
    const fSrc = await loadFile("./african-fragment.glsl");
    const africanShader = new AriaShader(gl,vSrc,fSrc);
    const africanBuffer = AriaComAfrican.initBuffer(gl,african);

    //Light Shader
    const vlsrc = await loadFile("./light-vertex.glsl");
    const flsrc = await loadFile("./light-fragment.glsl");
    const lightShaderProg = new AriaShader(gl,vlsrc,flsrc);
    const lightbuf = AriaComSimLightBox.initBuffer(gl)

    //Outline Shader
    const vosrc = await loadFile("./african-outline-vertex.glsl");
    const fosrc = await loadFile("./african-outline-fragment.glsl");
    const outlineShaderProg = new AriaShader(gl,vosrc,fosrc);

    //Post Shader
    const vpsrc = await loadFile("./post-vertex.glsl")
    const fpsrc = await loadFile("./post-fragment.glsl")
    const postShader = new AriaShader(gl,vpsrc,fpsrc)
    const postbuf = AriaComRectangle.initBuffer(gl)


    //Skybox Shader
    const vsksrc = await loadFile("./skybox-vertex.glsl")
    const fsksrc = await loadFile("./skybox-fragment.glsl")
    const skyShader = new AriaShader(gl,vsksrc,fsksrc)
    const skyCom = new AriaComSkybox(gl)

    await skyCom.loadTexFromFolder("./skybox/")
    skyCom.initBuffer()
    

    //Texture
    const texStruct = await loadTex(gl)

    //FrameBuffer
    const framebuf = new AriaFramebuffer(gl)
    texStruct.set("tex3",framebuf.tex)

    //Render
    function render(){
        framebuf.bind()
        clearScene(gl)
        skyCom.draw(skyShader,camera)
        AriaComAfrican.draw(gl,africanShader,africanBuffer,texStruct,camera)
        //drawAfricanOutline(gl,outProgInfo,buf,camera)
        framebuf.unbind()
        clearScene(gl)
        AriaComRectangle.draw(gl,postShader,postbuf,texStruct,camera)
        
        //drawLight(gl,lightProgInfo,lightbuf,camera)
        requestAnimationFrame(render)
    }
    requestAnimationFrame(render)
}

main()