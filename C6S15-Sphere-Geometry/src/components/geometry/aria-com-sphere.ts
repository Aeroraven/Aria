import { mat4, vec3 } from "../../../node_modules/gl-matrix-ts/dist/index";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { IAriaComGeometry } from "./aria-com-geometry";

export class AriaComSphere extends AriaComponent implements IAriaComGeometry {
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
        mat4.identity(this.rot)
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

    public setDensity(nb:number){
        const p = <AriaComBuffers>this.parent
        this.de = nb
        this.orgv = []
        this.elv = []
        this.texv = []
        this.norv = []
        let idx = 0;
        
        let deltaChange = 2*Math.PI/nb;

        const getCoord = (da:number,dc:number)=>{
            let y = this.sc * Math.sin(dc) + this.cy
            let xz = this.sc * Math.cos(dc)
            let x = xz * Math.cos(da) + this.cx
            let z = xz * Math.sin(da) + this.cz
            return [x,y,z]
        }
        const getCoordN = (da:number,dc:number)=>{
            let y = Math.sin(dc)
            let xz = Math.cos(dc)
            let x = xz * Math.cos(da) 
            let z = xz * Math.sin(da) 
            return [x,y,z]
        }



        const getCoordX = (di:number, dj:number)=>{
            return getCoord(di*deltaChange,dj*deltaChange)
        }

        const getCoordXN = (di:number, dj:number)=>{
            return getCoordN(di*deltaChange,dj*deltaChange)
        }

        const appendVec3 = (x:number[],ap:number[])=>{
            x.push(ap[0],ap[1],ap[2])
            return x
        }

        for(let i=0;i<this.de;i++){
            for(let j=0;j<this.de;j++){

                let lta = getCoordX(i,j)
                let ltb = getCoordX(i+1,j)
                let ltc = getCoordX(i+1,j+1)
                let ltd = getCoordX(i,j+1)

                let ltan = getCoordXN(i,j)
                let ltbn = getCoordXN(i+1,j)
                let ltcn = getCoordXN(i+1,j+1)
                let ltdn = getCoordXN(i,j+1)

                this.orgv.push(lta[0],lta[1],lta[2])
                this.orgv.push(ltb[0],ltb[1],ltb[2])
                this.orgv.push(ltc[0],ltc[1],ltc[2])
                this.texv.push(0,0)
                this.texv.push(1,0)
                this.texv.push(1,1)
                this.norv.push(ltan[0],ltan[1],ltan[2])
                this.norv.push(ltbn[0],ltbn[1],ltbn[2])
                this.norv.push(ltcn[0],ltcn[1],ltcn[2])

                
                this.orgv.push(ltc[0],ltc[1],ltc[2])
                this.orgv.push(ltd[0],ltd[1],ltd[2])
                this.orgv.push(lta[0],lta[1],lta[2])
                this.texv.push(1,1)
                this.texv.push(0,1)
                this.texv.push(0,0)
                this.norv.push(ltcn[0],ltcn[1],ltcn[2])
                this.norv.push(ltdn[0],ltdn[1],ltdn[2])
                this.norv.push(ltan[0],ltan[1],ltan[2])
                
                
            }
        }
        for(let i=0;i<this.de*this.de*6;i++){
            this.elv.push(idx)
            idx++;
        }
        
        return this
    }

    protected override register(): void {
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl

            const posBuffer = <WebGLBuffer>gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, posBuffer)
            const vertex:number[] = []
            for(let i=0;i<this.orgv.length;i+=3){
                vertex.push(this.orgv[i]*this.sc+this.cx);
                vertex.push(this.orgv[i+1]*this.sc+this.cy);
                vertex.push(this.orgv[i+2]*this.sc+this.cz);
            }
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertex), gl.STATIC_DRAW)
            p.getBuffer().set("pos", posBuffer)

            const eleBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(this.elv), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            
            const normBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(this.norv),gl.STATIC_DRAW)
            p.getBuffer().set("norm", normBuffer)
            console.log(this.orgv.length,this.de*this.de*6)
            p.getBuffer().setNumVertices(this.de*this.de*6)

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}