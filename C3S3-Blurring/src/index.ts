import axios from "../node_modules/axios/index";
import { mat4,mat3 } from "../node_modules/gl-matrix-ts/dist/index";
import {AriaCamera} from "./aria-camera"
import { AriaFramebuffer } from "./aria-framebuffer";
import { AriaLoadedMesh } from "./aria-loaded-mesh";

interface AriaShaderProgramStructure{
    program:WebGLProgram,
    attribLoc:{
        aTex:number,
        aVert:number,
        aColor:number,
        aNorm:number
    },
    uniformLoc:{
        projMat:WebGLUniformLocation,
        modelMat:WebGLUniformLocation,
        modelItMat:WebGLUniformLocation,
        texSampler:WebGLUniformLocation,
        specTexSampler:WebGLUniformLocation,
        cameraPos:WebGLUniformLocation,
        cameraFront:WebGLUniformLocation,
    }
}

interface AriaLightShaderProgramStructure{
    program:WebGLProgram,
    attribLoc:{
        aVert:number
    },
    uniformLoc:{
        projMat:WebGLUniformLocation,
        modelMat:WebGLUniformLocation
    }
}

interface AriaOutlineShaderProgramStructure{
    program:WebGLProgram,
    attribLoc:{
        aVert:number,
    },
    uniformLoc:{
        projMat:WebGLUniformLocation,
        modelMat:WebGLUniformLocation
    }
}

interface AriaPostShaderProgramStructure{
    program:WebGLProgram,
    attribLoc:{
        aVert:number,
        aTex:number
    },
    uniformLoc:{
        projMat:WebGLUniformLocation,
        texSampler:WebGLUniformLocation
    }
}

interface AriaPosBufferStructure{
    nums:number
    pos:WebGLBuffer,
    col:WebGLBuffer,
    tex:WebGLBuffer,
    norm:WebGLBuffer
    ele:WebGLBuffer,
}

interface AriaRectBufferStructure{
    pos:WebGLBuffer,
    ele:WebGLBuffer,
    tex:WebGLBuffer
}

interface AriaLightPosBufferStructure{
    pos:WebGLBuffer,
    ele:WebGLBuffer
}

interface AriaTextureStructure{
    tex1:WebGLTexture,
    tex2:WebGLTexture,
    tex3:WebGLTexture
}

function loadShader(gl:WebGL2RenderingContext,type:number,source:string):WebGLShader|null{
    const shader = <WebGLShader>gl.createShader(type);
    gl.shaderSource(shader,source);
    gl.compileShader(shader);
    if(!gl.getShaderParameter(shader,gl.COMPILE_STATUS)){
        alert("Shader error:"+gl.getShaderInfoLog(shader));
        gl.deleteShader(shader)
        return null;
    }
    return shader;
}

function initShaderProgram(gl:WebGL2RenderingContext,vsrc:string,fsrc:string):WebGLProgram|null{
    const vs = loadShader(gl,gl.VERTEX_SHADER,vsrc)
    const fs = loadShader(gl,gl.FRAGMENT_SHADER,fsrc)
    if(vs==null||fs==null){
        return null
    }else{
        const vsr = <WebGLShader>vs;
        const fsr = <WebGLShader>fs;
        const shaderProg = <WebGLProgram>gl.createProgram()
        gl.attachShader(shaderProg,vsr);
        gl.attachShader(shaderProg,fsr);
        gl.linkProgram(shaderProg)
        if(!gl.getProgramParameter(shaderProg,gl.LINK_STATUS)){
            alert("ShaderProg error:"+gl.getProgramInfoLog(shaderProg))
            return null;
        }
        return shaderProg
    }
}

async function loadFile(path:string):Promise<string>{
    const resp = await axios.get(path);
    return <string>resp.data;
}

