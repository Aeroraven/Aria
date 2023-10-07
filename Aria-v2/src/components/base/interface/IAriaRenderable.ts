import { IAriaRendererCore } from "../../../core/interface/IAriaRendererCore";

export interface IAriaRenderable<T=void>{
    render(renderer:IAriaRendererCore,preTriggers?:((_:IAriaRendererCore)=>any)[], postTriggers?:((_:IAriaRendererCore)=>any)[]):T
}