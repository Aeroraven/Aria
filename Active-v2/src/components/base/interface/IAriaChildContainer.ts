import { AriaComponent } from "../../../core/AriaComponent";

export interface IAriaChildContainer<T=AriaComponent,U=void>{
    addChild(c:T):U
}