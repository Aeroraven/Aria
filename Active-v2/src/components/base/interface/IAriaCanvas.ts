import { IAriaFBOContainer } from "./IAriaFBOContainer";
import { IAriaTextureAttached } from "./IAriaTextureAttached";

export interface IAriaCanavs extends IAriaFBOContainer, IAriaTextureAttached{
    compose(proc:()=>any):any
}