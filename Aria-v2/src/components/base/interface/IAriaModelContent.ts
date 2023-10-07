import { IAriaTexture } from "../../../core/interface/IAriaTexture";
import { IAriaGeometry } from "./IAriaGeometry";

export interface IAriaModelContent<T=any>{
    geometries: IAriaGeometry[]
    textures: IAriaTexture[]
    bufData: T[]
}