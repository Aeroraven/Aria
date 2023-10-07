import { AriaComponent } from "../../core/AriaComponent";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { AriaComTexture } from "../base/AriaComTexture";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { AriaComCanvasManager } from "./AriaComCanvasManager";

export class AriaComDefaultCanvas extends AriaComponent implements IAriaCanavs{
    constructor(){
        super("AriaCom/DefaultCanvas")
        this._logWarn("Using default canvas")
    }
    getTex(): AriaComTexture {
        this._logError("Default canvas cannot be exported as texture")
        return <AriaComTexture><unknown>null;
    }
    canvasUse(renderer:IAriaRendererCore): void {
        //AriaRenderOps.fboUnbind()
        AriaComCanvasManager.getInstance().setCanvas(this)
    }
    canvasDetach(renderer:IAriaRendererCore): void {
        this._logWarn("Default canvas cannot be detached. Operation ignored.")
        
    }
    compose(renderer:IAriaRendererCore,proc: () => any) {
        this.canvasUse(renderer)
        proc()
    }
}