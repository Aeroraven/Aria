import { AriaComponent } from "../../../core/AriaComponent";

export interface IAriaComponentContainer<T=AriaComponent,U=void>{
    addComponent(c:T):U
}