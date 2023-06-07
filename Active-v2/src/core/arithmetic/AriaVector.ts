import { AriaObject } from "../base/AriaObject";
import { AriaArithmetic } from "./AriaArithmetic";
import { IAriaLinearStruct } from "./IAriaLinearStruct";


export class AriaVector extends AriaObject implements IAriaLinearStruct{
    protected _value:number[] = []
    protected _dimen:number = 0
    constructor(n:number|number[]|undefined){
        super("AriaVector")
        if(typeof n === "number"){
            this._dimen = n
            for(let i=0;i<n;i++){
                this._value.push(0)
            }
        }else if(n===undefined){
            this._logError("dimension not specified")
        }else{
            this._dimen = n.length
            for(let i=0;i<n.length;i++){
                this._value.push(n[i])
            }
        }
        
    }

    public static create(n?:number|number[]|undefined){
        return new this(n)
    }
    public at(x:number){
        return this._value[x]
    }
    public log(){
        console.log(this._value)
    }
    public add(x:AriaVector|number):AriaVector{
        let ret = new AriaVector(this._dimen)
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                ret._value[i] = this._value[i] + x
            }else{
                ret._value[i] = this._value[i] + x._value[i]
            }
        }
        return ret
    }
    public add_(x:AriaVector|number):AriaVector{
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                this._value[i] = this._value[i] + x
            }else{
                this._value[i] = this._value[i] + x._value[i]
            }
        }
        return this
    }
    public addScaled_(x:AriaVector,s:number){
        for(let i=0;i<this._dimen;i++){
            this._value[i] = this._value[i] + x._value[i] * s
        }
        return this
    }

    public sub(x:AriaVector|number):AriaVector{
        let ret = new AriaVector(this._dimen)
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                ret._value[i] = this._value[i] - x
            }else{
                ret._value[i] = this._value[i] - x._value[i]
            }
        }
        return ret
    }
    public sub_(x:AriaVector|number):AriaVector{
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                this._value[i] = this._value[i] - x
            }else{
                this._value[i] = this._value[i] - x._value[i]
            }
        }
        return this
    }

    public mul(x:AriaVector|number):AriaVector{
        let ret = new AriaVector(this._dimen)
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                ret._value[i] = this._value[i] * x
            }else{
                ret._value[i] = this._value[i] * x._value[i]
            }
        }
        return ret
    }
    public mul_(x:AriaVector|number):AriaVector{
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                this._value[i] = this._value[i] * x
            }else{
                this._value[i] = this._value[i] * x._value[i]
            }
        }
        return this
    }
    
    public div(x:AriaVector|number):AriaVector{
        let ret = new AriaVector(this._dimen)
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                ret._value[i] = this._value[i] / x
            }else{
                ret._value[i] = this._value[i] / x._value[i]
            }
        }
        return ret
    }
    public div_(x:AriaVector|number):AriaVector{
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                this._value[i] = this._value[i] / x
            }else{
                this._value[i] = this._value[i] / x._value[i]
            }
        }
        return this
    }

    public dot(x:AriaVector):number{
        let ret = 0
        for(let i=0;i<this._dimen;i++){
            ret += this._value[i] * x._value[i]
        }
        return ret
    }

    public sum():number{
        let ret = 0
        for(let i=0;i<this._dimen;i++){
            ret += this._value[i] 
        }
        return ret
    }

    public len():number{
        let ret = 0
        for(let i=0;i<this._dimen;i++){
            ret += this._value[i]*this._value[i]
        }
        return Math.sqrt(ret)
    }

    public normalize_(){
        return this.div_(this.len())
    }

    public normalize(){
        let ret = new AriaVector(this._dimen)
        ret.fromArray(this)
        ret.div_(this.len())
        return ret
    }

    public toArray():number[]{
        let ret = []
        for(let i=0;i<this._dimen;i++){
            ret.push(this._value[i])
        }
        return ret
    }
    public fromArray(x:number[]|AriaVector){
        this._value = []
        if(x instanceof AriaVector){
            this._dimen = x._dimen
            for(let i=0;i<this._dimen;i++){
                this._value.push(x._value[i])
            }
        }else{
            this._dimen = x.length
            for(let i=0;i<this._dimen;i++){
                this._value.push(x[i])
            }
        }
    }
    public dim(){
        return this._dimen
    }
    public cross(x:AriaVector){
        return new AriaVector(AriaArithmetic.cross(this._value,x._value))
    }
}

export class AriaVec3 extends AriaVector{
    constructor(n:number[]|number|undefined){
        super(n===undefined?3:n)
        if(this._dimen!=3){
            this._logError("dimension should be 3")
        }
    }
    public static create(n?:number|number[]|undefined){
        return new this(n)
    }
}

export type AriaVec3C = AriaVec3 | number[]
export type AriaVecC = AriaVector | number[]