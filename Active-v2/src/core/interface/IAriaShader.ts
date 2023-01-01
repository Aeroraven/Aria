export interface IAriaShader{
    getAttribute(key:string):number
    getShaderProgram():WebGLProgram
    getUniform(key:string):WebGLUniformLocation|null
    allocateTexture():number
    use():any
}