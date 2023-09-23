import { IAriaRendererCore } from "./IAriaRendererCore"
import { IAriaTextureArgs } from "./IAriaTextureArgs"

export interface IAriaTexture{
    getTex(renderer:IAriaRendererCore):WebGLTexture
    setTex(c:WebGLTexture,args?:IAriaTextureArgs):void
    
}