import { AriaComponent } from "../../../../core/AriaComponent";

export class AriaSpatialHashTable<T> extends AriaComponent{
    private _buckets:T[][] = []
    private _bucketsCoords:number[][][] = []
    private _blen = 0
    constructor(buckets:number){
        super("AriaComSpatialHashTable")
        for(let i=0;i<buckets;i++){
            this._buckets.push([])
            this._bucketsCoords.push([])
        }
        this._blen = buckets
    }
    private hashFunction(x:number,y:number,z:number){
        return (((x*73856093)^(y*19349663)^(z*83492791))%this._blen+this._blen)%this._blen
    }
    public getItem(x:number,y:number,z:number){
        let c = this.hashFunction(x,y,z)
        let w = this._buckets[c]
        for(let i=0;i<w.length;i++){
            let dc = this._bucketsCoords[c][i]
            if(dc[0]==x&&dc[1]==y&&dc[2]==z){
                return w[i]
            }
        }
        return null
    }
    public setItem(x:number,y:number,z:number,v:T){
        let c = this.hashFunction(x,y,z)
        this._buckets[c].push(v)
        this._bucketsCoords[c].push([x,y,z])
    }

}