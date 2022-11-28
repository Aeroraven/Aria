import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/AriaShaderOps";
import { AriaComLight, AriaLightShaderVars } from "./AriaComLight";

export class AriaComDirectionalLight extends AriaComLight{
    private _lightColor = [1,1,1,1]
    private _lightPosition = [0,0,0]
    constructor(){
        super("Aria/DirectionalLight")
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
        AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_TYPE, AriaShaderUniformTp.ASU_VEC1I, 0, id)
        AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_COLOR, AriaShaderUniformTp.ASU_VEC4, this._lightColor, id)
        AriaShaderOps.defineUniformExtend(AriaLightShaderVars.ALSV_POS, AriaShaderUniformTp.ASU_VEC3, this._lightPosition, id)
    }
}