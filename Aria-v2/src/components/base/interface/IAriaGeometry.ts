import { IAriaShaderEmitter } from "../../../core/interface/IAriaShaderEmitter";
import { AriaRenderEnumDrawingShape } from "../../../core/renderer/AriaRendererEnums";

export interface IAriaGeometry extends IAriaShaderEmitter{
    getVertexNumber():number
    getRenderShape():AriaRenderEnumDrawingShape
}