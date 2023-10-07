export class AriaBufferMap{
    mapping:Map<string,WebGLBuffer>
    mappingType:Map<string,number|undefined>
    mappingSize:Map<string,number|undefined>
    numVertices:number
    constructor(){
        this.mapping = new Map()
        this.mappingType = new Map()
        this.mappingSize = new Map()
        this.numVertices = 0
    }
    set(k:string,v:WebGLBuffer,t:number|undefined = undefined, s:number|undefined = undefined){
        this.mapping.set(k,v)
        this.mappingType.set(k,t)
        this.mappingSize.set(k,s)
        //console.log(v)
    }
    get(k:string):WebGLBuffer{
        const r = this.mapping.get(k)
        if(r === undefined){
            throw new Error("Key missing ("+k+")")
        }else{
            return r
        }
    }
    getType(k:string){
        const r = this.mappingType.get(k)
        return r
    }
    getSize(k:string){
        const r = this.mappingSize.get(k)
        return r
    }
    exists(k:string){
        return this.mapping.has(k)
    }
    setNumVertices(v:number){
        this.numVertices = v
    }
    getNumVertices(){
        return this.numVertices
    }
}