function initRectangleBuffer(gl:WebGL2RenderingContext):AriaRectBufferStructure{
    const posBuffer = <WebGLBuffer>gl.createBuffer();
    let vertices = [
        -1.0, -1.0,  -1.0,
         1.0, -1.0,  -1.0,
         1.0,  1.0,  -1.0,
        -1.0,  1.0,  -1.0,
    ];
    gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)

    const texBuffer = <WebGLBuffer>gl.createBuffer();
    let texLst = [0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0]
    gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(texLst),gl.STATIC_DRAW)

    const eleBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
    const eleIndices = [
        0,  1,  2,      0,  2,  3,   
    ];
    
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
    return <AriaRectBufferStructure>{
        pos:posBuffer,
        tex:texBuffer,
        ele:eleBuffer
    }
}


function initLightSrcBuffer(gl:WebGL2RenderingContext):AriaLightPosBufferStructure{
    const posBuffer = <WebGLBuffer>gl.createBuffer();
    let vertices = [
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
        -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,
         1.0,  1.0, -1.0,
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0
    ];
    for(let i=0;i<vertices.length;i+=3){
        vertices[i] = vertices[i] * 0.2 + 1.5
        vertices[i+1] = vertices[i+1] * 0.2 + 0.5
        vertices[i+2] = vertices[i+2] * 0.2 + 2.0
    }
    gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)

    const eleBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
    const eleIndices = [
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23,   // left
    ];
    
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
    return <AriaLightPosBufferStructure>{
        pos:posBuffer,
        ele:eleBuffer
    }

}

function initBufferAfrican(gl:WebGL2RenderingContext,meshSrc:string):AriaPosBufferStructure{
    const mesh = new AriaLoadedMesh()
    mesh.loadFromWavefront(meshSrc)
    const meshdata = mesh.exportData()

    const posBuffer = <WebGLBuffer>gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.v),gl.STATIC_DRAW)

    const colBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ARRAY_BUFFER,colBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.vc),gl.STATIC_DRAW)

    const normBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.vn),gl.STATIC_DRAW)

    const texBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.vt),gl.STATIC_DRAW)

    const eleBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(meshdata.f),gl.STATIC_DRAW)
    return <AriaPosBufferStructure>{
        nums:meshdata.v.length/3,
        pos:posBuffer,
        col:colBuffer,
        norm:normBuffer,
        tex:texBuffer,
        ele:eleBuffer
    }
}

function initBuffer(gl:WebGL2RenderingContext):AriaPosBufferStructure{
    const posBuffer = <WebGLBuffer>gl.createBuffer();
    let vertices = [
        // Front face
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
      
        // Back face
        -1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0, -1.0, -1.0,
      
        // Top face
        -1.0,  1.0, -1.0,
        -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,
         1.0,  1.0, -1.0,
      
        // Bottom face
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,
      
        // Right face
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,
      
        // Left face
        -1.0, -1.0, -1.0,
        -1.0, -1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0
    ];
    gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)

    //Color
    const colBuffer = <WebGLBuffer>gl.createBuffer()
    let preColors = [
        [1.0,  1.0,  1.0,  1.0],    // Front face: white
        [1.0,  0.0,  0.0,  1.0],    // Back face: red
        [0.0,  1.0,  0.0,  1.0],    // Top face: green
        [0.0,  0.0,  1.0,  1.0],    // Bottom face: blue
        [1.0,  1.0,  0.0,  1.0],    // Right face: yellow
        [1.0,  0.0,  1.0,  1.0]     // Left face: purple
    ];
    let colList:number[] = []
    for(let i=0;i<6;i++){
        for(let j=0;j<4;j++){
            colList = colList.concat(preColors[i])
        }
    }
    gl.bindBuffer(gl.ARRAY_BUFFER,colBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(colList),gl.STATIC_DRAW)

    //Normal
    const normBuffer = <WebGLBuffer>gl.createBuffer()
    let preNorms = [
        [0,0,1],
        [0,0,-1],
        [0,1,0],
        [0,-1,0],
        [1,0,0],
        [-1,0,0]
    ]
    let normList:number[] = []
    for(let i=0;i<6;i++){
        for(let j=0;j<4;j++){
            normList = normList.concat(preNorms[i])
        }
    }
    gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(normList),gl.STATIC_DRAW)

    //Texture
    const texBuffer = <WebGLBuffer>gl.createBuffer()
    let preTex = [0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0]
    let texList:number[] = []
    for(let i=0;i<6;i++){
        texList = texList.concat(preTex)
    }
    gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(texList),gl.STATIC_DRAW)


    const eleBuffer = <WebGLBuffer>gl.createBuffer()
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
    const eleIndices = [
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23,   // left
    ];
    
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
    return <AriaPosBufferStructure>{
        pos:posBuffer,
        col:colBuffer,
        tex:texBuffer,
        norm:normBuffer,
        ele:eleBuffer
    }
}

