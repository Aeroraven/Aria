export interface IAriaCoreTexture<T=any>{
    data:T
}

export interface IAriaCoreBuffer<T=any>{
    data:T
}

export interface IAriaCoreShaderProgram<T=any>{
    data:T
}

export interface IAriaCoreRenderingContext<T=any>{
    data:T
}

export interface IAriaCoreData<T=any>{
    data:T
}

export class AriaRendererCompatUtils{
    public static castBufferUnsafe<T>(x:IAriaCoreBuffer){
        return <T>(x.data)
    }
    public static castTextureUnsafe<T>(x:IAriaCoreTexture){
        return <T>(x.data)
    }
    public static castShaderProgramUnsafe<T>(x:IAriaCoreShaderProgram){
        return <T>(x.data)
    }
    public static castContextUnsafe<T>(x:IAriaCoreRenderingContext){
        return <T>(x.data)
    }
    public static createShaderProgram<T>(x:T):IAriaCoreShaderProgram<T>{
        return {data:x}
    }
    public static createTexture<T>(x:T):IAriaCoreShaderProgram<T>{
        return {data:x}
    }
    public static createTexturePlaceholder():IAriaCoreShaderProgram<any>{
        return {data:null}
    }
    public static createBuffer<T>(x:T):IAriaCoreBuffer<T>{
        return {data:x}
    }
    public static createData<T>(x:T):IAriaCoreData<T>{
        return {data:x}
    }
    public static createBufferPlaceholder():IAriaCoreBuffer<any>{
        return {data:null}
    }
}