import { AriaComRTAbstractGeometry } from "./AriaComRTAbstractGeometry";
import defs from '../../assets/pathtracer/shaders/geometry_sphere/defs.glsl'
import { AriaVec3 } from "../../../core/arithmetic/AriaVector";
import { AriaComRTAbstractMaterial } from "../material/AriaComRTAbstractMaterial";

export class AriaComRTSphere extends AriaComRTAbstractGeometry{
    private center:AriaVec3
    private radius:number
    private material:AriaComRTAbstractMaterial
    private fname:string
    constructor(name:string,center:AriaVec3, radius:number,material:AriaComRTAbstractMaterial){
        super()
        this._rename("AriaCom/RTSphere")
        this.center = center
        this.radius = radius
        this.material = material
        this.fname = name

        this.registerDependency(material)
    }
    public injectDefinition(): string {
        return defs
    }
    public injectRayCollisionCriterion(): string {
        return this.emitScope(()=>{
            let ret = ""
            ret += this.emitStructObj("rts_Sphere",this.fname,{
                center:this.emitVec3(this.center),
                radius:this.radius.toFixed(6)
            })
            ret += "float tc;\n"
            ret += "vec3 tnorm;\n"
            ret += "rtf_SphereRayIntersection("+ this.getArgs().ACRTS_CURRENT_RAY+","+this.fname+",tc,tnorm);\n";
            ret += this.emitUpdateShortestDistance("tc",this.material.getIdentifier(),"tnorm",this.material);
            return ret;
        })
    }
    
}