import { AriaComShaderMaterial } from "./AriaComShaderMaterial";
import vertex from "../assets/shaders/model-depth/vertex.glsl"
import fragment from "../assets/shaders/model-depth/fragment.glsl"

export class AriaDepthMaterial extends AriaComShaderMaterial{
    constructor(){
        super({
            vertex:vertex,
            fragment:fragment
        })
        this._rename("AriaCom/Preset/DepthMaterial")
    }
}