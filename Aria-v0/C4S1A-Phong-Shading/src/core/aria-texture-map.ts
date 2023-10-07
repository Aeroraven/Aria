export class AriaTextureMap{
    mapping:Map<string,WebGLTexture>
    numVertices:number
    constructor(){
        this.mapping = new Map()
        this.numVertices = 0
    }
    set(k:string,v:WebGLTexture){
        this.mapping.set(k,v)
    }
    get(k:string):WebGLTexture{
        const r = this.mapping.get(k)
        if(r === undefined){
            throw new Error("Key missing")
        }else{
            return r
        }
    }
    setNumVertices(v:number){
        this.numVertices = v
    }
    getNumVertices(){
        return this.numVertices
    }
}