function clearScene(gl:WebGL2RenderingContext){
    //Clear
    gl.clearColor(0,0,0,1);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
} 

function drawLight(gl:WebGL2RenderingContext,progInfo:AriaLightShaderProgramStructure,
    buffer:AriaLightPosBufferStructure,camera:AriaCamera){
    
    //Projection 
    const modelview = camera.getLookAt()
    const projectionMatrix = camera.getPerspective()

    //Attrib
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.pos);
    gl.vertexAttribPointer(progInfo.attribLoc.aVert,3,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aVert)

    //Use Shader
    gl.useProgram(progInfo.program)
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.ele)

    //Uniforms
    gl.uniformMatrix4fv(progInfo.uniformLoc.modelMat,false,modelview)
    gl.uniformMatrix4fv(progInfo.uniformLoc.projMat,false,projectionMatrix)

    gl.drawElements(gl.TRIANGLES,36,gl.UNSIGNED_SHORT,0)
}


function drawRect(gl:WebGL2RenderingContext,progInfo:AriaPostShaderProgramStructure,
    buffer:AriaRectBufferStructure,tx:AriaTextureStructure,camera:AriaCamera){
    
    //Projection 
    const projectionMatrix = camera.getViewportOrtho()

    //Attrib
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.pos);
    gl.vertexAttribPointer(progInfo.attribLoc.aVert,3,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aVert)

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.tex);
    gl.vertexAttribPointer(progInfo.attribLoc.aTex,2,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aTex)

    //Use Shader
    gl.useProgram(progInfo.program)
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.ele)
    gl.activeTexture(gl.TEXTURE0)
    gl.bindTexture(gl.TEXTURE_2D, tx.tex3)
    gl.uniform1i(progInfo.uniformLoc.texSampler,0);


    //Uniforms
    gl.uniformMatrix4fv(progInfo.uniformLoc.projMat,false,projectionMatrix)

    gl.drawElements(gl.TRIANGLES,6,gl.UNSIGNED_SHORT,0)
}


function drawAfricanOutline(gl:WebGL2RenderingContext,progInfo:AriaOutlineShaderProgramStructure,
    buffer:AriaPosBufferStructure,camera:AriaCamera){
    
    gl.useProgram(progInfo.program)
    gl.stencilOp(gl.KEEP,gl.KEEP,gl.KEEP)
    gl.stencilFunc(gl.NOTEQUAL,0x01,0xff)
    gl.stencilMask(0x00)
    gl.disable(gl.DEPTH_TEST)
    //Projection 
    const modelview = camera.getLookAt()
    const projectionMatrix = camera.getPerspective()

    //Attrib
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.pos);
    gl.vertexAttribPointer(progInfo.attribLoc.aVert,3,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aVert)

    //Use Shader
    
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.ele)

    //Uniforms
    gl.uniformMatrix4fv(progInfo.uniformLoc.modelMat,false,modelview)
    gl.uniformMatrix4fv(progInfo.uniformLoc.projMat,false,projectionMatrix)

    gl.drawElements(gl.TRIANGLES,buffer.nums,gl.UNSIGNED_SHORT,0)

    gl.stencilMask(0xff)
    gl.enable(gl.DEPTH_TEST)
}

