import { AriaComponent } from "../../../core/AriaComponent";
import { AriaMatrix } from "../../../core/arithmetic/AriaMatrix";
import { AriaVector } from "../../../core/arithmetic/AriaVector";
import { AriaComGaussSeidelSolver } from "../../math/equ_solver/AriaComGaussSeidelSolver";
import { AriaPhyFluidGrid2D } from "./AriaPhyFluidGrid2D";

export class AriaPhyFluid2D extends AriaComponent{
    protected initState:AriaPhyFluidGrid2D
    protected state:AriaPhyFluidGrid2D[] = []
    protected iteration:number = 0
    protected diffRate:number = 0
    constructor(initialState:AriaPhyFluidGrid2D,diffusionRate=0.01){
        super("AriaPhy/Fluid2D")
        this.initState = initialState
        this.state.push(this.initState.duplicate())
        this.state.push(this.initState.duplicate())
        this.diffRate = diffusionRate
    }
    getCurrentState(){
        return this.state[this.iteration&1]
    }
    getLastState(){
        return this.state[1-(this.iteration&1)]
    }
    proceed(){
        this.performDiffusion()
        this.performAdvection()
        this.performProjection()
        this.iteration++
    }
    performDiffusion(){
        //Fluid Diffusion

        // dn = dc+k((d1c+d2c+d3c+d4c)/4-dc)
        // dc = dn-k((d1n+d2n+d3n+d4n)/4-dn) => (1+k)dn-(d1n+d2n+d3n+d4n)k/4
        // (1+k)dn = dc+(d1n+d2n+d3n+d4n)k/4
        // dn = [dc+(d1n+d2n+d3n+d4n)k/4]/(1+k)

        let c = this.getCurrentState()
        const numCells = this.initState.getW()*this.initState.getH()
        let sMatrix = new AriaMatrix(numCells,numCells)
        let sVector = new AriaVector(numCells)

        //Build matrix
        for(let i=0;i<this.initState.getW();i++){
            for(let j=0;j<this.initState.getH();j++){
                let adjGrids:number[] = []
                let adjDirs = [[-1,0],[1,0],[0,1],[0,-1]]
                adjDirs.forEach((el)=>{
                    if(c.isValidGrid(i+el[0],j+el[1])){
                        adjGrids.push(c.getIndex(i+el[0],j+el[1]))
                    }
                })
                let idx = c.getIndex(i,j)
                sMatrix.setVal(idx,idx,1)
                adjGrids.forEach((el)=>{
                    sMatrix.setVal(idx,el,-this.diffRate/(1.0+this.diffRate)/adjGrids.length)
                })
                sVector.setVal(idx,c.getMass(i,j)/(1.0+this.diffRate))
            }
        }
        let solver = new AriaComGaussSeidelSolver();
        let nVec = solver.solve(sMatrix,sVector,55)
        
        let nextState = this.getLastState()
        nextState.setMassV(nVec.toArray())
    }
    performProjection(){
        //Fluid Projection

        // (p1+p2+p3+p4)-4p = div(velocity)


        let c = this.getCurrentState()
        const numCells = this.initState.getW()*this.initState.getH()
        let sMatrix = new AriaMatrix(numCells,numCells)
        let sVector = new AriaVector(numCells)

        // Obtaining Pressure Scalar Field
        let divVelocity = c.getVelocityDivergence()
        for(let i=0;i<this.initState.getW();i++){
            for(let j=0;j<this.initState.getH();j++){
                let adjGrids:number[] = []
                let adjDirs = [[-1,0],[1,0],[0,1],[0,-1]]
                adjDirs.forEach((el)=>{
                    if(c.isValidGrid(i+el[0],j+el[1])){
                        adjGrids.push(c.getIndex(i+el[0],j+el[1]))
                    }else{
                        adjGrids.push(c.getIndex(i,j))
                    }
                })
                let idx = c.getIndex(i,j)
                sMatrix.setVal(idx,idx,-4)
                adjGrids.forEach((el)=>{
                    sMatrix.setVal(idx,el,1)
                })
                sVector.setVal(idx,divVelocity[idx])
            }
        }
        let solver = new AriaComGaussSeidelSolver();
        let nVec = solver.solve(sMatrix,sVector,55)

        // Get Gradients of Pressure
        

    }
    performAdvection(){

    }

}