import { AriaCallable } from "../base/AriaBaseDefs";
import { AriaObject } from "../base/AriaObject";
import { AriaRendererCompatUtils, IAriaCoreBuffer, IAriaCoreData, IAriaCoreRenderingContext, IAriaCoreShaderProgram, IAriaCoreTexture } from "../base/AriaRendererCompatDef";
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
    createElementBuffer(): IAriaCoreBuffer<any> {
        throw new Error("Method not implemented.");
    }
    useElementBuffer(buffer: IAriaCoreBuffer<any>) {
        throw new Error("Method not implemented.");
    }
    setElementBufferData(buffer: IAriaCoreBuffer<any>, data: any) {
        throw new Error("Method not implemented.");
    }
    useArrayBuffer(buffer: IAriaCoreBuffer<any>) {
        throw new Error("Method not implemented.");
    }
    setArrayBufferData(buffer: IAriaCoreBuffer<any>, data: any) {
        throw new Error("Method not implemented.");
    }
    createArrayBuffer(): IAriaCoreBuffer<any> {
        throw new Error("Method not implemented.");
    }
    getUniformLocation(shader: IAriaCoreShaderProgram<any>, name: string): IAriaCoreData {
        throw new Error("Method not implemented.");
    }
    getAttribLocation(shader: IAriaCoreShaderProgram<any>, name: string): number {
        throw new Error("Method not implemented.");
    }
    public getTextureBufferData(id: IAriaCoreTexture, dataType: number, format: number, w: number, h: number): unknown {
        throw new Error("Method not implemented.");
    }
    public createTextureData2D(img: any, w: number, h: number): IAriaCoreTexture {
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
    public  createFramebuffer(depthComponent:IAriaCoreBuffer,texture:IAriaCoreTexture,postTrigger:()=>any):any{
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
        return AriaRendererCompatUtils.createBufferPlaceholder()
    }
    public createEmptyTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean){
        this._logError("aria_renderer_core: createEmptyTexture not implemented")
        return AriaRendererCompatUtils.createTexturePlaceholder()
    }
    public createEmptyCubicTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean){
        this._logError("aria_renderer_core: createEmptyCubicTexture not implemented")
        return AriaRendererCompatUtils.createTexturePlaceholder()
    }
    public getCubicLookat(x:number){
        this._logError("aria_renderer_core: getCubicLookat not implemented")
        return <Float32Array><unknown>(0);
    }
    public readElementBuffer(destBuffer:ArrayBuffer){
        this._logError("aria_renderer_core: readElementBuffer not implemented")
        
    }
    public initShaderProgram(vsrc:string,fsrc:string):IAriaCoreShaderProgram|null{
        this._logError("aria_renderer_core: initShaderProgram not implemented")
        return null
    }
    public useShader(shader:IAriaShader, onSuccess?:AriaCallable){
        this._logError("aria_renderer_core: useShader not implemented")
    }
    public createTexture(img:HTMLImageElement):IAriaCoreTexture{
        this._logError("aria_renderer_core: createTexture not implemented")
        return AriaRendererCompatUtils.createTexturePlaceholder()
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
    public createCubicTexture(img:HTMLImageElement[]):IAriaCoreTexture{
        this._logError("aria_renderer_core: createCubicTexture not implemented")
        
        return AriaRendererCompatUtils.createTexturePlaceholder()
    }
    public getEnv(): IAriaCoreRenderingContext{
        this._logError("aria_renderer_core: getEnv not implemented")
        
        return <IAriaCoreRenderingContext><unknown>0;
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
    public createTexture3D(img:any,w:number,h:number,d:number):IAriaCoreTexture{
        this._logError("aria_render_core: createTexture3D not implemented")
        throw Error()
    }
}