import { AriaComponent } from "../../../core/AriaComponent";
import { AriaMatrix } from "../../../core/arithmetic/AriaMatrix";
import { AriaVector } from "../../../core/arithmetic/AriaVector";
import { AriaComGaussSeidelSolver } from "../../math/equ_solver/AriaComGaussSeidelSolver";
import { AriaPhyFluidGrid2D } from "./AriaPhyFluidGrid2D";

export class AriaPhyFluid2D extends AriaComponent{
    protected initState:AriaPhyFluidGrid2D
    protected state:AriaPhyFluidGrid2D[] = []
    protected iteration:number = 0
    protected diffRate:number
    protected solver:AriaComGaussSeidelSolver = new AriaComGaussSeidelSolver()
    protected iters:number = 100
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
        //let solver = new AriaComGaussSeidelSolver();
        let nVec = this.solver.solve(sMatrix,sVector,this.iters)
        
        let nextState = this.getLastState()
        nextState.setMassV(nVec.toArray())
    }
    performAdvection(timestep=1.0){
        //Fluid Projection

        // (p1+p2+p3+p4)-4p = div(velocity)
        let c = this.getCurrentState()
        const numCells = this.initState.getW()*this.initState.getH()
        let sMatrix = new AriaMatrix(numCells,numCells)
        let sVector = new AriaVector(numCells)

        // Obtaining Pressure Scalar Field
        let divVelocity = c.getVelocityDivergence()
        // console.log(divVelocity)
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
                    if(el!=idx){
                        sMatrix.setVal(idx,el,1)
                    }else{
                        sMatrix.setVal(idx,el,sMatrix.at(idx,el)+1)
                    }
                })
                sVector.setVal(idx,divVelocity[idx])
            }
        }
        //let solver = new AriaComGaussSeidelSolver();
        let nVec = this.solver.solve(sMatrix,sVector,this.iters)

        // Get Gradients of Pressure
        let gradX = new Array<number>(numCells)
        let gradY = new Array<number>(numCells)
        for(let i=0;i<this.initState.getW();i++){
            for(let j=0;j<this.initState.getH();j++){
                let topIdx = c.isValidGrid(i,j-1)?c.getIndex(i,j-1):-1
                let botIdx = c.isValidGrid(i,j+1)?c.getIndex(i,j+1):-1
                let leftIdx = c.isValidGrid(i-1,j)?c.getIndex(i-1,j):-1
                let rightIdx = c.isValidGrid(i+1,j)?c.getIndex(i+1,j):-1
                let curIdx = c.getIndex(i,j)

                let leftp = (leftIdx>=0)? nVec.at(leftIdx):0
                let rightp = (rightIdx>=0)? nVec.at(rightIdx):0
                let topp = (topIdx>=0)? nVec.at(topIdx):0
                let botp = (botIdx>=0)? nVec.at(botIdx):0
                
                gradX[curIdx] = rightp-leftp
                gradY[curIdx] = botp-topp
            }
        }
        //console.log(nVec.toArray())

        let nextState = this.getLastState()
        
        //Advection
        for(let i=0;i<this.initState.getW();i++){
            for(let j=0;j<this.initState.getH();j++){
                let curIdx = c.getIndex(i,j)
                let velocity = c.getVelocity(i,j).add(new AriaVector([gradX[curIdx],gradY[curIdx]]))
                let px = velocity.at(0)*timestep+i
                let py = velocity.at(1)*timestep+j
                
                let ix = Math.ceil(px)
                let iy = Math.ceil(py)
                let fx = px-ix
                let fy = py-iy

                //LT,RT,LB,RB
                let percents = [fx*fy,(1-fx)*fy,fx*(1-fy),(1-fx)*(1-fy)]
                let dirs = [[0,0,0],[1,0,1],[0,1,2],[1,1,3]]
                let dws = 0
                dirs.forEach((el)=>{
                    if(c.isValidGrid(i+el[0],j+el[1])){
                        dws+=percents[2]*c.getMass(i+el[0],j+el[1])
                    }
                })
                nextState.setMass(i,j,nextState.getMass(i,j)+dws)
            }
        }

    }


}