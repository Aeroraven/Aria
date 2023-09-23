import { IAriaCoreTexture } from "../base/AriaRendererCompatDef"
import { IAriaRendererCore } from "./IAriaRendererCore"
import { IAriaTextureArgs } from "./IAriaTextureArgs"

export interface IAriaTexture{
    getTex(renderer:IAriaRendererCore):IAriaCoreTexture
    setTex(c:IAriaCoreTexture,args?:IAriaTextureArgs):void
}