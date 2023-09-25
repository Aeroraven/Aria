import { AriaComponent } from "../../core/AriaComponent";
import { AriaComRTFragShaderInjector } from "./AriaComRTFragShaderInjector";
import fragmentRaw from '../assets/pathtracer/shaders/base/fragment.glsl'

export class AriaComRTFragShaderProcessor extends AriaComponent{
    private child:AriaComRTFragShaderInjector[] = []
    constructor(){
        super("AriaCom/RTFragShaderInjector")
    }
    public add(child:AriaComRTFragShaderInjector){
        this.child.push(child)
    }
    private _generateCodes(child:AriaComRTFragShaderInjector[]){
        let map = new Set<typeof AriaComRTFragShaderInjector>()
        let defs = ""
        for(let c of child){
            let deps = c.getDependencies()
            for(let d of deps){
                if(map.has(d.constructor as typeof AriaComRTFragShaderInjector))continue
                map.add(d.constructor as typeof AriaComRTFragShaderInjector)
                defs += d.injectDefinition() + "\n"
            }
        }

        let hits = ""
        for(let c of child){
            hits += c.injectRayCollisionCriterion() + "\n"
        }

        return {
            "definitions":defs,
            "hits":hits
        }
    }
    public generateCodes(){
        let codes = this._generateCodes(this.child)
        let rawFragShader = fragmentRaw
        rawFragShader = rawFragShader.replace("//!@asuPathTracerDefinitionInjection()",codes.definitions)
        rawFragShader = rawFragShader.replace("//!@asuPathTracerHitTestInjection()",codes.hits)
        return rawFragShader
    }
}