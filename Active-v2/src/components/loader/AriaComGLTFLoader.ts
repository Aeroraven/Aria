import * as gltf from 'webgl-gltf';
import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from '../../core/AriaEnv';
import { AriaShaderOps } from '../../core/AriaShaderOps';
import { AriaComEBO } from '../base/AriaComEBO';
import { AriaComTexture } from '../base/AriaComTexture';
import { AriaComVAO } from '../base/AriaComVAO';
import { IAriaModelContent } from '../base/interface/IAriaModelContent';
import { AriaGeometryVars } from '../geometry/AriaComGeometry';
import { AriaComLoadedGeometry } from '../geometry/AriaComLoadedGeometry';

interface AriaLoaderGLTFBuf{
    buffer: WebGLBuffer,
    size:number,
    type:number
}

export class AriaComGLTFLoader extends AriaComponent{
    model: gltf.Model | null = null
    mesh:gltf.Mesh | null = null

    constructor(){
        super("AriaCom/GLTFLoader")
    }
    public async load(path:string){
        const gl = AriaEnv.env
        const model =  await gltf.loadModel(gl,path)
        const mesh = model.meshes[<number>model.nodes[0].mesh]
        this.model = model
        this.mesh = mesh
        const ret:IAriaModelContent = {
            textures:[],
            geometries: []
        }
        this._logInfo("Found " +this.getTotalMeshes() + " Meshes")
        for(let i=0;i<this.getTotalMeshes();i++){
            const ng = new AriaComLoadedGeometry()

            const posBufT = this.getPosBuffer(i)
            const posBuf = new AriaComVAO(posBufT.buffer)

            const texBufT = this.getTexBuffer(i)
            const texBuf = new AriaComVAO(texBufT.buffer)

            const normBufT = this.getNormalBuffer(i)
            const normBuf = new AriaComVAO(normBufT.buffer)

            const eleBufT = this.getElementBuffer(i)
            const eleBuf = new AriaComEBO(eleBufT)

            ng.record(()=>{
                AriaShaderOps.defineAttribute(AriaGeometryVars.AGV_POSITION, posBuf, posBufT.size, posBufT.type)
                AriaShaderOps.defineAttribute(AriaGeometryVars.AGV_TEXTURE_POSITION, texBuf, texBufT.size, texBufT.type)
                AriaShaderOps.defineAttribute(AriaGeometryVars.AGV_NORMAL, normBuf, normBufT.size, normBufT.type)
                eleBuf.bind()
            },this.getElements(i))
            ret.geometries.push(ng)

            const tx = new AriaComTexture()
            tx.setTex(this.getBaseMaterialTexture(i))
            ret.textures.push(tx)
        }
        return ret
    }

    private getTotalMeshes(){
        return this.model?.meshes.length || 0
    }
    private getElements(id:number){
        return <number>this.model?.meshes[id].elementCount
    }
    private getPosBuffer(id:number):AriaLoaderGLTFBuf{
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
    private getNormalBuffer(id:number):AriaLoaderGLTFBuf{
        const gl = AriaEnv.env
        if(this.model?.meshes[id].normals?.buffer instanceof WebGLBuffer){
            
            return {
                buffer: <WebGLBuffer>this.model?.meshes[id].normals?.buffer,
                size: this.model?.meshes[id].normals?.size || 3,
                type: this.model?.meshes[id].normals?.type || gl.FLOAT
            } 
        }else{
            throw new Error("Not invalid buffer")
        }
    }
    private getTexBuffer(id:number):AriaLoaderGLTFBuf{
        const gl = AriaEnv.env
        if(this.model?.meshes[id].texCoord?.buffer instanceof WebGLBuffer){
            console.log(this.model?.meshes[id].texCoord?.size, this.model?.meshes[id].texCoord?.type)
            console.log(gl.FLOAT,gl.INT,gl.UNSIGNED_INT)
            return {
                buffer: <WebGLBuffer>this.model?.meshes[id].texCoord?.buffer,
                size: this.model?.meshes[id].texCoord?.size || 2,
                type: this.model?.meshes[id].texCoord?.type || gl.FLOAT
            } 
        }else{
            throw new Error("Not invalid buffer")
        }
    }
    private getBaseMaterialTexture(id:number){
        const w = <WebGLTexture> this.model?.materials[this.model?.meshes[id].material].baseColorTexture
        console.log(w)
        return w
    }
    private getElementBuffer(id:number){
        if(this.model?.meshes[id].indices instanceof WebGLBuffer){
            return <WebGLBuffer>this.model?.meshes[id].indices
        }else{
            throw new Error("Not invalid buffer")
        }
    }
}