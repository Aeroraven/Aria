import { AriaObject } from "../base/AriaObject";
import { AriaVecC, AriaVector } from "./AriaVector";
import { IAriaLinearStruct } from "./IAriaLinearStruct";

export class AriaMatrix extends AriaObject implements IAriaLinearStruct{
    private _rows = 0
    private _cols = 0
    private _value:number[] = []

    constructor(...args:number[]|number[][][]|AriaMatrix[]|AriaVector[]|number[][]){
        super("AriaMatrix")
        if(args.length==2){
            if(typeof args[0] == 'number' && typeof args[1] == 'number'){
                this._rows = args[0]
                this._cols = args[1]
                for(let i=0;i<this._rows*this._cols ;i++){
                    this._value.push(0)
                }
            }else{
                this._logError("expected arguments: (number[][]) or (AriaMatrix) or (number,number) or (AriaVector)")
            }
        }else if(args.length==1){
            let rp = args[0]
            if(rp instanceof AriaMatrix){
                this._rows = rp._rows
                this._cols = rp._cols
                for(let i=0;i<rp._cols*rp._rows;i++){
                    this._value.push(rp._value[i])
                }
            }else if(rp instanceof AriaVector){
                this._rows = rp.dim()
                this._cols = 1
                for(let i=0;i<rp.dim();i++){
                    this._value.push(rp.at(i))
                }
            }else if(typeof rp != "number"){
                if(typeof rp[0] == 'number'){
                    rp = <number[]>rp
                    this._rows = rp.length
                    this._cols = 1
                    for(let i=0;i<rp.length;i++){
                        this._value.push(rp[i])
                    }
                }else{
                    rp = <number[][]>rp
                    this._rows = rp.length
                    this._cols = rp[0].length
                    for(let i=0;i<this._rows;i++){
                        for(let j=0;j<this._cols;j++){
                            this._value[i*this._cols+j] = rp[i][j]
                        }
                    }
                }
                
            }else{
                this._logError("expected arguments: (number[][]) or (AriaMatrix) or (number,number) or (AriaVector)")
            }
        }
    }
    protected _sameShape(x:AriaMatrix){
        return this._cols == x._cols && this._rows == x._rows
    }
    public static identity(x:number){
        let ret = new AriaMatrix(x,x)
        for(let i=0;i<x;i++){
            ret._value[i*x+i] = x 
        }
        return ret
    }
    public static ones(...args:number[]){
        if(args.length!=1&&args.length!=2){
            this._logErrorS("expected arguments: (number) or (number,number)")
        }
        let ret = (args.length == 1 ? new AriaMatrix(args[0],args[0]) : (args.length == 2 ? new AriaMatrix(args[0],args[1]) : undefined))!
        for(let i=0;i<ret._value.length;i++){
            ret._value[i] = 1
        }
    }
    public static zeros(...args:number[]){
        if(args.length!=1&&args.length!=2){
            this._logErrorS("expected arguments: (number) or (number,number)")
        }
        let ret = (args.length == 1 ? new AriaMatrix(args[0],args[0]) : (args.length == 2 ? new AriaMatrix(args[0],args[1]) : undefined))!
        for(let i=0;i<ret._value.length;i++){
            ret._value[i] = 0
        }
    }

    public fromMatrix(x:AriaMatrix){
        let rp = x
        this._rows = rp._rows
        this._cols = rp._cols
        this._value = []
        for(let i=0;i<rp._cols*rp._rows;i++){
            this._value.push(rp._value[i])
        }
    }
    public fromVector(x:AriaVecC){
        x = x instanceof AriaVector ? x : new AriaVector(x)
        this._rows = x.dim()
        this._cols = 1
        for(let i=0;i<x.dim();i++){
            this._value.push(x.at(i))
        }
    }
    public toVector(){
        this._assert(this._cols == 1, "column should be 1")
        return new AriaVector(this._value)
    }