function drawScene(gl:WebGL2RenderingContext,progInfo:AriaShaderProgramStructure,
    buffer:AriaPosBufferStructure,tx:AriaTextureStructure,delta:number,camera:AriaCamera){
    
    gl.enable(gl.DEPTH_TEST)
    gl.stencilOp(gl.KEEP,gl.KEEP,gl.REPLACE)
    gl.stencilFunc(gl.ALWAYS,1,0xff)
    gl.stencilMask(0xff)
    

    //Projection
    const modelview = camera.getLookAt()
    const projectionMatrix = camera.getPerspective()
    const modelIT2 = mat4.create()
    const modelIT = mat4.create()
    mat4.invert(modelIT2,modelview)
    mat4.transpose(modelIT,modelIT2)

    //Attrib
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.pos);
    gl.vertexAttribPointer(progInfo.attribLoc.aVert,3,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aVert)

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.col);
    gl.vertexAttribPointer(progInfo.attribLoc.aColor,4,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aColor)

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.tex);
    gl.vertexAttribPointer(progInfo.attribLoc.aTex,2,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aTex)

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer.norm);
    gl.vertexAttribPointer(progInfo.attribLoc.aNorm,3,gl.FLOAT,false,0,0)
    gl.enableVertexAttribArray(progInfo.attribLoc.aNorm)

    //Use Shader
    gl.useProgram(progInfo.program)
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.ele)

    //Uniforms
    gl.uniform3fv(progInfo.uniformLoc.cameraPos,camera.getCamPosArray())
    gl.uniform3fv(progInfo.uniformLoc.cameraFront,camera.getCamFrontArray())
    gl.uniformMatrix4fv(progInfo.uniformLoc.modelMat,false,modelview)
    gl.uniformMatrix4fv(progInfo.uniformLoc.projMat,false,projectionMatrix)
    gl.uniformMatrix4fv(progInfo.uniformLoc.modelItMat,false,modelIT)

    gl.activeTexture(gl.TEXTURE0)
    gl.bindTexture(gl.TEXTURE_2D, tx.tex1)
    gl.uniform1i(progInfo.uniformLoc.texSampler,0);

    gl.activeTexture(gl.TEXTURE1)
    gl.bindTexture(gl.TEXTURE_2D, tx.tex2)
    gl.uniform1i(progInfo.uniformLoc.specTexSampler,0);

    //gl.drawArrays(gl.TRIANGLE_STRIP,0,4)
    gl.drawElements(gl.TRIANGLES,buffer.nums,gl.UNSIGNED_SHORT,0)

    
}

async function loadImage(path:string):Promise<HTMLImageElement>{
    const img = new Image();
    await new Promise((resolve,reject)=>{
        img.onload = ()=>{
            resolve(true)
        }
        img.src = path
    })
    return img
}

function createTexture(gl:WebGL2RenderingContext,img:HTMLImageElement):WebGLTexture|null{
    const tex = gl.createTexture()
    gl.bindTexture(gl.TEXTURE_2D,tex);
    gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE,img);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.bindTexture(gl.TEXTURE_2D,null)
    return tex;
}

async function loadMeshTest(){
    const wsrc = await loadFile("./african.obj")
    const mesh = new AriaLoadedMesh()
    mesh.loadFromWavefront(wsrc)
    const w = mesh.exportData()
    console.log(w)
}

