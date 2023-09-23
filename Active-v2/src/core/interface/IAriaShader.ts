import { IAriaCoreShaderProgram } from "../base/AriaRendererCompatDef"
import { IAriaRendererCore } from "./IAriaRendererCore"

export interface IAriaShader{
    getAttribute(renderer:IAriaRendererCore,key:string):number
    getShaderProgram():IAriaCoreShaderProgram
    getUniform(renderer:IAriaRendererCore,key:string):WebGLUniformLocation|null
    getSide():string
    allocateTexture():number
    use(renderer:IAriaRendererCore):any
}