import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { IAriaFBOContainer } from "./IAriaFBOContainer";
import { IAriaTextureAttached } from "./IAriaTextureAttached";

export interface IAriaCanvasComposeAttributes{
    preserve:boolean
}

export interface IAriaCanavs extends IAriaFBOContainer, IAriaTextureAttached{
    compose(renderer:IAriaRendererCore,proc:()=>any,attrs?:IAriaCanvasComposeAttributes):any
}