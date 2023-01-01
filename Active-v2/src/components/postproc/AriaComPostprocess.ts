import { AriaComponent } from "../../core/AriaComponent";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaTexture } from "../../core/interface/IAriaTexture";
import { IAriaChildContainer } from "../base/interface/IAriaChildContainer";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCanvas } from "../canvas/AriaComCanvas";

class AriaComPostprocessEmitter extends AriaComponent implements IAriaShaderEmitter{
    private _tex:IAriaTexture
    constructor(tex:IAriaTexture){
        super("AriaCom/PostprocessEmitter")
        this._tex = tex
    }
    exportToShader(): void {
    }
}

export class AriaComPostprocess extends AriaComponent implements IAriaRenderable<void>, IAriaChildContainer<IAriaRenderable>{
    private _child:IAriaRenderable[] = []
    private _canvasOption = {
        scaler:1,
        hdr:true
    }
    private _canvas:AriaComCanvas[] = []
    private _inputCanvas:AriaComCanvas|null = null

    constructor(){
        super("AriaCom/Postprocess")
    }
    setEntryCanvas(c:AriaComCanvas){
        this._inputCanvas = c
    }
    addChild(c: IAriaRenderable<void>): void {
        this._child.push(c)
    }
    render(preTriggers?: (() => any)[] | undefined, postTriggers?: (() => any)[] | undefined): void {
        if(this._canvas.length==0){
            this._canvas.push((new AriaComCanvas(this._canvasOption.scaler, this._canvasOption.hdr)))
            this._canvas.push((new AriaComCanvas(this._canvasOption.scaler, this._canvasOption.hdr)))
        }

    }
}