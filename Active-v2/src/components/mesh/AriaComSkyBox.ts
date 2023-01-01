import { AriaComMesh } from "./AriaComMesh";
import vertex from '../assets/shaders/skybox/vertex.glsl'
import fragment from '../assets/shaders/skybox/fragment.glsl'
import { AriaComShaderMaterial } from "../material/AriaComShaderMaterial";
import { AriaComCube } from "../geometry/AriaComCube";
import { IAriaTexture } from "../../core/interface/IAriaTexture";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";

export class AriaComSkyBox extends AriaComMesh{
    private _cubeMap:IAriaTexture|null = null
    constructor(){
        super()
        this._rename("AriaCom/SkyBox")
        const material = new AriaComShaderMaterial({
            vertex:vertex,
            fragment:fragment
        })
        this.setMaterial(material)
        const geometry = new AriaComCube()
        this.setGeometry(geometry)
    }
    public setTexture(c:IAriaTexture){
        this._cubeMap = c
    }
    public render(preTriggers?: (() => any)[] | undefined, postTriggers?: (() => any)[] | undefined): void {
        const pt = (preTriggers)?preTriggers:[]
        if(this._cubeMap!=null){
            AriaRenderOps.withCubicTexture(this._cubeMap,()=>{
                AriaRenderOps.withNoDepthMask(()=>{
                    super.render(pt.concat([
                        ()=>{
                            AriaShaderOps.defineUniform("uSkybox",AriaShaderUniformTp.ASU_TEXCUBE,this._cubeMap!)
                        }
                    ]))
                })
            })
        }else{
            this._logError("cube map should be specified for skybox")
        }
    }

}