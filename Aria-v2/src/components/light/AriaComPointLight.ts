import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { AriaObjArray } from "../../core/base/AriaBaseDefs";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCubicCanvas } from "../canvas/AriaComCubicCanvas";
import { AriaDepthMaterial } from "../material/AriaDepthMaterial";
import { AriaComLight, AriaLightShaderVars } from "./AriaComLight";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaAuxiliaryOps } from "../../core/AriaAuxiliaryOps";

export class AriaComPointLight extends AriaComLight{
    private _lightColor = [1,1,1,1]
    private _lightPosition = [0,0,0]
    private _material = new AriaDepthMaterial()
    constructor(){
        super("Aria/PointLight")
        this._validLight = true
    }
    public setLightColor(r:number, g:number, b:number, a:number){
        this._lightColor[0] = r
        this._lightColor[1] = g
        this._lightColor[2] = b
        this._lightColor[3] = a
    }
    public setLightPosition(x:number, y:number, z:number){
        this._lightPosition[0] = x
        this._lightPosition[1] = y
        this._lightPosition[2] = z
    }
    public exportToShader(renderer:IAriaRendererCore): void {
        super.exportToShader(renderer)
        const id = this._shaderId-1
        renderer.defineUniformExtend(AriaLightShaderVars.ALSV_TYPE, AriaShaderUniformTp.ASU_VEC1I, 1, id)
        renderer.defineUniformExtend(AriaLightShaderVars.ALSV_COLOR, AriaShaderUniformTp.ASU_VEC4, this._lightColor, id)
        renderer.defineUniformExtend(AriaLightShaderVars.ALSV_POS, AriaShaderUniformTp.ASU_VEC3, this._lightPosition, id)
        if(this._shadowMap!==null){
            renderer.defineUniformExtend(AriaLightShaderVars.ALSV_SHADOW_MAP_CUBE,AriaShaderUniformTp.ASU_TEXCUBE, this._shadowMap.getTex(),id)
            renderer.defineUniformExtend(AriaLightShaderVars.ALSV_SHADOW_MAP_TYPE,AriaShaderUniformTp.ASU_VEC1I, 2 ,id)
        }
    }
    public override renderShadowMap(renderer:IAriaRendererCore,renderables: AriaObjArray<IAriaRenderable<void>>,quality:number=1): void {
        this._camera.setPos(this._lightPosition[0],this._lightPosition[1],this._lightPosition[2])
        this._camera.setFov(90.0)
        this._camera.setAspect(1.0)
        this._material.withShader(renderer,()=>{
            this._camera.exportToShader(renderer)
            AriaAuxiliaryOps.iterateObjArray(renderables,(x)=>{
                x.render(renderer)
            })
        })
    }
    public override generateShadowMap(renderer:IAriaRendererCore,renderables: AriaObjArray<IAriaRenderable<void>>,quality:number=1): IAriaCanavs {
        if(this._shadowMap === null){
            this._shadowMap = new AriaComCubicCanvas(quality)
        }
        this._shadowMap.compose(renderer,()=>[
            this.renderShadowMap(renderer,renderables,1)
        ])
        return this._shadowMap
    }
}