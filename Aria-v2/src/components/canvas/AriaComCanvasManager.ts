import { AriaComponent } from "../../core/AriaComponent";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { AriaComCanvas } from "./AriaComCanvas";
import { AriaComDefaultCanvas } from "./AriaComDefaultCanvas";

export class AriaComCanvasManager extends AriaComponent{
    private static _instance:AriaComCanvasManager|null = null
    private _activeCanvas: IAriaCanavs[] = []
    private _enablePush: boolean = true
    public static getInstance(){
        if(this._instance==null){
            this._instance = new this()
        }
        return this._instance!
    }
    private constructor(){
        super("AriaCom/CanvasManager")
        this._activeCanvas.push(new AriaComDefaultCanvas())
    }
    public detachCanvas(renderer:IAriaRendererCore){
        this._enablePush = false
        this._activeCanvas.pop()
        this.getCanvas().canvasUse(renderer)
        this._enablePush = true
    }
    public setCanvas(w:IAriaCanavs|null){
        if(this._enablePush){
            this._activeCanvas.push( (w==null)?(new AriaComDefaultCanvas()):w)
        }
    }
    private getCanvas(){
        if(this._activeCanvas.length==0){
            this._logError("Cannot use empty canvas")
        }
        return this._activeCanvas![this._activeCanvas.length-1]
    }

}