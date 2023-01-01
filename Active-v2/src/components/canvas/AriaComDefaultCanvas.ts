import { AriaComManager } from "../../core/AriaComManager";
import { AriaComponent } from "../../core/AriaComponent";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
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
    canvasUse(): void {
        AriaRenderOps.fboUnbind()
        AriaComCanvasManager.getInstance().setCanvas(this)
    }
    canvasDetach(): void {
        this._logWarn("Default canvas cannot be detached. Operation ignored.")
        
    }
    compose(proc: () => any) {
        this.canvasUse()
        proc()
    }
}