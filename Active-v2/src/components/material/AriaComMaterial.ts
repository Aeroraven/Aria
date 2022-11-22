import { AriaComponent } from "../../core/AriaComponent";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/AriaShaderOps";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaTexture } from "../../core/interface/IAriaTexture";

export class AriaComMaterial extends AriaComponent implements IAriaShaderEmitter{
    protected _impAttNames: string[] = []
    protected _types: AriaShaderUniformTp[] = []
    protected _values: (number[]|number|Float32Array|IAriaTexture)[] = []
    constructor(name:string){
        super(name)
    }

    public use(){
        this._logError("use: Method not implemented")
    }

    public exportToShader(): void {
        for(let i=0;i<this._impAttNames.length;i++){
            AriaShaderOps.defineUniform(this._impAttNames[i], this._types[i], this._values[i])
        }
    }

    public addParam(attName:string, type:AriaShaderUniformTp, value:(number[]|number|Float32Array|IAriaTexture)){
        this._impAttNames.push(attName)
        this._types.push(type)
        this._values.push(value)
    }
}