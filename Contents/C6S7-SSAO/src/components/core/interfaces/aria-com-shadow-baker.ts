import { AriaComponent } from "../../base/aria-component";
import { IAriaComLight } from "./aria-com-light";

export interface IAriaComShadowBaker{
    renderLightDepthMap(x:AriaComponent&IAriaComLight,id:number):void
    renderLightDepthMapS(id:number):void
}