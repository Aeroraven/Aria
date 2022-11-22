export interface IAriaShader{
    getAttribute(key:string):number
    getUniform(key:string):WebGLUniformLocation|null
    allocateTexture():number
    use():any
}