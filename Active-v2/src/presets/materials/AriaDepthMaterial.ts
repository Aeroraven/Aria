import { AriaComShaderMaterial } from "../../components/material/AriaComShaderMaterial";
import vertex from "../shaders/material/model-depth/vertex.glsl"
import fragment from "../shaders/material/model-depth/fragment.glsl"

export class AriaDepthMaterial extends AriaComShaderMaterial{
    constructor(){
        super({
            vertex:vertex,
            fragment:fragment
        })
        this._rename("AriaCom/Preset/DepthMaterial")
    }
}