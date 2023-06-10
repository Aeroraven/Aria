import { AriaObject } from "../base/AriaObject";
import { AriaMatrix } from "./AriaMatrix";
import { AriaVec3, AriaVec3C, AriaVector } from "./AriaVector";

export class AriaQuaternion extends AriaObject{
    private readonly _dimen = 4
    private _value = [0,0,0,0]
    constructor(x?:AriaVector|number[]){
        super("AriaQuaternion")
        if(x){
            if(x instanceof AriaVector){
                if(x.dim()!=4){
                    this._logError("dimension should be 4")
                }
                for(let i=0;i<4;i++){
                    this._value[i] = x.at(i)
                }
            }else{
                if(x.length!=4){
                    this._logError("dimension should be 4")
                }
                for(let i=0;i<4;i++){
                    this._value[i] = x[i]
                }
            }
        }
    }

    public static identity(){
        return new AriaQuaternion([1,0,0,0])
    }

    public at(x:number){
        return this._value[x]
    }

    public real(){
        return this._value[0]
    }

    public imag(x:number){
        return this._value[x+1]
    }

    public len():number{
        let ret = 0
        for(let i=0;i<this._dimen;i++){
            ret += this._value[i]*this._value[i]
        }
        return Math.sqrt(ret)
    }

    public add(x:AriaQuaternion):AriaQuaternion{
        let ret = new AriaQuaternion()
        for(let i=0;i<this._dimen;i++){
            ret._value[i] = this._value[i] + x._value[i]
        }
        return ret
    }

    public add_(x:AriaQuaternion):AriaQuaternion{
        for(let i=0;i<this._dimen;i++){
            this._value[i] = this._value[i] + x._value[i]
        }
        return this
    }

    public conj(){
        let ret = new AriaQuaternion([this.real(),-this.imag(0),-this.imag(1),-this.imag(2)])
        return ret
    }

    public conj_(){
        this._value[1] = - this._value[1]
        this._value[2] = - this._value[2]
        this._value[3] = - this._value[3]
        return this
    }

    public mul_(x:AriaQuaternion|number){
        if(typeof x == 'number'){
            for(let i=0;i<4;i++){
                this._value[i] *= x
            }
            return this
        }else{
            let di = this.imag(1) * x.imag(2) - this.imag(2) * x.imag(1) + x.real() * this.imag(0) + this.real() * x.imag(0)
            let dj = this.imag(2) * x.imag(0) - this.imag(0) * x.imag(2) + x.real() * this.imag(1) + this.real() * x.imag(1)
            let dk = this.imag(0) * x.imag(1) - this.imag(1) * x.imag(0) + x.real() * this.imag(2) + this.real() * x.imag(2)
            let dw = this.real() * x.real() - this.imag(0) * x.imag(0) - this.imag(1) * this.imag(1) - this.imag(2) * this.imag(2)
            this._value[0] = dw
            this._value[1] = di
            this._value[2] = dj
            this._value[3] = dk
            return this
        }
        
    }
    
    public mul(x:AriaQuaternion|number){
        if(typeof x == 'number'){
            return new AriaQuaternion([this.at(0)*x,this.at(1)*x,this.at(2)*x,this.at(3)*x])
        }else{
            let di = this.imag(1) * x.imag(2) - this.imag(2) * x.imag(1) + x.real() * this.imag(0) + this.real() * x.imag(0)
            let dj = this.imag(2) * x.imag(0) - this.imag(0) * x.imag(2) + x.real() * this.imag(1) + this.real() * x.imag(1)
            let dk = this.imag(0) * x.imag(1) - this.imag(1) * x.imag(0) + x.real() * this.imag(2) + this.real() * x.imag(2)
            let dw = this.real() * x.real() - this.imag(0) * x.imag(0) - this.imag(1) * this.imag(1) - this.imag(2) * this.imag(2)
            return new AriaQuaternion([dw,di,dj,dk])
        }
        
    }
    public toRotMatrix(){
        let ret = new AriaMatrix(
            [
                [1-2*(this.y*this.y+this.z*this.z),2*(this.x*this.y+this.z*this.w),2*(this.x*this.z-this.y*this.w)],
                [2*(this.x*this.y-this.z*this.w),1-2*(this.x*this.x+this.z*this.z),2*(this.y*this.y+this.x*this.w)],
                [2*(this.x*this.z+this.y*this.w),2*(this.y*this.z-this.x*this.w),1-2*(this.x*this.x+this.y*this.y)]
            ]
        )
        return ret
    }

    public fromAxisAngle(axis:AriaVec3C,angle:number){
        let _axis = AriaVec3.create()
        _axis.fromArray(axis)
        _axis.normalize_()
        this._value[0] = Math.cos(angle)
        this._value[1] = Math.sin(angle) * _axis.at(0)
        this._value[2] = Math.sin(angle) * _axis.at(1)
        this._value[3] = Math.sin(angle) * _axis.at(2)
    }

    public fromVector(x:AriaVector){
        if(x.dim()!=4){
            this._logError("dimension should be 4")
        }
        for(let i=0;i<4;i++){
            this._value[i] = x.at(i)
        }
    }

    public toVector(){
        return new AriaVector(this._value)
    }

    public get x(){
        return this.imag(0)
    }
    public get y(){
        return this.imag(1)
    }
    public get z(){
        return this.imag(2)
    }
    public get w(){
        return this.real()
    }

    public rdiv_(x:number){
        for(let i=0;i<this._dimen;i++){
            if(typeof x === 'number'){
                this._value[i] = this._value[i] / x
            }
        }
        return this
    }

    public normalize_(){
        return this.rdiv_(this.len())
    }


    public toRotTranslateMatrix(position:AriaVec3C,homography:boolean=true){
        position = position instanceof AriaVec3 ? position : new AriaVec3(position)
        let sp = [
            [1-2*(this.y*this.y+this.z*this.z),2*(this.x*this.y+this.z*this.w),2*(this.x*this.z-this.y*this.w),position.at(0)],
            [2*(this.x*this.y-this.z*this.w),1-2*(this.x*this.x+this.z*this.z),2*(this.y*this.y+this.x*this.w),position.at(1)],
            [2*(this.x*this.z+this.y*this.w),2*(this.y*this.z-this.x*this.w),1-2*(this.x*this.x+this.y*this.y),position.at(2)],
        ]
        if(homography){
            sp.push([0,0,0,1])
        }
        let ret = new AriaMatrix(sp)
        return ret
    }
}