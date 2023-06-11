import { IAriaRendererCore } from "./IAriaRendererCore"

export interface IAriaShader{
    getAttribute(renderer:IAriaRendererCore,key:string):number
    getShaderProgram():WebGLProgram
    getUniform(renderer:IAriaRendererCore,key:string):WebGLUniformLocation|null
    allocateTexture():number
    use(renderer:IAriaRendererCore):any
}