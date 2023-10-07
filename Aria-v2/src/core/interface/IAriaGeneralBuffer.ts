import { IAriaCoreBuffer } from "../base/AriaRendererCompatDef"
import { IAriaRendererCore } from "./IAriaRendererCore"

export interface IAriaGeneralBuffer<T=IAriaCoreBuffer>{
    bind(renderer:IAriaRendererCore):void
    unbind(renderer:IAriaRendererCore):void
    getGeneralBufferObject():T
}