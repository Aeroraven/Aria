import { IAriaTextureAttached } from "../../components/base/interface/IAriaTextureAttached";
import { IAriaGLBuffer } from "./IAriaGLBuffer";

export interface IAriaFramebuffer extends IAriaGLBuffer<WebGLFramebuffer>, IAriaTextureAttached{
    onRender(renderCall:()=>any):void
    onClear():void
}