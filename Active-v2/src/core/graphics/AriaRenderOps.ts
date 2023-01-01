import { mat4, vec3 } from "gl-matrix-ts";
import { AriaEnv } from "./AriaEnv";
import { AriaGacCompiler } from "../aux-compiler/AriaGacCompiler";
import { AriaObject } from "../base/AriaObject";
import { IAriaFramebuffer } from "../interface/IAriaFramebuffer";
import { IAriaTexture } from "../interface/IAriaTexture";

class AriaFramebufferManager extends AriaObject{
    private static _instance:AriaFramebufferManager|undefined = undefined
    private activeFramebuffer:IAriaFramebuffer[] = []
    private constructor(){
        super("AriaFramebufferManager")
    }
    public static getInst(){
        if(this._instance===undefined){
            this._instance = new this()
        }
        return this._instance!
    }
    public setFramebuffer(buf:IAriaFramebuffer){
        this.activeFramebuffer.push(buf)
    }
    public getFramebuffer(){
        if(this.activeFramebuffer.length==0){
            return null
        }
        return this.activeFramebuffer[this.activeFramebuffer.length-1]
    }
    public removeFramebuffer(){
        return this.activeFramebuffer.pop()
    }

}

class AriaCameraPositionManager extends AriaObject{
    private camPos = [0,0,0]
    private static _instance:AriaCameraPositionManager|undefined = undefined
    private constructor(){
        super("AriaCameraPositionManager")
    }
    public static getInst(){
        if(this._instance===undefined){
            this._instance = new this()
        }
        return this._instance!
    }
    public setCameraPosition(x:number,y:number,z:number){
        this.camPos = [x,y,z]
    }
    public getCubeMapLookat(face:number){
        const lookDirs = [
            [1,0,0],
            [-1,0,0],
            [0,1,0],
            [0,-1,0],
            [0,0,1],
            [0,0,-1]
        ]
        const upDirs = [
            [0,-1,0],
            [0,-1,0],
            [0,0,1],
            [0,0,-1],
            [0,-1,0],
            [0,-1,0],
            [0,-1,0]
        ]
        const dpos = new Float32Array([this.camPos[0],this.camPos[1],this.camPos[2]])
        const dfront = new Float32Array([this.camPos[0]+lookDirs[face][0],this.camPos[1]+lookDirs[face][1],this.camPos[2]+lookDirs[face][2]])
        const dtop = new Float32Array([upDirs[face][0],upDirs[face][1],upDirs[face][2]])
        const ret = mat4.create()
        mat4.lookAt(ret,dpos,dfront,dtop)
        return ret
    }
}

export class AriaRenderOps extends AriaObject{
    constructor(){
        super("AriaRenderOps")
    }
    public static getCubicLookat(face:number){
        return AriaCameraPositionManager.getInst().getCubeMapLookat(face)
    }
    public static setCameraPos(x:number,y:number,z:number){
        AriaCameraPositionManager.getInst().setCameraPosition(x,y,z)
    }

