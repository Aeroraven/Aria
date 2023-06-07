import { AriaComponent } from "../../../core/AriaComponent";
import { IAriaDynamicGeometry } from "../../base/interface/IAriaDynamicGeometry";

export class AriaPhyVirtualGeometry extends AriaComponent implements IAriaDynamicGeometry {
    private rotx:number = 0
    private roty:number = 0
    private rotz:number = 0
    private posx:number = 0
    private posy:number = 0
    private posz:number = 0

    constructor(){
        super("AriaPhy/VirtualGeometry")
    }
    getLocalPosition(): number[] {
        return [this.posx,this.posy,this.posz]
    }
    localRotateX(r: number): void {
        this.rotx+=r
    }
    localRotateY(r: number): void {
        this.roty+=r
    }
    localRotateZ(r: number): void {
        this.rotz+=r
    }
    localTranslate(x: number, y: number, z: number): void {
        this.posx+=x
        this.posy+=y
        this.posz+=z
    }
    localTranslateAbsolute(x: number, y: number, z: number): void {
        this.posx+=x
        this.posy+=y
        this.posz+=z
    }
    localPosition(x: number, y: number, z: number): void {
        this.posx=x
        this.posy=y
        this.posz=z
    }
    
}