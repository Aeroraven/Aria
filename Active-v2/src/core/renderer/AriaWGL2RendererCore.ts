import { IAriaRenderable } from "../../components/base/interface/IAriaRenderable";
import { AriaGacCompiler } from "../aux-compiler/AriaGacCompiler";
import { AriaCallable } from "../base/AriaBaseDefs";
import { AriaObject } from "../base/AriaObject";
import { AriaRendererCompatUtils, IAriaCoreBuffer, IAriaCoreData, IAriaCoreRenderingContext, IAriaCoreShaderProgram, IAriaCoreTexture } from "../base/AriaRendererCompatDef";
import { AriaShaderUniformTp } from "../graphics/AriaShaderOps";
import { IAriaFramebuffer } from "../interface/IAriaFramebuffer";
import { IAriaGeneralBuffer } from "../interface/IAriaGeneralBuffer";
import { IAriaRendererCore } from "../interface/IAriaRendererCore";
import { IAriaShader } from "../interface/IAriaShader";
import { IAriaTexture } from "../interface/IAriaTexture";
import { AriaRendererCore } from "./AriaRendererCore";
import { mat4, vec3 } from "gl-matrix-ts";

class AriaWGL2RendererFramebufferManager extends AriaObject{
    private activeFramebuffer:IAriaFramebuffer[] = []
    constructor(){
        super("AriaWGL2RendererFramebufferManager")
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

class AriaWGL2RendererCameraPositionManager extends AriaObject{
    private camPos = [0,0,0]
    constructor(){
        super("AriaCameraPositionManager")
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

class AriaWGL2ShaderManager extends AriaObject{
    private activatedShader:IAriaShader|null = null
    private extendUniformMaps: Map<string,number> = new Map<string,number>()
    private counterUniformMaps: Map<string,number> = new Map<string,number>()
    private invariantShader:boolean = false

    constructor(){
        super("AriaShaderManager")
    }
    public getRenderSide(){
        if(this.activatedShader==null){
            return 'front'
        }else{
            return this.activatedShader.getSide()
        }
    }
    public disableShaderChange(){
        this.invariantShader = true
    }
    public enableShaderChange(){
        this.invariantShader = false
    }
    public setShader(x:IAriaShader){
        if(this.invariantShader==false){
            this.activatedShader = x
        }
        this.extendUniformMaps = new Map<string,number>()
        this.counterUniformMaps = new Map<string,number>()
        return this.invariantShader==false
    }
    public getShader(){
        if(this.activatedShader==null){
            this._logError("Invalid shader operation")
        }
        return <IAriaShader>this.activatedShader
    }
    public allocNewExtendUniform(s:string){
        const g = this.extendUniformMaps.get(s)
        if(!g){
            this.extendUniformMaps.set(s,1)
            return 0;
        }else{
            this.extendUniformMaps.set(s,g+1)
            return g
        }
    }
    public addCounterUniform(s:string, inc:number=1, returnAfter:boolean=false){
        const g = this.counterUniformMaps.get(s)
        if(!g){
            this.counterUniformMaps.set(s,inc);
            return 0 + (returnAfter?inc:0);
        }else{
            this.counterUniformMaps.set(s,g+inc)
            return g + (returnAfter?inc:0)
        }
    }
}

class AriaWGL2RendererRenderOps extends AriaObject{
    env:WebGL2RenderingContext
    parent:IAriaRendererCore
    camManager:AriaWGL2RendererCameraPositionManager = new AriaWGL2RendererCameraPositionManager()
    fbManager:AriaWGL2RendererFramebufferManager = new AriaWGL2RendererFramebufferManager()

    constructor(parent:IAriaRendererCore, renderingContext:WebGL2RenderingContext){
        super("AriaWGL2RendererRenderOps")
        this.env = renderingContext
        this.parent = parent
    }
    public getTextureBufferData(id: WebGLTexture, dataType: number, format: number, w: number, h: number): unknown {
        let gl = this.env
        let fb = gl.createFramebuffer();
        gl.bindFramebuffer(gl.FRAMEBUFFER, fb);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, id, 0);
        let pixels:ArrayBufferView|undefined = undefined
        if(format == gl.R8){
            pixels = new Uint8Array(w * h * 4);
        }else{
            this._logError("aria.wgl2renderer.renderops.gettexbufferdata: unsupported type")
            throw Error()
            
        }
        if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) == gl.FRAMEBUFFER_COMPLETE) {
            gl.readPixels(0, 0, w, h, format, dataType, pixels);
        }
        return pixels
    }

    public readArrayBuffer(destBuffer:ArrayBuffer){
        this.env.getBufferSubData(this.env.ARRAY_BUFFER,0,new Float32Array(destBuffer))
    }
    public readElementBuffer(destBuffer:ArrayBuffer){
        this.env.getBufferSubData(this.env.ELEMENT_ARRAY_BUFFER,0,new Uint16Array(destBuffer))
    }
    public getCubicLookat(face:number){
        return this.camManager.getCubeMapLookat(face)
    }
    public setCameraPos(x:number,y:number,z:number){
        this.camManager.setCameraPosition(x,y,z)
    }

    public activateFramebuffer(buf:IAriaFramebuffer){
        this.fbManager.setFramebuffer(buf)
    }
    public removeFramebuffer(){
        this.fbManager.removeFramebuffer()
    }
    private getActiveFramebuffer(){
        return this.fbManager.getFramebuffer()
    }
    public clearScreenRequest(){
        let frame = this.fbManager.getFramebuffer()
        if(frame===null){
            this.parent.clearScreen()
        }else{
            frame.onClear(this.parent)
        }
    }
    public clearScreenDeprecated(color:number[] = [0,0,0,0]){
        const gl = this.env
        gl.clearColor(color[0],color[1],color[2],color[3]);
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LEQUAL);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
    }

