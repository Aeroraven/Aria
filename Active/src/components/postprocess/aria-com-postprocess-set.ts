import { AriaComponent } from "../base/aria-component";
import { IAriaRenderable } from "../core/aria-com-mesh";
import { IAriaComLight } from "../core/interfaces/aria-com-light";
import { IAriaComShadowBaker } from "../core/interfaces/aria-com-shadow-baker";

export class AriaComPostprocessPass extends AriaComponent implements IAriaRenderable,IAriaComShadowBaker{
    
    
    renderLightDepthMap(x: AriaComponent & IAriaComLight, id: number): void {
        throw new Error("Method not implemented.");
    }
    renderLightDepthMapS(id: number): void {
        throw new Error("Method not implemented.");
    }
    render(): void {
        throw new Error("Method not implemented.");
    }


}