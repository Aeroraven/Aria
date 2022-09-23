export class AriaBufferMap{
    mapping:Map<string,WebGLBuffer>
    numVertices:number
    constructor(){
        this.mapping = new Map()
        this.numVertices = 0
    }
    set(k:string,v:WebGLBuffer){
        this.mapping.set(k,v)
    }
    get(k:string):WebGLBuffer{
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