    public at(r:number,c:number){
        return this._value[r*this._cols+c]
    }
    public setVal(r:number,c:number,v:number){
        this._value[r*this._cols+c] = v
    }


    public add_(x:AriaMatrix){
        this._assert(this._sameShape(x),"matrices have the same shape")
        for(let i=0;i<this._value.length;i++){
            this._value[i] += x._value[i]
        }
        return this
    }
    public add(x:AriaMatrix){
        this._assert(this._sameShape(x),"matrices have the same shape")
        let ret = new AriaMatrix(this)
        for(let i=0;i<this._value.length;i++){
            ret._value[i] += x._value[i]
        }
        return ret
    }
    public mul(x:AriaMatrix|AriaVector){
        x = x instanceof AriaVector? new AriaMatrix(x) : x
        this._assert(this._cols == x._rows,"columns of the first matrix should be equal to the rows of the second matrix")
        let ret = new AriaMatrix(this._rows,x._cols)
        for(let i=0;i<this._rows;i++){
            for(let j=0;j<x._cols;j++){
                for(let k=0;k<this._cols;k++){
                    ret._value[i*x._cols+j] += this.at(i,k)*x.at(k,j)
                }
            }
        }
        return ret
    }
    public transform(x:AriaVector){
        let r = this.mul(x)
        return r.toVector()
    }
    public mul_(x:AriaMatrix|AriaVector){
        x = x instanceof AriaVector? new AriaMatrix(x) : x
        this._assert(this._cols == x._rows ,"columns of the first matrix should be equal to the rows of the second matrix")
        let ret = new AriaMatrix(this._rows,x._cols)
        for(let i=0;i<this._rows;i++){
            for(let j=0;j<x._cols;j++){
                for(let k=0;k<this._cols;k++){
                    ret._value[i*x._cols+j] += this.at(i,k)*x.at(k,j)
                }
            }
        }
        this.fromMatrix(ret)
        return this
    }
    public t(){
        let ret = new AriaMatrix(this._cols,this._rows)
        for(let i=0;i<this._rows;i++){
            for(let j=0;j<this._cols;j++){
                ret.setVal(j,i,this.at(i,j))
            }
        }
        return ret
    }
    public t_(){
        this.fromMatrix(this.t())
        return this
    }

    public algcof(r:number,c:number){
        this._assert(this._rows == this._cols,"operation should be performed on a square matrix")
        this._assert(this._rows>1,"operation cannot be performed on a scalar")
        let ret = new AriaMatrix(this._rows-1,this._cols-1)
        let sing = (r+c)%2==0
        for(let i=0;i<this._rows-1;i++){
            for(let j=0;j<this._cols-1;j++){
                ret.setVal(i,j,this.at(i>=r?i+1:i,j>=c?j+1:j)*(sing?-1:1))
            }
        }
        return ret
    }
    public det(){
        this._assert(this._rows == this._cols,"operation should be performed on a square matrix")
        if(this._rows==1){
            return this._value[0]
        }else{
            let res = 0
            for(let i=0;i<this._cols;i++){
                res += this.algcof(0,i).det() * this.at(0,i)
            }
            return res
        }
    }
    public adj(){
        let ret = new AriaMatrix(this._rows,this._cols)
        for(let i=0;i<this._rows;i++){
            for(let j=0;j<this._cols;j++){
                ret.setVal(j,i,this.algcof(i,j).det())
            }
        }
        return ret
    }
    public inv(){
        let ret = new AriaMatrix(this._rows,this._cols)
        let det = this.det()
        this._assert(det!=0,"the determinant of the matrix should not be zero")
        for(let i=0;i<this._rows;i++){
            for(let j=0;j<this._cols;j++){
                ret.setVal(j,i,this.algcof(i,j).det()/det)
            }
        }
        return ret
    }
    public inv_(){
        this.fromMatrix(this.inv())
        return this
    }
}