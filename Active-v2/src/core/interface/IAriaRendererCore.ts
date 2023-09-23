import { AriaCallable } from "../base/AriaBaseDefs";
import { IAriaCoreBuffer, IAriaCoreData, IAriaCoreRenderingContext, IAriaCoreShaderProgram, IAriaCoreTexture } from "../base/AriaRendererCompatDef";
import { AriaShaderUniformTp } from "../graphics/AriaShaderOps";
import { IAriaFramebuffer } from "./IAriaFramebuffer";
import { IAriaGeneralBuffer } from "./IAriaGeneralBuffer";
import { IAriaShader } from "./IAriaShader";
import { IAriaTexture } from "./IAriaTexture";

export interface IAriaRendererCore{
    defineUniform(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)):void;
    setCameraPos(x:number,y:number,z:number):void;
    defineAttribute(attName:string, value:IAriaGeneralBuffer, size?:number, type?:number):void;
    createFramebuffer():IAriaCoreBuffer;
    clearScreen():void;
    activateFramebuffer(buf:IAriaFramebuffer):void;
    removeFramebuffer():void;
    createEmptyRBO(width:number,height:number):IAriaCoreBuffer;
    createEmptyTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean):IAriaCoreTexture;
    createEmptyCubicTexture(width:number, height:number, mipmap?:boolean, hdr?:boolean):IAriaCoreTexture;
    getCubicLookat(x:number):Float32Array;
    readElementBuffer(destBuffer:ArrayBuffer):void;
    initShaderProgram(vsrc:string,fsrc:string):IAriaCoreShaderProgram|null;
    useShader(shader:IAriaShader, onSuccess?:AriaCallable):void;
    createTexture(img:HTMLImageElement):IAriaCoreTexture;
    readArrayBuffer(destBuffer:ArrayBuffer):void;
    getTextureBufferData(id:IAriaCoreTexture,dataType:number,format:number,w:number,h:number):unknown
    defineUniformCounter(attName:string, increment?:number, returnAfter?:boolean):number;
    defineUniformExtend(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture), index:number):number;
    createCubicTexture(img:HTMLImageElement[]):IAriaCoreTexture;
    getEnv(): IAriaCoreRenderingContext
    useInvariantShader(shader:IAriaShader,effRange:()=>any):void
    renderInstancedEntry(num:number, instances?:number):void
    clearScreenRequest():void;
    withCubicTexture(c:IAriaTexture,callable:()=>any):void;
    withNoDepthMask(callable:()=>any):void;
    clearScreenInternal():void;
    createTexture3D(img:any,w:number,h:number,d:number):IAriaCoreTexture;
    createTextureData2D(img:any,w:number,h:number):IAriaCoreTexture;

    getUniformLocation(shader:IAriaCoreShaderProgram,name:string):IAriaCoreData
    getAttribLocation(shader:IAriaCoreShaderProgram,name:string):number

    createArrayBuffer():IAriaCoreBuffer
    useArrayBuffer(buffer:IAriaCoreBuffer):any
    setArrayBufferData(buffer:IAriaCoreBuffer,data:any):any

    createElementBuffer():IAriaCoreBuffer
    useElementBuffer(buffer:IAriaCoreBuffer):any
    setElementBufferData(buffer:IAriaCoreBuffer,data:any):any

    setViewport(w:number,h:number):any

    generateMipmap2D(tex:IAriaCoreTexture):any
    switchFramebufferAttachmentsForCubemap(renderBuffer:IAriaCoreBuffer,texture:IAriaCoreTexture,order:number):any
    setFramebufferRenderBuffer(renderbuffer:IAriaCoreBuffer):any
    setFramebufferTexture(texture:IAriaCoreTexture):any
    setFramebufferTextureCubic(texture:IAriaCoreTexture):any
}