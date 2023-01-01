import { IAriaFBOContainer } from "./IAriaFBOContainer";
import { IAriaTextureAttached } from "./IAriaTextureAttached";

export interface IAriaCanvasComposeAttributes{
    preserve:boolean
}

export interface IAriaCanavs extends IAriaFBOContainer, IAriaTextureAttached{
    compose(proc:()=>any,attrs?:IAriaCanvasComposeAttributes):any
}