import { IAriaRendererCore } from "./IAriaRendererCore"

export interface IAriaShader{
    getAttribute(renderer:IAriaRendererCore,key:string):number
    getShaderProgram():WebGLProgram
    getUniform(renderer:IAriaRendererCore,key:string):WebGLUniformLocation|null
    getSide():string
    allocateTexture():number
    use(renderer:IAriaRendererCore):any
}