    public loadShader(type:number,source:string):WebGLShader|null{
        const gl = this.env
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

    public initShaderProgram(vsrc:string,fsrc:string,useAux:boolean=true):IAriaCoreShaderProgram|null{
        const gl = this.env
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
            return AriaRendererCompatUtils.createShaderProgram(shaderProg)
        }
    }

    public renderInstancedEntry(num:number, instances:number = 1){
        const renderCall = ()=>{
            this.renderInstancedImpl(num, instances)
        }
        const curFbo = this.getActiveFramebuffer()
        if(curFbo===null){
            
            renderCall()
        }else{
            curFbo.onRender(this.parent,renderCall)
        }
    }
    public renderInstancedImpl(num:number, instances:number = 1){
        const gl = this.env
        gl.drawElementsInstanced(gl.TRIANGLES, num, gl.UNSIGNED_SHORT, 0, instances)
    }
    public fboUnbind(){
        const gl = this.env
        gl.viewport(0,0,window.innerWidth,window.innerHeight)
        gl.bindBuffer(gl.FRAMEBUFFER, null)
    }
    public createTexture(img:HTMLImageElement):IAriaCoreTexture{
        const gl = this.env
        const tex = gl.createTexture()
        gl.bindTexture(gl.TEXTURE_2D,tex);
        gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE,img);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.bindTexture(gl.TEXTURE_2D,null)
        return AriaRendererCompatUtils.createTexture(tex)
    }
    public createTexture3D(img:any,w:number,h:number,d:number):IAriaCoreTexture{
        const gl = this.env
        const tex = gl.createTexture()
        gl.bindTexture(gl.TEXTURE_3D,tex);
        gl.texImage3D(gl.TEXTURE_3D,0,gl.R8,w,h,d,0,gl.RED,gl.UNSIGNED_BYTE,img);
        gl.texParameteri(gl.TEXTURE_3D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.bindTexture(gl.TEXTURE_3D,null)
        return AriaRendererCompatUtils.createTexture(tex)
    }
    public createTextureData2D(img:any,w:number,h:number):IAriaCoreTexture{
        const gl = this.env
        const tex = gl.createTexture()
        gl.bindTexture(gl.TEXTURE_2D,tex);
        gl.texImage2D(gl.TEXTURE_2D,0,gl.R8,w,h,0,gl.RED,gl.UNSIGNED_BYTE,img);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.bindTexture(gl.TEXTURE_2D,null)
        return AriaRendererCompatUtils.createTexture(tex)
    }
    public createCubicTexture(img:HTMLImageElement[]):IAriaCoreTexture{
        const gl = this.env
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
        return AriaRendererCompatUtils.createTexture(tex!)
    }
    public createEmptyTexture(width:number, height:number, mipmap:boolean = false, hdr:boolean = true){
        const gl = this.env
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
        return AriaRendererCompatUtils.createTexture(tex)
    }
    public createEmptyCubicTexture(width:number, height:number, mipmap:boolean = false, hdr:boolean = true){
        const gl = this.env
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
        return AriaRendererCompatUtils.createTexture(tex)
    }
    public createEmptyRBO(width:number,height:number){
        const gl = this.env
        const rbo = <WebGLRenderbuffer>gl.createRenderbuffer()
        gl.bindRenderbuffer(gl.RENDERBUFFER, rbo)
        gl.renderbufferStorage(gl.RENDERBUFFER,gl.DEPTH24_STENCIL8,width,height)
        gl.bindRenderbuffer(gl.RENDERBUFFER,null)
        return AriaRendererCompatUtils.createBuffer(rbo)
    }
    public withNoDepthMask(callable:()=>any){
        const gl = this.env
        gl.depthMask(false)
        callable()
        gl.depthMask(true)
    }
    public  withCubicTexture(c:IAriaTexture,callable:()=>any){
        const gl = this.env
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,c.getTex(this.parent))
        callable()
        gl.bindTexture(gl.TEXTURE_CUBE_MAP,null)
    }

    public createFramebuffer(){
        const gl = this.env
        const fb = gl.createFramebuffer()!
        return AriaRendererCompatUtils.createBuffer(fb)
    }
    public createArrayBuffer(): IAriaCoreBuffer<any> {
        return AriaRendererCompatUtils.createBuffer(this.env.createBuffer()!)
    }
    public useArrayBuffer(buffer: IAriaCoreBuffer<any>) {
        this.env.bindBuffer(this.env.ARRAY_BUFFER,buffer.data)
    }
    public setArrayBufferData(buffer: IAriaCoreBuffer<any>, data: any) {
        this.env.bindBuffer(this.env.ARRAY_BUFFER,buffer.data)
        this.env.bufferData(this.env.ARRAY_BUFFER,data,this.env.STATIC_DRAW)
        
    }
    createElementBuffer(): IAriaCoreBuffer<any> {
        return AriaRendererCompatUtils.createBuffer(this.env.createBuffer()!)
    }
    useElementBuffer(buffer: IAriaCoreBuffer<any>) {
        this.env.bindBuffer(this.env.ELEMENT_ARRAY_BUFFER,buffer.data)
    }
    setElementBufferData(buffer: IAriaCoreBuffer<any>, data: any) {
        this.env.bindBuffer(this.env.ELEMENT_ARRAY_BUFFER,buffer.data)
        this.env.bufferData(this.env.ELEMENT_ARRAY_BUFFER,data,this.env.STATIC_DRAW)
    }
    public setViewport(w: number, h: number) {
        this.env.viewport(0,0,w,h)
    }
    public generateMipmap2D(tex: IAriaCoreTexture<any>) {
        this.env.bindTexture(this.env.TEXTURE_2D,tex.data)
        this.env.generateMipmap(this.env.TEXTURE_2D)
        this.env.bindTexture(this.env.TEXTURE_2D,null)
    }
    public switchFramebufferAttachmentsForCubemap(renderBuffer: IAriaCoreBuffer<any>, texture: IAriaCoreTexture<any>,order:number) {
        const gl = this.env
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_CUBE_MAP_POSITIVE_X+order, texture.data, 0);
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,renderBuffer.data)
    }
    public setFramebufferRenderBuffer(renderbuffer: IAriaCoreBuffer<any>) {
        const gl = this.env
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,renderbuffer.data)
    }
    setFramebufferTexture(texture: IAriaCoreTexture<any>) {
        const gl = this.env
        gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,texture.data,0)
    }
    setFramebufferTextureCubic(texture: IAriaCoreTexture<any>) {
        const gl = this.env
        gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_CUBE_MAP_POSITIVE_X,texture.data,0)
    }
}



