import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps } from "../../core/graphics/AriaShaderOps";
import { AriaObjArray } from "../../core/base/AriaBaseDefs";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCamera } from "../camera/AriaComCamera";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export enum AriaLightShaderVars{
    ALSV_POS = "uLightPos",
    ALSV_COLOR = "uLightColor",
    ALSV_TYPE = "uLightType",
    ALSV_SHADOW_MAP_TYPE = "uShadowMapType",
    ALSV_SHADOW_MAP = "uShadowMap",
    ALSV_SHADOW_MAP_CUBE = "uShadowMapCube"
}

export class AriaComLight extends AriaComponent implements IAriaShaderEmitter{
    protected _shaderId:number = -1
    protected _validLight = false
    protected _camera = new AriaComCamera()
    protected _shadowMap:IAriaCanavs|null = null
    constructor(name:string){
        super(name)
    }
    exportToShader(renderer:IAriaRendererCore): void {
        this._shaderId = renderer.defineUniformCounter("uLightCount")
        if(this._validLight==false){
            this._logError("light: exportToShader not implemented")
        }
    }
    renderShadowMap(renderer:IAriaRendererCore,renderables:AriaObjArray<IAriaRenderable>,quality:number=1){
        this._logError("light: renderShadowMap not implemented")
    }
    generateShadowMap(renderer:IAriaRendererCore,renderables:AriaObjArray<IAriaRenderable>,quality:number=1):IAriaCanavs{
        this._logError("light: generateShadowMap not implemented")
        throw Error()
    }
}