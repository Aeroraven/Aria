import { IAriaTexture } from "../../../core/interface/IAriaTexture";
import { IAriaGeometry } from "./IAriaGeometry";

export interface IAriaModelContent{
    geometries: IAriaGeometry[]
    textures: IAriaTexture[]
}