export class AriaWGL2RendererShaderOps extends AriaObject{
    shaderManager:AriaWGL2ShaderManager = new AriaWGL2ShaderManager()
    parent:IAriaRendererCore
    env:WebGL2RenderingContext
    constructor(parent:IAriaRendererCore,env:WebGL2RenderingContext){
        super("AriaWGL2RendererShaderOps")
        this.env = env
        this.parent = parent
    }
    public clearScreenByShader(color:number[] = [0,0,0,0]){
        const gl = this.env
        gl.clearColor(color[0],color[1],color[2],color[3]);
        gl.enable(gl.DEPTH_TEST);
        let side = this.shaderManager.getRenderSide()
        if(side=='front'){
            gl.depthFunc(gl.LEQUAL);
            gl.clearDepth(1)
        }else if(side=='back'){
            gl.depthFunc(gl.GEQUAL);
            gl.clearDepth(0);
        }else{
            this._logError("unsupported clear side")
        }
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
    }
    
    public useShader(shader:IAriaShader, onSuccess:AriaCallable=()=>{}){
        if(this.shaderManager.setShader(shader)){
            const gl = this.env
            gl.useProgram(shader.getShaderProgram().data)
            onSuccess()
        }
    }
    public useInvariantShader(shader:IAriaShader,effRange:()=>any){
        this.useShader(shader)
        this.shaderManager.disableShaderChange()
        effRange()
        this.shaderManager.enableShaderChange()
    }

