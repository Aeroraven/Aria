import { AriaComGeometry } from "./AriaComGeometry";

export class AriaComLoadedGeometry extends AriaComGeometry{
    private _rec:()=>any = ()=>{}
    private _elements:number = 0

    constructor(){
        super("AriaCom/LoadedGeometry")
    }
    record(proc:()=>any, nums:number){
        this._logInfo("Recorded")
        this._rec = proc
        this._elements = nums
    }
    public exportToShader(): void {
        this._rec()
    }
    public getVertexNumber(): number {
        return this._elements
    }
}