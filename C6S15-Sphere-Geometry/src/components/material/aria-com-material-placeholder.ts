import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComMaterial } from "../core/interfaces/aria-com-material";

export class AriaComMaterialPlaceholder extends AriaComponent implements IAriaComMaterial{
    emitUniforms(shader: AriaShader): void {
        
    }
}