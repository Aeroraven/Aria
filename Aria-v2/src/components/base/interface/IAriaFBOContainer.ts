import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore"

export interface IAriaFBOContainer{
    canvasUse(renderer:IAriaRendererCore):void
    canvasDetach(renderer:IAriaRendererCore):void
}