    public defineAttribute(attName:string, value:IAriaGeneralBuffer, size:number = 3, type:number = this.env.FLOAT){
        const gl = this.env
        const acShader = this.shaderManager.getShader()
        //gl.bindBuffer(gl.ARRAY_BUFFER, value.getGLObject())
        value.bind(this.parent)
        gl.vertexAttribPointer(acShader.getAttribute(this.parent,attName), size, type, false, 0, 0)
        gl.enableVertexAttribArray(acShader.getAttribute(this.parent,attName))
    }
    public defineUniformCounter(attName:string, increment:number=1, returnAfter:boolean=true){
        let nVal = this.shaderManager.addCounterUniform(attName, increment, returnAfter)
        this.defineUniform(attName,AriaShaderUniformTp.ASU_VEC1I, nVal)
        return nVal
    }
    public defineUniformExtend(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture), index:number=-1){
        let nId = index
        if(index==-1){
            nId = this.shaderManager.allocNewExtendUniform(attName)
        }
        const newAttName = attName + "[" + nId + "]"
        this.defineUniform(newAttName, type, value)
        return nId
    }
    public getUniformLocation(shader: IAriaCoreShaderProgram<any>, name: string): IAriaCoreData {
        return AriaRendererCompatUtils.createData(this.env.getUniformLocation(shader.data,name)!)
    }
    public getAttribLocation(shader: IAriaCoreShaderProgram<any>, name: string): number {
        return this.env.getAttribLocation(shader.data,name)
    }
    public defineUniform(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)){
        const gl = this.env
        const acShader = this.shaderManager.getShader();
        
        const isArrayGuard = (x:any):x is number[]=>{
            if(x instanceof Array){
                return true
            }
            return false
        }
        const isNumGuard = (x:any):x is number=>{
            if (typeof x == 'number'){
                return true
            }
            return false
        }
        const isFloat32 = (x:any):x is Float32Array=>{
            if (x instanceof Float32Array){
                return true
            }
            return false
        }
        const isTex = (x:any):x is IAriaTexture=>{
            return true
        }
        const uniformLoc = acShader.getUniform(this.parent,attName)?.data
        switch (type) {
            case AriaShaderUniformTp.ASU_MAT4:
                if(isArrayGuard(value)){
                    gl.uniformMatrix4fv(uniformLoc,false,new Float32Array(value))
                }
                else if(isFloat32(value)){
                    gl.uniformMatrix4fv(uniformLoc,false,value)
                }else{
                    this._logError("Invalid type")
                }
                break;
            
            case AriaShaderUniformTp.ASU_VEC3:
                if(isArrayGuard(value)){
                    gl.uniform3fv(uniformLoc,new Float32Array(value))
                }
                else if(isFloat32(value)){
                    gl.uniform3fv(uniformLoc,value)
                }else{
                    this._logError("Invalid type")
                }
                break;
            
            case AriaShaderUniformTp.ASU_VEC4:
                if(isArrayGuard(value)){
                    gl.uniform4fv(uniformLoc,new Float32Array(value))
                }
                else if(isFloat32(value)){
                    gl.uniform4fv(uniformLoc,value)
                }else{
                    this._logError("Invalid type")
                }
                break;
            
            case AriaShaderUniformTp.ASU_VEC1:
                if(isNumGuard(value)){
                    gl.uniform1f(uniformLoc,value)
                }else{
                    this._logError("Invalid type")
                }
                break;
            case AriaShaderUniformTp.ASU_VEC1I:
                if(isNumGuard(value)){
                    gl.uniform1i(uniformLoc,value)
                }else{
                    this._logError("Invalid type")
                }
                break;
            case AriaShaderUniformTp.ASU_TEX2D:
                if(isTex(value)){
                    const v = acShader.allocateTexture()
                    gl.activeTexture(v)
                    gl.bindTexture(gl.TEXTURE_2D, value.getTex(this.parent).data)
                    gl.uniform1i(uniformLoc,v-this.env.TEXTURE0);
                }else{
                    this._logError("Invalid type")
                }
                break;
            case AriaShaderUniformTp.ASU_TEX3D:
                if(isTex(value)){
                    const v = acShader.allocateTexture()
                    gl.activeTexture(v)
                    gl.bindTexture(gl.TEXTURE_3D, value.getTex(this.parent).data)
                    gl.uniform1i(uniformLoc,v-this.env.TEXTURE0);
                }else{
                    this._logError("Invalid type")
                }
                break;
            case AriaShaderUniformTp.ASU_TEXCUBE:
                    if(isTex(value)){
                        const v = acShader.allocateTexture()
                        gl.activeTexture(v)
                        gl.bindTexture(gl.TEXTURE_CUBE_MAP, value.getTex(this.parent).data)
                        gl.uniform1i(uniformLoc,v-this.env.TEXTURE0);
                    }else{
                        this._logError("Invalid type")
                    }
                    break;
            default:
                this._logError("Invalid type:" + type)
                break;
        }
    }
}

