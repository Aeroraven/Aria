import { mat4, vec3 } from "../../../node_modules/gl-matrix-ts/dist/index";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { IAriaComFrameUpdateable } from "../core/interfaces/aria-com-frame-updateable";
import { IAriaComGeometry } from "./aria-com-geometry";

export class AriaComFluidSurface extends AriaComponent implements IAriaComGeometry,IAriaComFrameUpdateable {
    cx: number
    cy: number
    cz: number
    sc: number
    de: number
    orgv: number[]
    elv:number[]
    colv:number[]
    texv:number[]
    norv:number[]
    rot: Float32Array

    velocity: number
    distance: number
    interval: number
    resistance: number

    pbuf:number[][]
    pcurBuf:number

    posBuffer:WebGLBuffer
    normBuffer:WebGLBuffer
    eleBuffer:WebGLBuffer
    
    protected constructor(gl: WebGL2RenderingContext) {
        super(gl);
        this.cx = 0
        this.cy = 0
        this.cz = 0
        this.sc = 0
        this.rot = mat4.create()
        this.de = 20
        this.orgv = []
        this.elv = []
        this.colv = []
        this.texv = []
        this.norv = []
        this.velocity = 0
        this.distance = 0
        this.interval = 0
        this.resistance = 0
        mat4.identity(this.rot)
        this.pbuf = []
        this.pcurBuf = 0

        this.posBuffer = <WebGLBuffer>gl.createBuffer();
        this.eleBuffer = <WebGLBuffer>gl.createBuffer();
        this.normBuffer = <WebGLBuffer>gl.createBuffer();
    }

    public addPointInterference(dx:number,dy:number,deltaY:number){
        this.pbuf[this.pcurBuf][(dx*this.de+dy)*3+1] = deltaY
    }

    public initFluid(){
        this.pbuf = [[],[]]
        this.pbuf.forEach((buf)=>{
            for(let i=0;i<this.de;i++){
                for(let j=0;j<this.de;j++){
                    let lt = [i/this.de*this.sc,0,j/this.de*this.sc]
                    lt.forEach((el)=>{
                        buf.push(el)
                    })
                }
            }
        })
        return this
    }

    public generateCurFluid(){
        this.orgv = []
        this.elv = []
        this.texv = []
        this.norv = []
        let idx = 0;

        const yCurr = (x:number,z:number) =>{
            if(x>=this.de||x<0||z>=this.de||z<0){
                return 0
            }
            let idx = (x*this.de+z)*3;
            return this.pbuf[1-this.pcurBuf][idx+1]
        }
        const getNorm = (x:number,z:number) =>{
            let dx = -(yCurr(x+1,z)-yCurr(x-1,z))/(2*this.distance)
            let dy = 1
            let dz = -(yCurr(x,z+1)-yCurr(x,z-1))/(2*this.distance)
            return [dx,dy,dz]
        }

        for(let i=0;i<this.de;i++){
            for(let j=0;j<this.de;j++){
                let lts = (i*this.de+j)*3
                let lt = [this.pbuf[this.pcurBuf][lts],this.pbuf[this.pcurBuf][lts+1],this.pbuf[this.pcurBuf][lts+2]]
                let a1 = getNorm(i,j)
                this.orgv.push(lt[0],yCurr(i,j),lt[2])
                this.texv.push(0,0)
                this.norv.push(a1[0],a1[1],a1[2])
            }
        }
        const gid = (di:number,dj:number)=>{
            return (di*this.de+dj)
        }
        for(let i=0;i<this.de-1;i++){
            for(let j=0;j<this.de-1;j++){
                let ida = gid(i,j)
                let idb = gid(i+1,j)
                let idc = gid(i+1,j+1)
                let idd = gid(i,j+1)
                this.elv.push(ida,idb,idc,idc,idd,ida)
                idx+=6
            }
        }
    }

    public frameUpdate(){

        //Generate Next Frame
        let curBuf = this.pbuf[this.pcurBuf]
        let prevBuf = this.pbuf[1-this.pcurBuf]

        const yCurr = (x:number,z:number) =>{
            if(x>=this.de||x<=0||z>=this.de||z<=0){
                return 0
            }
            let idx = (x*this.de+z)*3;
            if(isNaN(curBuf[idx+1])){
                console.log(x,z,idx+1)
                console.log(curBuf)
            }
            return curBuf[idx+1]
        }

        const yPrev = (x:number,z:number) =>{
            let idx = (x*this.de+z)*3;
            return prevBuf[idx+1]
        }

        const setYNext = (x:number,z:number,v:number) =>{
            let idx = (x*this.de+z)*3;
            prevBuf[idx+1] = v
        }
        
        let t2 = this.interval * this.interval
        let c3 = (this.velocity*this.velocity)/(this.distance*this.distance)
        let sharedDeno = 1/(this.interval*this.interval) + this.resistance/(2*this.interval)
        for(let i=0;i<this.de;i++){
            for(let j=0;j<this.de;j++){
                let term1 = (2*yCurr(i,j)-yPrev(i,j))/t2
                let term2 = yPrev(i,j)*this.resistance*0.5/this.interval
                let term3 = c3 * (yCurr(i+1,j)+yCurr(i-1,j)+yCurr(i,j+1)+yCurr(i,j-1)-4*yCurr(i,j))
                setYNext(i,j,(term1+term2+term3)/sharedDeno)
                if(isNaN((term1+term2+term3)/sharedDeno)){
                    console.log(term1,term2,term3,sharedDeno,c3)
                    throw Error("Fuck NaN")
                }
            }
        }

        //Generate Buffer for Current Frame
        this.generateCurFluid()
        this.regenBuffers()

        //Swap Buffer
        this.pcurBuf = 1 - this.pcurBuf
    }

    public translate(dx: number, dy: number, dz: number) {
        this.cx = dx
        this.cy = dy
        this.cz = dz
        return this
    }

    public scale(sc: number) {
        this.sc = sc
        return this
    }

    public setFluidParams(v:number, d:number, t:number, mu: number){
        this.velocity = v
        this.distance = d
        this.interval = t
        this.resistance = mu
        return this
    }

    public setDensity(nb:number){
        const p = <AriaComBuffers>this.parent
        this.de = nb
        return this
    }

    public regenBuffers(){
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl

            gl.deleteBuffer(this.posBuffer)
            this.posBuffer = gl.createBuffer()!
            const posBuffer = this.posBuffer
            gl.bindBuffer(gl.ARRAY_BUFFER, posBuffer)
            const vertex:number[] = []
            for(let i=0;i<this.orgv.length;i+=3){
                vertex.push(this.orgv[i]*this.sc+this.cx);
                vertex.push(this.orgv[i+1]*this.sc+this.cy);
                vertex.push(this.orgv[i+2]*this.sc+this.cz);
            }
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertex), gl.STATIC_DRAW)
            p.getBuffer().set("pos", posBuffer)

            gl.deleteBuffer(this.eleBuffer)
            this.eleBuffer = gl.createBuffer()!
            const eleBuffer = this.eleBuffer
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(this.elv), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            gl.deleteBuffer(this.normBuffer)
            this.normBuffer = gl.createBuffer()!
            const normBuffer = this.normBuffer
            gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(this.norv),gl.STATIC_DRAW)
            p.getBuffer().set("norm", normBuffer)

            p.getBuffer().setNumVertices((this.de-1)*(this.de-1)*6)

            console.log(this.de*this.de*6,this.elv.length)

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }

    protected override register(): void {
        this.frameUpdate()
    }
}