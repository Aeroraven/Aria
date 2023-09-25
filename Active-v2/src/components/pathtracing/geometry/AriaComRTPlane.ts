import { AriaComRTAbstractGeometry } from "./AriaComRTAbstractGeometry";
import defs from '../../assets/pathtracer/shaders/geometry_plane/defs.glsl'
import { AriaVec3 } from "../../../core/arithmetic/AriaVector";
import { AriaComRTAbstractMaterial } from "../material/AriaComRTAbstractMaterial";

export class AriaComRTPlane extends AriaComRTAbstractGeometry{
    private base:AriaVec3
    private normal:AriaVec3
    private material:AriaComRTAbstractMaterial
    private fname:string
    constructor(name:string,base:AriaVec3,normal:AriaVec3,material:AriaComRTAbstractMaterial){
        super()
        this._rename("AriaCom/RTSphere")
        this.base = base
        this.normal = normal
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
            ret += this.emitStructObj("rts_Plane",this.fname,{
                point:this.emitVec3(this.base),
                normal:this.emitVec3(this.normal),
            })
            ret += "float tc;\n"
            ret += "vec3 tnorm = "+ this.emitVec3(this.normal) +";\n"
            ret += "rtf_PlaneRayIntersection("+ this.getArgs().ACRTS_CURRENT_RAY+","+this.fname+",tc);\n";
            ret += this.emitUpdateShortestDistance("tc",this.material.getIdentifier(),"tnorm");
            return ret;
        })
    }
    
}