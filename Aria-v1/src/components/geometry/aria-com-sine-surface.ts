import { mat4, vec3 } from "gl-matrix-ts";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { IAriaComGeometry } from "./aria-com-geometry";

export class AriaComSineSurface extends AriaComponent implements IAriaComGeometry {
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
        for(let i=0;i<this.de;i++){
            for(let j=0;j<this.de;j++){
                let delta = 1/this.de
                let lt = [i*delta,0,j*delta]
                this.orgv.push(lt[0],lt[1],lt[2])
                this.orgv.push(lt[0]+delta,lt[1],lt[2])
                this.orgv.push(lt[0]+delta,lt[1],lt[2]+delta)
                this.texv.push(0,0)
                this.texv.push(1,0)
                this.texv.push(1,1)
                this.norv.push(0,1,0)
                this.norv.push(0,1,0)
                this.norv.push(0,1,0)

                this.orgv.push(lt[0]+delta,lt[1],lt[2]+delta)
                this.orgv.push(lt[0],lt[1],lt[2]+delta)
                this.orgv.push(lt[0],lt[1],lt[2])
                this.texv.push(1,1)
                this.texv.push(0,1)
                this.texv.push(0,0)
                this.norv.push(0,1,0)
                this.norv.push(0,1,0)
                this.norv.push(0,1,0)
                
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

            p.getBuffer().setNumVertices(this.de*this.de*6)

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}