async function main(){
    const camera = new AriaCamera()
    const canvas = <HTMLCanvasElement>(document.getElementById("webgl_displayer"));
    canvas.width = window.innerWidth
    canvas.height = window.innerHeight
    const gl = <WebGL2RenderingContext>canvas.getContext("webgl2", { stencil: true });

    //Stencil Test
    gl.enable(gl.STENCIL_TEST)
    gl.stencilOp(gl.KEEP,gl.KEEP,gl.REPLACE)
    gl.stencilFunc(gl.ALWAYS,1,0xff)
    gl.stencilMask(0xff)
    //Mesh
    const african = await loadFile("./african.obj")

    //Camera
    //camera.registerInteractionEvent()

    //Texture
    const texImg = await loadImage("./african_diffuse.jpg")
    const tex = createTexture(gl,texImg);
    const specTexImg = await loadImage("./african_specular.jpg")
    const specTex = createTexture(gl,specTexImg)
    const postTexImg = await loadImage("./star.jpg")
    const postTex = createTexture(gl,postTexImg)

    const texStruct = <AriaTextureStructure>{
        tex1:tex,
        tex2:specTex,
        tex3:postTex
    }
    //Shader
    const vSrc = await loadFile("./african-vertex.glsl");
    const fSrc = await loadFile("./african-fragment.glsl");
    const shaderProg = <WebGLProgram>initShaderProgram(gl,vSrc,fSrc);
    const progInfo = <AriaShaderProgramStructure>{
        program: shaderProg,
        attribLoc:{
            aVert: gl.getAttribLocation(shaderProg,"aVert"),
            aColor: gl.getAttribLocation(shaderProg,"aColor"),
            aTex: gl.getAttribLocation(shaderProg,"aTex"),
            aNorm: gl.getAttribLocation(shaderProg,"aNorm"),
        },
        uniformLoc:{
            projMat: gl.getUniformLocation(shaderProg,"uProj"),
            modelMat: gl.getUniformLocation(shaderProg,"uModel"),
            modelItMat: gl.getUniformLocation(shaderProg,"uModelInvTrans"),
            texSampler:gl.getUniformLocation(shaderProg,"uSampler"),
            cameraPos:gl.getUniformLocation(shaderProg,"uCamPos"),
            cameraFront:gl.getUniformLocation(shaderProg,"uCamFront"),
        }
    }
    const buf = initBufferAfrican(gl,african);

    //Light Shader
    const vlsrc = await loadFile("./light-vertex.glsl");
    const flsrc = await loadFile("./light-fragment.glsl");
    const lightShaderProg = <WebGLProgram>initShaderProgram(gl,vlsrc,flsrc);
    const lightProgInfo = <AriaLightShaderProgramStructure>{
        program:lightShaderProg,
        attribLoc:{
            aVert: gl.getAttribLocation(lightShaderProg,"aVert"),
        },
        uniformLoc:{
            projMat: gl.getUniformLocation(lightShaderProg,"uProj"),
            modelMat: gl.getUniformLocation(lightShaderProg,"uModel"),
        }
    }
    const lightbuf = initLightSrcBuffer(gl)

    //Outline Shader
    const vosrc = await loadFile("./african-outline-vertex.glsl");
    const fosrc = await loadFile("./african-outline-fragment.glsl");
    const outlineShaderProg = <WebGLProgram>initShaderProgram(gl,vosrc,fosrc);
    const outProgInfo = <AriaLightShaderProgramStructure>{
        program:outlineShaderProg,
        attribLoc:{
            aVert: gl.getAttribLocation(outlineShaderProg,"aVert"),
        },
        uniformLoc:{
            projMat: gl.getUniformLocation(outlineShaderProg,"uProj"),
            modelMat: gl.getUniformLocation(outlineShaderProg,"uModel"),
        }
    }

    //Post Shader
    const vpsrc = await loadFile("./post-vertex.glsl")
    const fpsrc = await loadFile("./post-fragment.glsl")
    const postShaderProg = <WebGLProgram>initShaderProgram(gl,vpsrc,fpsrc)
    const postProgInfo = <AriaPostShaderProgramStructure>{
        program:postShaderProg,
        attribLoc:{
            aVert:gl.getAttribLocation(postShaderProg,"aVert"),
            aTex:gl.getAttribLocation(postShaderProg,"aTex")
        },
        uniformLoc:{
            projMat: gl.getUniformLocation(postShaderProg,"uProj"),
            texSampler: gl.getUniformLocation(postShaderProg,"uSampler")
        }
    }
    const postbuf = initRectangleBuffer(gl)

    //FrameBuffer
    const framebuf = new AriaFramebuffer(gl)
    texStruct.tex3 = framebuf.tex

    //Render
    let delta = 0
    function render(){
        
        framebuf.bind()
        clearScene(gl)
        drawScene(gl,progInfo,buf,texStruct,delta,camera)
        //drawAfricanOutline(gl,outProgInfo,buf,camera)

        framebuf.unbind()
        clearScene(gl)
        drawRect(gl,postProgInfo,postbuf,texStruct,camera)
        
        //drawLight(gl,lightProgInfo,lightbuf,camera)
        delta+=0.01
        requestAnimationFrame(render)
    }
    requestAnimationFrame(render)
}

main()