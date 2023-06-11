import { IAriaTextureAttached } from "../../components/base/interface/IAriaTextureAttached";
import { IAriaGLBuffer } from "./IAriaGLBuffer";
import { IAriaRendererCore } from "./IAriaRendererCore";

export interface IAriaFramebuffer extends IAriaGLBuffer<WebGLFramebuffer>, IAriaTextureAttached{
    onRender(renderer:IAriaRendererCore,renderCall:()=>any):void
    onClear(renderer:IAriaRendererCore):void
}