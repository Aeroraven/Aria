import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";
import { AriaComGeometry } from "../base/AriaComGeometry";

export class AriaComLoadedGeometry extends AriaComGeometry{
    private _rec:(renderer:IAriaRendererCore)=>any = (renderer:IAriaRendererCore)=>{}
    private _elements:number = 0

    constructor(){
        super("AriaCom/LoadedGeometry")
        this._valid = true
    }
    record(proc:(renderer:IAriaRendererCore)=>any, nums:number){
        this._logInfo("Recorded")
        this._rec = proc
        this._elements = nums
    }
    public exportToShader(renderer: IAriaRendererCore): void {
        super.exportToShader(renderer)
        this._rec(renderer)
    }
    public getVertexNumber(): number {
        return this._elements
    }
}