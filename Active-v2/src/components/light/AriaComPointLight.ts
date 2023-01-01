import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { AriaObjArray } from "../../core/base/AriaBaseDefs";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCubicCanvas } from "../canvas/AriaComCubicCanvas";
import { AriaDepthMaterial } from "../material/AriaDepthMaterial";
import { AriaComLight, AriaLightShaderVars } from "./AriaComLight";

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
    public exportToShader(): void {
        super.exportToShader()
        const id = this._shaderId-1
        AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_TYPE, AriaShaderUniformTp.ASU_VEC1I, 1, id)
        AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_COLOR, AriaShaderUniformTp.ASU_VEC4, this._lightColor, id)
        AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_POS, AriaShaderUniformTp.ASU_VEC3, this._lightPosition, id)
        if(this._shadowMap!==null){
            AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_SHADOW_MAP_CUBE,AriaShaderUniformTp.ASU_TEXCUBE, this._shadowMap.getTex(),id)
            AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_SHADOW_MAP_TYPE,AriaShaderUniformTp.ASU_VEC1I, 2 ,id)
        }
    }
    public override renderShadowMap(renderables: AriaObjArray<IAriaRenderable<void>>): void {
        this._camera.setPos(this._lightPosition[0],this._lightPosition[1],this._lightPosition[2])
        this._camera.setFov(90.0)
        this._camera.setAspect(1.0)
        this._material.withShader(()=>{
            this._camera.initiateRender(renderables)
        })
    }
    public override generateShadowMap(renderables: AriaObjArray<IAriaRenderable<void>>): IAriaCanavs {
        if(this._shadowMap === null){
            this._shadowMap = new AriaComCubicCanvas()
        }
        this._shadowMap.compose(()=>[
            this.renderShadowMap(renderables)
        ])
        return this._shadowMap
    }
}