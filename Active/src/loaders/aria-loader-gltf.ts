import * as gltf from 'webgl-gltf';

export interface AriaLoaderGLTFBuf{
    buffer: WebGLBuffer,
    size:number,
    type:number
}
export class AriaLoaderGLTF {
    model: gltf.Model | null
    mesh:gltf.Mesh | null
    gl: WebGL2RenderingContext

    constructor(gl:WebGL2RenderingContext){
        this.model = null
        this.mesh = null
        this.gl = gl
    }
    async loadModel(path:string){
        this.model =  await gltf.loadModel(this.gl,path)
        this.mesh = this.model.meshes[<number>this.model.nodes[0].mesh]
    }
    getTotalMeshes(){
        return this.model?.meshes.length || 0
    }
    getElements(id:number){
        return <number>this.model?.meshes[id].elementCount
    }
    getPosBuffer(id:number):AriaLoaderGLTFBuf{
        if(this.model?.meshes[id].positions.buffer instanceof WebGLBuffer){
            return {
                buffer: <WebGLBuffer>this.model?.meshes[id].positions.buffer,
                size: this.model?.meshes[id].positions.size,
                type: this.model?.meshes[id].positions.type
            } 
        }else{
            throw new Error("Not invalid buffer")
        }
    }
    getNormalBuffer(id:number):AriaLoaderGLTFBuf{
        if(this.model?.meshes[id].normals?.buffer instanceof WebGLBuffer){
            
            return {
                buffer: <WebGLBuffer>this.model?.meshes[id].normals?.buffer,
                size: this.model?.meshes[id].normals?.size || 3,
                type: this.model?.meshes[id].normals?.type || this.gl.FLOAT
            } 
        }else{
            throw new Error("Not invalid buffer")
        }
    }
    getTexBuffer(id:number):AriaLoaderGLTFBuf{
        if(this.model?.meshes[id].texCoord?.buffer instanceof WebGLBuffer){
            console.log(this.model?.meshes[id].texCoord?.size, this.model?.meshes[id].texCoord?.type)
            console.log(this.gl.FLOAT,this.gl.INT,this.gl.UNSIGNED_INT)
            return {
                buffer: <WebGLBuffer>this.model?.meshes[id].texCoord?.buffer,
                size: this.model?.meshes[id].texCoord?.size || 2,
                type: this.model?.meshes[id].texCoord?.type || this.gl.FLOAT
            } 
        }else{
            throw new Error("Not invalid buffer")
        }
    }
    getBaseMaterialTexture(id:number){
        return <WebGLTexture> this.model?.materials[this.model?.meshes[id].material].baseColorTexture
    }
    getElementBuffer(id:number){
        if(this.model?.meshes[id].indices instanceof WebGLBuffer){
            return <WebGLBuffer>this.model?.meshes[id].indices
        }else{
            throw new Error("Not invalid buffer")
        }
    }
}