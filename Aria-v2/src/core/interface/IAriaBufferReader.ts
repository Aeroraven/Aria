import { IAriaRendererCore } from "./IAriaRendererCore";

export interface IAriaBufferReader<T>{
    getRawData(renderer:IAriaRendererCore):T
}