export class AriaWGL2RendererCore extends AriaRendererCore{

    private env:WebGL2RenderingContext|null = null
    private canvas:HTMLCanvasElement|null = null
    private renderOps:AriaWGL2RendererRenderOps
    private shaderOps:AriaWGL2RendererShaderOps

    constructor(containerId:string){
        super()
        this._rename("AriaWGL2Renderer")
        this.createRuntime(containerId)
        this.renderOps = new AriaWGL2RendererRenderOps(this,this.env!)
        this.shaderOps = new AriaWGL2RendererShaderOps(this,this.env!)
    }

    private createRuntime(containerId:string=this.defaultDisplayId){
        const canvas = <HTMLCanvasElement>(document.getElementById(containerId));
        canvas.width = window.innerWidth
        canvas.height = window.innerHeight

        this.env = canvas.getContext("webgl2", { stencil: true })
        || (()=>{
            this._logError("WebGL2 is not supported. Update your browser to view the content")
            return null
        })();
        if(this.env!=null){
            this._logInfo("Initialized WebGL2 context")
        }

        this.canvas = canvas
        const gl = this.env!
        gl.viewport(0,0,window.innerWidth,window.innerHeight)
        gl.disable(gl.CULL_FACE)

        if(gl.getExtension('EXT_color_buffer_float')==null){
            this._logError("Cannot enable extension `EXT_color_buffer_float`. Please upgrade your browser!")
        }else{
            this._logInfo("Enabled extension `EXT_color_buffer_float`")
        }
    }

