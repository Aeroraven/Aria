import { IAriaRendererCore } from "./IAriaRendererCore"

export interface IAriaTexture{
    getTex(renderer:IAriaRendererCore):WebGLTexture
    setTex(c:WebGLTexture):void
}