import { AriaComShaderMaterial } from "./AriaComShaderMaterial";
import vertex from "../assets/shaders/model-depth/vertex.glsl"
import fragment from "../assets/shaders/model-depth/fragment.glsl"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaDepthMaterial extends AriaComShaderMaterial{
    constructor(){
        super({
            vertex:vertex,
            fragment:fragment
        })
        this._rename("AriaCom/Preset/DepthMaterial")
    }
}