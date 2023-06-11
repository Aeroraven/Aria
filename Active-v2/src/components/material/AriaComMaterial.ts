import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaTexture } from "../../core/interface/IAriaTexture";

export class AriaComMaterial extends AriaComponent implements IAriaShaderEmitter{
    protected _impAttNames: string[] = []
    protected _types: AriaShaderUniformTp[] = []
    protected _values: (number[]|number|Float32Array|IAriaTexture)[] = []
    constructor(name:string){
        super(name)
    }

    public use(renderer:IAriaRendererCore){
        this._logError("use: Method not implemented")
    }

    public exportToShader(renderer:IAriaRendererCore): void {
        for(let i=0;i<this._impAttNames.length;i++){
            renderer.defineUniform(this._impAttNames[i], this._types[i], this._values[i])
        }
    }

    public addParam(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)){
        this._impAttNames.push(attName)
        this._types.push(type)
        this._values.push(value)
    }
}