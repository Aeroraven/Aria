import { AriaCallable, AriaObjArray } from "../../../core/base/AriaBaseDefs";
import { IAriaRenderable } from "./IAriaRenderable";

export interface IAriaRenderInitiatorOptions{
    preTriggers?:AriaCallable[]
    postTriggers?:AriaCallable[]
}

export interface IAriaRenderInitiator<T=void>{
    initiateRender(renderables:AriaObjArray<IAriaRenderable>, options:IAriaRenderInitiatorOptions):T
}