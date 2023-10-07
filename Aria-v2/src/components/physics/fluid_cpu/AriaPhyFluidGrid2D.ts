import { AriaComponent } from "../../../core/AriaComponent";
import { AriaVec2, AriaVec2C } from "../../../core/arithmetic/AriaVector";

export class AriaPhyFluidGrid2D extends AriaComponent{
    protected velocity:AriaVec2[]
    protected mass:number[]
    protected width:number
    protected height:number

    constructor(w:number,h:number){
        super("AriaPhy/FluidGrid2D")
        this.width = w
        this.height = h
        this.mass = new Array<number>(this.width*this.height)
        this.velocity = new Array<AriaVec2>(this.width*this.height)
        for(let i=0;i<this.width*this.height;i++){
            this.velocity[i] = AriaVec2.create()
            this.mass[i] = 0
        }
    }
    isValidGrid(x:number,y:number){
        if(x<0||x>=this.width||y<0||y>=this.height){
            return false
        }
        return true
    }
    getW(){
        return this.width
    }
    getH(){
        return this.height
    }
    getMassV(){
        return this.mass
    }
    getIndex(x:number,y:number){
        return x*this.height+y
    }
    getMass(x:number,y:number){
        if(this.mass[x*this.height+y]==undefined){
            this._logError("aria.phy.fluid2dgrid: undefined")
        }
        return this.mass[x*this.height+y]
    }
    setMass(x:number,y:number,v:number){
        this.mass[x*this.height+y] = v
    }
    setMassV(vector:number[]){
        this.mass = vector
    }
    getVelocity(x:number,y:number){
        return this.velocity[x*this.height+y]
    }
    setVelocity(x:number,y:number,v:AriaVec2C){
        this.velocity[x*this.height+y].fromArray(v)
    }
    duplicate(){
        let x = new AriaPhyFluidGrid2D(this.width,this.height)
        for(let i=0;i<this.width*this.height;i++){
            x.mass[i] = this.mass[i]
            x.velocity[i].fromArray(this.velocity[i])
        }
        return x
    }

    getVelocityDivergence(){
        //Discrete Divergence for Velocity Field
        let div = new Array<number>(this.width*this.height)
        for(let i=0;i<this.width;i++){
            for(let j=0;j<this.height;j++){
                let right = this.isValidGrid(i+1,j)?this.getVelocity(i+1,j).at(0):0
                let left = this.isValidGrid(i-1,j)?this.getVelocity(i-1,j).at(0):0
                let top = this.isValidGrid(i,j-1)?this.getVelocity(i,j-1).at(1):0
                let bottom = this.isValidGrid(i,j+1)?this.getVelocity(i,j+1).at(1):0
                div[i*this.height+j] = (right-left)+(bottom-top)
            }
        }
        return div
    }

}