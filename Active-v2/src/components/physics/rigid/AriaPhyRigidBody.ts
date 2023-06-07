import { AriaComponent } from "../../../core/AriaComponent";
import { AriaQuaternion } from "../../../core/arithmetic/AriaQuaternion";

export class AriaPhyRigidBody extends AriaComponent{
    private _mass:number = 0
    private _orientation:AriaQuaternion = new AriaQuaternion()
    constructor(){
        super("AriaPhy/RigidBody")
    }
}