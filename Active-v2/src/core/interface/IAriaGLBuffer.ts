import { IAriaRendererCore } from "./IAriaRendererCore"

export interface IAriaGLBuffer<T=WebGLBuffer>{
    bind(renderer:IAriaRendererCore):void
    unbind(renderer:IAriaRendererCore):void
    getGLObject():T
}