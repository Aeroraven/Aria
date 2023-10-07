import { AriaComponent } from "../../core/AriaComponent";
import { AriaVec3, AriaVec4 } from "../../core/arithmetic/AriaVector";
import { AriaComRTAbstractMaterial } from "./material/AriaComRTAbstractMaterial";

enum AriaComRTFragShaderInjectorVariables{
    ACRTS_OPTIMAL_DISTANCE = "bestT",
    ACRTS_CURRENT_RAY = "ray",
    ACRTS_EPS = "epsf",
    ACRTS_PI = "pi",
    ACRTS_OPTIMAL_MATERIAL = "bestMaterial",
    ACRTS_OPTIMAL_NORMAL = "bestNorm",
    ACRTS_UPDATE_MATERIAL_COLOR = "rfMaterialColor",
    ACRTS_UPDATE_EMISSION_COLOR = "rfEmissionColor"
}

export abstract class AriaComRTFragShaderInjector extends AriaComponent{
    private dependencies:AriaComRTFragShaderInjector[] = []

    constructor(){
        super("AriaCom/RTFragShaderInjector")
    }
    public abstract injectDefinition():string;
    public abstract injectRayCollisionCriterion():string;
    public abstract injectRaySpreadingCriterion():string;

    public resolveDependencies(){

    }
    public registerDependency(dep:AriaComRTFragShaderInjector){
        this.dependencies.push(dep)
    }
    public getDependencies(){
        let ret:AriaComRTFragShaderInjector[] = [this]
        for(let d of this.dependencies){
            ret = ret.concat(d.getDependencies())
        }
        return ret
    }

    protected getArgs(){
        return AriaComRTFragShaderInjectorVariables
    }
    protected emitUpdateShortestDistance(distanceVar:string,materialVar:string,normalVar:string,materialData:AriaComRTAbstractMaterial){
        return "if("+distanceVar+"<"+AriaComRTFragShaderInjectorVariables.ACRTS_OPTIMAL_DISTANCE+"&&"+distanceVar+">epsf){\n"
            +AriaComRTFragShaderInjectorVariables.ACRTS_OPTIMAL_DISTANCE+"="+distanceVar+";\n"
            +AriaComRTFragShaderInjectorVariables.ACRTS_OPTIMAL_MATERIAL+"="+materialVar+";\n"
            +AriaComRTFragShaderInjectorVariables.ACRTS_OPTIMAL_NORMAL+"="+normalVar+";\n"
            +materialData.injectMaterialUpdates()
        +"}\n"
    }
    protected emitScope(callable:()=>string){
        return "if(true){\n"+callable()+"}\n"
    }
    protected emitStructObj(type:string, name:string,values:{[key:string]:string}){
        let result = type+" "+name+";"
        for(let key in values){
            result += name+"."+key+"="+values[key]+";"
        }
        return result
    }
    protected emitVec3(vec3:AriaVec3){
        return "vec3("+vec3.at(0).toFixed(6)+","+vec3.at(1).toFixed(6)+","+vec3.at(2).toFixed(6)+")";
    }
    protected emitVec4(vec3:AriaVec4){
        return "vec4("+vec3.at(0).toFixed(6)+","+vec3.at(1).toFixed(6)+","+vec3.at(2).toFixed(6)+","+vec3.at(3).toFixed(6)+")";
    }
}