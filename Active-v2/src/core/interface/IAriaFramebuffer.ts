import { IAriaTextureAttached } from "../../components/base/interface/IAriaTextureAttached";
import { IAriaCoreBuffer } from "../base/AriaRendererCompatDef";
import { IAriaGeneralBuffer } from "./IAriaGeneralBuffer";
import { IAriaRendererCore } from "./IAriaRendererCore";

export interface IAriaFramebuffer extends IAriaGeneralBuffer<IAriaCoreBuffer>, IAriaTextureAttached{
    onRender(renderer:IAriaRendererCore,renderCall:()=>any):void
    onClear(renderer:IAriaRendererCore):void
}