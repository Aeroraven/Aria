import { AriaCallable } from "../base/AriaBaseDefs";
import { AriaObject } from "../base/AriaObject";
import { AriaShaderUniformTp } from "../graphics/AriaShaderOps";
import { IAriaFramebuffer } from "../interface/IAriaFramebuffer";
import { IAriaGLBuffer } from "../interface/IAriaGLBuffer";
import { IAriaRendererCore } from "../interface/IAriaRendererCore";
import { IAriaShader } from "../interface/IAriaShader";
import { IAriaTexture } from "../interface/IAriaTexture";

export class AriaRendererCore extends AriaObject implements IAriaRendererCore{
    defaultDisplayId:string = "webgl_displayer"
    constructor(){
        super("AriaRenderer")
    }
    public getTextureBufferData(id: WebGLTexture, dataType: number, format: number, w: number, h: number): unknown {
        throw new Error("Method not implemented.");
    }
    public createTextureData2D(img: any, w: number, h: number): WebGLTexture {
        throw new Error("Method not implemented.");
    }
    public setCameraPos(x: number, y: number, z: number): void {
        this._logError("aria_renderer_core: setCameraPos not implemented")
    }
    public defineUniform(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)){
        this._logError("aria_renderer_core: defineUniform not implemented")
    }
    public defineAttribute(attName:string, value:IAriaGLBuffer, size?:number, type?:number){
        this._logError("aria_renderer_core: defineAttribute not implemented")
    }
    public  createFramebuffer(depthComponent:WebGLBuffer,texture:WebGLTexture,postTrigger:()=>any):any{
        this._logError("aria_renderer_core: createFramebuffer not implemented")
    }
    public clearScreen(){
        this._logError("aria_renderer_core: clearScreen not implemented")
    }
    public activateFramebuffer(buf:IAriaFramebuffer){
        this._logError("aria_renderer_core: activateFramebuffer not implemented")
    }
    public removeFramebuffer(){
        this._logError("aria_renderer_core: removeFramebuffer not implemented")
    }
    public createEmptyRBO(width:number,height:number){
        this._logError("aria_renderer_core: createEmptyRBO not implemented")
        return <WebGLBuffer>(0);
    }
    public createEmptyTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean){
        this._logError("aria_renderer_core: createEmptyTexture not implemented")
        return <WebGLTexture>(0);
    }
    public createEmptyCubicTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean){
        this._logError("aria_renderer_core: createEmptyCubicTexture not implemented")
        return <WebGLTexture>(0);
    }
    public getCubicLookat(x:number){
        this._logError("aria_renderer_core: getCubicLookat not implemented")
        return <Float32Array><unknown>(0);
    }
    public readElementBuffer(destBuffer:ArrayBuffer){
        this._logError("aria_renderer_core: readElementBuffer not implemented")
        
    }
    public initShaderProgram(vsrc:string,fsrc:string):WebGLProgram|null{
        this._logError("aria_renderer_core: initShaderProgram not implemented")
        return null
    }
    public useShader(shader:IAriaShader, onSuccess?:AriaCallable){
        this._logError("aria_renderer_core: useShader not implemented")
    }
    public createTexture(img:HTMLImageElement):WebGLTexture{
        this._logError("aria_renderer_core: createTexture not implemented")
        return <WebGLTexture>(0);
    }
    public readArrayBuffer(destBuffer:ArrayBuffer){
        this._logError("aria_renderer_core: readArrayBuffer not implemented")
    }
    public defineUniformCounter(attName:string, increment?:number, returnAfter?:boolean){
        this._logError("aria_renderer_core: defineUniformCounter not implemented")
        return 0
    }
    public defineUniformExtend(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture), index:number){
        this._logError("aria_renderer_core: defineUniformExtend not implemented")
        return 0
    }
    public createCubicTexture(img:HTMLImageElement[]):WebGLTexture{
        this._logError("aria_renderer_core: createCubicTexture not implemented")
        
        return <WebGLTexture>0
    }
    public getEnv(): WebGL2RenderingContext{
        this._logError("aria_renderer_core: getEnv not implemented")
        
        return <WebGL2RenderingContext><unknown>0;
    }
    public useInvariantShader(shader:IAriaShader,effRange:()=>any){
        this._logError("aria_renderer_core: useInvariantShader not implemented")
        
    }
    public renderInstancedEntry(num:number, instances?:number){
        this._logError("aria_renderer_core: renderInstancedEntry not implemented")
    }
    public clearScreenRequest(){

    }
    public withCubicTexture(c:IAriaTexture,callable:()=>any){

    }
    public  withNoDepthMask(callable:()=>any){
        
    }
    public clearScreenInternal(){
        
    }
    public createTexture3D(img:any,w:number,h:number,d:number):WebGLTexture{
        this._logError("aria_render_core: createTexture3D not implemented")
        throw Error()
    }
}