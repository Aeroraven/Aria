export interface IAriaGLBuffer<T=WebGLBuffer>{
    bind():void
    unbind():void
    getGLObject():T
}