    public static activateFramebuffer(buf:IAriaFramebuffer){
        AriaFramebufferManager.getInst().setFramebuffer(buf)
    }
    public static removeFramebuffer(){
        AriaFramebufferManager.getInst().removeFramebuffer()
    }
    private static getActiveFramebuffer(){
        return AriaFramebufferManager.getInst().getFramebuffer()
    }
    public static clearScreenRequest(){
        return AriaFramebufferManager.getInst().getFramebuffer()?.onClear()
    }
    public static clearScreen(color:number[] = [0,0,0,0]){
        const gl = AriaEnv.env
        gl.clearColor(color[0],color[1],color[2],color[3]);
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LEQUAL);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
    }

    public static loadShader(type:number,source:string):WebGLShader|null{
        const gl = AriaEnv.env
        const shader = <WebGLShader>gl.createShader(type);
        gl.shaderSource(shader,source);
        gl.compileShader(shader);
        if(!gl.getShaderParameter(shader,gl.COMPILE_STATUS)){
            alert("Shader error:"+gl.getShaderInfoLog(shader));
            console.log("Cannot Compile Shader")
            console.log(source)
            gl.deleteShader(shader)
            return null;
        }
        return shader;
    }
    

    public static initShaderProgram(vsrc:string,fsrc:string,useAux:boolean=true):WebGLProgram|null{
        const gl = AriaEnv.env
        const compiler = new AriaGacCompiler()
        compiler.compile(vsrc)
        vsrc = compiler.generateCode()
        compiler.compile(fsrc)
        fsrc = compiler.generateCode()

        const vs = this.loadShader(gl.VERTEX_SHADER,vsrc)
        const fs = this.loadShader(gl.FRAGMENT_SHADER,fsrc)
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

    public static renderInstancedEntry(num:number, instances:number = 1){
        
        const renderCall = ()=>{
            this.renderInstancedImpl(num, instances)
        }
        const curFbo = this.getActiveFramebuffer()
        if(curFbo===null){
            
            renderCall()
        }else{
            curFbo.onRender(renderCall)
        }
    }

    public static renderInstancedImpl(num:number, instances:number = 1){
        const gl = AriaEnv.env
        gl.drawElementsInstanced(gl.TRIANGLES, num, gl.UNSIGNED_SHORT, 0, instances)
    }

    public static fboUnbind(){
        const gl = AriaEnv.env
        gl.viewport(0,0,window.innerWidth,window.innerHeight)
        gl.bindBuffer(gl.FRAMEBUFFER, null)
    }

    public static createTexture(img:HTMLImageElement):WebGLTexture{
        const gl = AriaEnv.env
        const tex = gl.createTexture()
        gl.bindTexture(gl.TEXTURE_2D,tex);
        gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE,img);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.bindTexture(gl.TEXTURE_2D,null)
        return <WebGLTexture>tex;
    }

    public static createCubicTexture(img:HTMLImageElement[]):WebGLTexture{
        const gl = AriaEnv.env
        const tex = gl.createTexture()
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,tex);
        for(let i=0;i<6;i++){
            gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X+i,0,gl.RGB,gl.RGB,gl.UNSIGNED_BYTE,img[i]);
        }
        
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_R, gl.CLAMP_TO_EDGE);

        gl.bindTexture(gl.TEXTURE_CUBE_MAP,null)
        return <WebGLTexture>tex;
    }

    public static createEmptyTexture(width:number, height:number, mipmap:boolean = false, hdr:boolean = true){
        const gl = AriaEnv.env
        const tex = gl.createTexture()!
        gl.bindTexture(gl.TEXTURE_2D,tex)
        if(hdr){
            gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA16F,width,height,0,gl.RGBA,gl.FLOAT,null)
        }else{
            gl.texImage2D(gl.TEXTURE_2D,0,gl.RGB,width,height,0,gl.RGB,gl.UNSIGNED_BYTE,null)
        
        }
        if(mipmap){
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        }else{
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        }
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindTexture(gl.TEXTURE_2D,null);
        return tex
    }

    public static createEmptyCubicTexture(width:number, height:number, mipmap:boolean = false, hdr:boolean = true){
        const gl = AriaEnv.env
        const tex = gl.createTexture()!
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,tex)
        for(let i=0;i<6;i++){
            if(hdr){
                gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X+i,0,gl.RGBA16F,width,height,0,gl.RGBA,gl.FLOAT,null)
            }else{
                gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X+i,0,gl.RGB,width,height,0,gl.RGB,gl.UNSIGNED_BYTE,null)
            }
            
        }
        if(mipmap){
            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        }else{
            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        }
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,null);
        return tex
    }

    public static createEmptyRBO(width:number,height:number){
        const gl = AriaEnv.env
        const rbo = <WebGLRenderbuffer>gl.createRenderbuffer()
        gl.bindRenderbuffer(gl.RENDERBUFFER, rbo)
        gl.renderbufferStorage(gl.RENDERBUFFER,gl.DEPTH24_STENCIL8,width,height)
        gl.bindRenderbuffer(gl.RENDERBUFFER,null)
        return rbo
    }

    public static withNoDepthMask(callable:()=>any){
        const gl = AriaEnv.env
        gl.depthMask(false)
        callable()
        gl.depthMask(true)
    }

    public static  withCubicTexture(c:IAriaTexture,callable:()=>any){
        const gl = AriaEnv.env
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,c.getTex())
        callable()
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,null)
    }

    public static createFramebuffer(depthComponent:WebGLBuffer,texture:WebGLTexture,postTrigger:()=>any){
        const gl = AriaEnv.env
        const fb = gl.createFramebuffer()!
        gl.bindFramebuffer(gl.FRAMEBUFFER,fb)
        gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,texture,0)
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,depthComponent)
        
        postTrigger()
        gl.bindFramebuffer(gl.FRAMEBUFFER,null)
    }
}