    public getCanvas(): HTMLCanvasElement{
        if(this.canvas==undefined||this.canvas==null){
            this._logError("Canvas is not defined")
        }
        return <HTMLCanvasElement>this.canvas
    }

    public getEnv(): IAriaCoreRenderingContext{
        if(this.env==null){
            this._logError("WebGL2 context is not valid")
        }
        return {data:this.env}
    }
    public defineUniform(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)){
        return this.shaderOps.defineUniform(attName,type,value)
    }
    public setCameraPos(x:number,y:number,z:number){
        return this.renderOps.setCameraPos(x,y,z)
    }
    public  defineAttribute(attName:string, value:IAriaGeneralBuffer, size?:number, type?:number){
        size = (size === undefined)? 3 : size
        type = (type === undefined)? this.env?.FLOAT : type
        return this.shaderOps.defineAttribute(attName,value,size,type)
    }

    public createFramebuffer():IAriaCoreBuffer{
        return this.renderOps.createFramebuffer()
    }
    public clearScreen(): void {
        return this.shaderOps.clearScreenByShader()
    }
    public activateFramebuffer(buf:IAriaFramebuffer){
        this.env?.bindFramebuffer(this.env!.FRAMEBUFFER,buf.getGeneralBufferObject().data)
        return this.renderOps.activateFramebuffer(buf)
    }
    public removeFramebuffer(){
        this.env?.bindFramebuffer(this.env!.FRAMEBUFFER,null)
        return this.renderOps.removeFramebuffer()
    }
    public createEmptyRBO(width:number,height:number):IAriaCoreBuffer{
        return this.renderOps.createEmptyRBO(width,height)
    }
    public createEmptyTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean){
        mipmap = (mipmap === undefined)?false:mipmap
        hdr = (hdr === undefined)?true:hdr
        return this.renderOps.createEmptyTexture(width,height,mipmap,hdr)
    }
    public createEmptyCubicTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean){
        mipmap = (mipmap === undefined)?false:mipmap
        hdr = (hdr === undefined)?true:hdr
        return this.renderOps.createEmptyCubicTexture(width,height,mipmap,hdr)
    }
    public getCubicLookat(x:number){
        return this.renderOps.getCubicLookat(x)
    }
    public readElementBuffer(destBuffer:ArrayBuffer){
        return this.renderOps.readElementBuffer(destBuffer)
    }
    public initShaderProgram(vsrc:string,fsrc:string):IAriaCoreShaderProgram|null{
        return this.renderOps.initShaderProgram(vsrc,fsrc,true)
    }
    public useShader(shader:IAriaShader, onSuccess?:AriaCallable){
        onSuccess = (onSuccess===undefined)?(()=>{}):onSuccess
        return this.shaderOps.useShader(shader,onSuccess)
    }
    public createTexture(img:HTMLImageElement):IAriaCoreTexture{
        return this.renderOps.createTexture(img)
    }
    public readArrayBuffer(destBuffer:ArrayBuffer){
        return this.renderOps.readArrayBuffer(destBuffer)
    }
    public defineUniformCounter(attName:string, increment?:number, returnAfter?:boolean){
        increment = (increment === undefined)?1:increment
        returnAfter = (returnAfter === undefined)?true:returnAfter
        return this.shaderOps.defineUniformCounter(attName,increment,returnAfter)
    }
    public defineUniformExtend(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture), index:number){
        return this.shaderOps.defineUniformExtend(attName,type,value,index)
    }
    public createCubicTexture(img:HTMLImageElement[]):IAriaCoreTexture{
        return this.renderOps.createCubicTexture(img)
    }
    public useInvariantShader(shader:IAriaShader,effRange:()=>any){
        return this.shaderOps.useInvariantShader(shader,effRange)
    }
    public renderInstancedEntry(num:number, instances?:number){
        instances = (instances == undefined)?1:instances
        return this.renderOps.renderInstancedEntry(num,instances)
    }
    public clearScreenInternal(){
        return this.shaderOps.clearScreenByShader()
    }
    public clearScreenRequest(){
        return this.renderOps.clearScreenRequest()
    }
    public withCubicTexture(c:IAriaTexture,callable:()=>any){
        return this.renderOps.withCubicTexture(c,callable)
    }
    public withNoDepthMask(callable:()=>any){
        return this.renderOps.withNoDepthMask(callable)
    }
    public createTexture3D(img:any,w:number,h:number,d:number):IAriaCoreTexture{
        return this.renderOps.createTexture3D(img,w,h,d)
    }
    public createTextureData2D(img:any,w:number,h:number):IAriaCoreTexture{
        return this.renderOps.createTextureData2D(img,w,h)
    }
    public getTextureBufferData(id: IAriaCoreTexture, dataType: number, format: number, w: number, h: number): unknown {
        return this.renderOps.getTextureBufferData(id,dataType,format,w,h)
    }
    public getUniformLocation(shader: IAriaCoreShaderProgram<any>, name: string): IAriaCoreData {
        return this.shaderOps.getUniformLocation(shader,name)
    }
    public getAttribLocation(shader: IAriaCoreShaderProgram<any>, name: string): number {
        return this.shaderOps.getAttribLocation(shader,name)
    }
    public createArrayBuffer(): IAriaCoreBuffer<any> {
        return this.renderOps.createArrayBuffer()
    }
    useArrayBuffer(buffer: IAriaCoreBuffer<any>) {
        return this.renderOps.useArrayBuffer(buffer)
    }
    setArrayBufferData(buffer: IAriaCoreBuffer<any>, data: any) {
        return this.renderOps.setArrayBufferData(buffer,data)
    }

    createElementBuffer(): IAriaCoreBuffer<any> {
        return this.renderOps.createElementBuffer()
    }
    useElementBuffer(buffer: IAriaCoreBuffer<any>) {
        return this.renderOps.useElementBuffer(buffer)
    }
    setElementBufferData(buffer: IAriaCoreBuffer<any>, data: any) {
        return this.renderOps.setElementBufferData(buffer,data)
    }
    setViewport(w: number, h: number) {
        return this.renderOps.setViewport(w,h)
    }
    public generateMipmap2D(tex: IAriaCoreTexture<any>) {
        return this.renderOps.generateMipmap2D(tex)
    }
    switchFramebufferAttachmentsForCubemap(renderBuffer: IAriaCoreBuffer<any>, texture: IAriaCoreTexture<any>,order:number) {
        return this.renderOps.switchFramebufferAttachmentsForCubemap(renderBuffer,texture,order)
    }
    setFramebufferRenderBuffer(renderbuffer: IAriaCoreBuffer<any>) {
        return this.renderOps.setFramebufferRenderBuffer(renderbuffer)
    }
    setFramebufferTexture(texture: IAriaCoreTexture<any>) {
        return this.renderOps.setFramebufferTexture(texture)
    }
    setFramebufferTextureCubic(texture: IAriaCoreTexture<any>) {
        return this.renderOps.setFramebufferTextureCubic(texture)
    }
}