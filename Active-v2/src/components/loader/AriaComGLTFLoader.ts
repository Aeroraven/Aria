import * as gltf from 'webgl-gltf';
import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from '../../core/graphics/AriaEnv';
import { AriaShaderOps } from '../../core/graphics/AriaShaderOps';
import { AriaComEBO } from '../base/AriaComEBO';
import { AriaComTexture } from '../base/AriaComTexture';
import { AriaComVAO } from '../base/AriaComVAO';
import { IAriaModelContent } from '../base/interface/IAriaModelContent';
import { AriaGeometryVars } from '../geometry/base/AriaComGeometry';
import { AriaComLoadedGeometry } from '../geometry/primary/AriaComLoadedGeometry';
import { IAriaRendererCore } from '../../core/interface/IAriaRendererCore';

interface AriaLoaderGLTFBuf{
    buffer: WebGLBuffer,
    size:number,
    type:number
}

interface AriaLoaderGLTFMeshData{
    position:Float32Array
    elements:Uint16Array
}

export class AriaComGLTFLoader extends AriaComponent{
    model: gltf.Model | null = null
    mesh:gltf.Mesh | null = null
    renderer:IAriaRendererCore|null = null

    constructor(){
        super("AriaCom/GLTFLoader")
    }
    public async load(renderer:IAriaRendererCore, path:string){
        const gl = renderer.getEnv()
        this.renderer = renderer
        const model =  await gltf.loadModel(gl,path)
        const mesh = model.meshes[<number>model.nodes[0].mesh]
        this.model = model
        this.mesh = mesh
        const ret:IAriaModelContent<AriaLoaderGLTFMeshData> = {
            textures:[],
            geometries: [],
            bufData:[]
        }
        this._logInfo("Found " +this.getTotalMeshes() + " Meshes")
        for(let i=0;i<this.getTotalMeshes();i++){
            const ng = new AriaComLoadedGeometry()

            const eleBufT = this.getElementBuffer(i)
            const eleBuf = new AriaComEBO(eleBufT,this.getElements(i))
            const eleBufRaw = eleBuf.getRawData(renderer)

            const posBufT = this.getPosBuffer(i)
            const posBufMax = (()=>{
                let idx = -1
                for(let i=0;i<eleBufRaw.length;i++){
                    idx = Math.max(eleBufRaw[i],idx)
                }
                return idx+1
            })()
            const posBuf = new AriaComVAO(posBufT.buffer,posBufMax)

            const texBufT = this.getTexBuffer(i)
            const texBuf = new AriaComVAO(texBufT.buffer)

            const normBufT = this.getNormalBuffer(i)
            const normBuf = new AriaComVAO(normBufT.buffer)

            ret.bufData.push({
                position:posBuf.getRawData(renderer),
                elements:eleBuf.getRawData(renderer)
            })

            ng.record((renderer:IAriaRendererCore)=>{
                renderer.defineAttribute(AriaGeometryVars.AGV_POSITION, posBuf, posBufT.size, posBufT.type)
                renderer.defineAttribute(AriaGeometryVars.AGV_TEXTURE_POSITION, texBuf, texBufT.size, texBufT.type)
                renderer.defineAttribute(AriaGeometryVars.AGV_NORMAL, normBuf, normBufT.size, normBufT.type)
                eleBuf.bind(renderer)
            },this.getElements(i))
            ret.geometries.push(ng)

            const tx = new AriaComTexture(renderer)
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
        const gl = this.renderer?.getEnv()!
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
        const gl = this.renderer?.getEnv()!
        if(this.model?.meshes[id].texCoord?.buffer instanceof WebGLBuffer){
            //console.log(this.model?.meshes[id].texCoord?.size, this.model?.meshes[id].texCoord?.type)
            //console.log(gl.FLOAT,gl.INT,gl.UNSIGNED_INT)
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
        //console.log(this.model?.materials)
        const w = <WebGLTexture> this.model?.materials[this.model?.meshes[id].material].baseColorTexture
        //console.log(w)
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