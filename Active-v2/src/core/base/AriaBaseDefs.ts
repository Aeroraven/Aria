export type AriaCallable<T=any> = ()=>T
export type AriaObjArray<T> = T|T[]
export type AriaObjArrayIterCallable<T> = (x:T)=>any

export class AriaRef<T>{
    private _value:T
    constructor(x:T){
        this._value = x
    }
    public set value(v : T) {
        this._value = v;
    }
    public get value() : T {
        return this._value
    }
}