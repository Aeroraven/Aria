import { AriaComponent } from "../../core/AriaComponent"
import { AriaSpatialHashTable } from "../base/algo/hash/AriaSpatialHashTable"

export interface AriaVoxelAttr{
    tsdf:number
    valid:boolean
}

export interface AriaVoxelAttrRecord<T=AriaVoxelAttr>{
    data:T
    pos:number[]
}

export class AriaVoxel<T=AriaVoxelAttr> extends AriaComponent{
    private _hashTable: AriaSpatialHashTable<T>
    private _list: AriaVoxelAttrRecord<T>[] = []
    protected _mag:number = 1
    constructor(mag:number = 1){
        super("AriaCom/Voxel")
        this._mag = mag
        this._hashTable = new AriaSpatialHashTable<T>(10)
    }
    public getMag(){
        return this._mag
    }
    public getVoxel(x:number,y:number,z:number){
        return this._hashTable.getItem(x*this._mag,y*this._mag,z*this._mag)
    }
    public setVoxel(x:number,y:number,z:number,v:T){
        if(this.getVoxel(x*this._mag,y*this._mag,z*this._mag)!=null){
            return
        }
        this._list.push({
            data:v,
            pos:[x,y,z]
        })
        return this._hashTable.setItem(x*this._mag,y*this._mag,z*this._mag,v)
    }
    public getAllVoxels(){
        return this._list
    }
}