import { AriaCallable } from "../base/AriaBaseDefs";
import { AriaShaderUniformTp } from "../graphics/AriaShaderOps";
import { IAriaFramebuffer } from "./IAriaFramebuffer";
import { IAriaGLBuffer } from "./IAriaGLBuffer";
import { IAriaShader } from "./IAriaShader";
import { IAriaTexture } from "./IAriaTexture";

export interface IAriaRendererCore{
    defineUniform(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)):void;
    setCameraPos(x:number,y:number,z:number):void;
    defineAttribute(attName:string, value:IAriaGLBuffer, size?:number, type?:number):void;
    createFramebuffer(depthComponent:WebGLBuffer,texture:WebGLTexture,postTrigger:()=>any):any;
    clearScreen():void;
    activateFramebuffer(buf:IAriaFramebuffer):void;
    removeFramebuffer():void;
    createEmptyRBO(width:number,height:number):WebGLBuffer;
    createEmptyTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean):WebGLTexture;
    createEmptyCubicTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean):WebGLTexture;
    getCubicLookat(x:number):Float32Array;
    readElementBuffer(destBuffer:ArrayBuffer):void;
    initShaderProgram(vsrc:string,fsrc:string):WebGLProgram|null;
    useShader(shader:IAriaShader, onSuccess?:AriaCallable):void;
    createTexture(img:HTMLImageElement):WebGLTexture;
    readArrayBuffer(destBuffer:ArrayBuffer):void;
    defineUniformCounter(attName:string, increment?:number, returnAfter?:boolean):number;
    defineUniformExtend(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture), index:number):number;
    createCubicTexture(img:HTMLImageElement[]):WebGLTexture;
    getEnv(): WebGL2RenderingContext
    useInvariantShader(shader:IAriaShader,effRange:()=>any):void
    renderInstancedEntry(num:number, instances?:number):void
    clearScreenRequest():void;
    withCubicTexture(c:IAriaTexture,callable:()=>any):void;
    withNoDepthMask(callable:()=>any):void;
    clearScreenInternal():void;
    createTexture3D(img:any,w:number,h:number,d:number):WebGLTexture;
    createTextureData2D(img:any,w:number,h:number):WebGLTexture;
}