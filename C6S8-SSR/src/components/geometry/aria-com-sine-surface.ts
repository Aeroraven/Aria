import { mat4, vec3 } from "../../../node_modules/gl-matrix-ts/dist/index";
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
                vertex.push(this.orgv[i+1]*this.sc+this.cz);
                vertex.push(this.orgv[i+2]*this.sc+this.cz);
            }
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertex), gl.STATIC_DRAW)
            p.getBuffer().set("pos", posBuffer)

            const eleBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(this.elv), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            //Tangents
            const texList = this.texv
            let tangents:number[] = []
            let bitangents:number[] = []
            for(let ik = 0;ik<this.de*this.de*2;ik++){
                let va = ik*3
                let vb = ik*3+1
                let vc = ik*3+2
                let pa = [vertex[3*va],vertex[3*va+1],vertex[3*va+2]]
                let pb = [vertex[3*vb],vertex[3*vb+1],vertex[3*vb+2]]
                let pc = [vertex[3*vc],vertex[3*vc+1],vertex[3*vc+2]]

                let vta = va
                let vtb = vb
                let vtc = vc
                let uva = [texList[2*vta],texList[2*vta+1]]
                let uvb = [texList[2*vtb],texList[2*vtb+1]]
                let uvc = [texList[2*vtc],texList[2*vtc+1]]

                let tangent = [0,0,0]
                let bitangent = [0,0,0]

                let sAB = [pb[0]-pa[0],pb[1]-pa[1],pb[2]-pa[2]]
                let sAC = [pc[0]-pc[0],pc[1]-pa[1],pc[2]-pa[2]]
                let a11 = uvb[0]-uva[0]
                let a12 = uvb[1]-uva[1]
                let a21 = uvc[0]-uva[0]
                let a22 = uvc[1]-uva[1]
                if(a11*a12*a21*a22>0.1){
                    let tan_deno = (uvc[1]-uva[1])*(uvb[0]-uva[0]) - (uvb[1]-uva[1])*(uvc[0]-uva[0])
                    let bitan_deno = (uvb[1]-uva[1])*(uvc[0]-uva[0]) - (uvc[1]-uva[1])*(uvb[0]-uva[0]) 
                    if(tan_deno == 0 || bitan_deno==0){
                        console.log("WARNING",uva[0],uva[1],uvb[0],uvb[1],uvc[0],uvc[1],tan_deno,bitan_deno)
                    }
                    for(let i=0;i<3;i++){
                        tangent[i] = ((uvc[1]-uva[1])*(pb[i]-pa[i]) - (uvb[1]-uva[1]) * (pc[i]-pa[i])) / tan_deno
                        bitangent[i] = ((uvc[0]-uva[0])*(pb[i]-pa[i]) - (uvb[0]-uva[0])*(pc[i]-pa[i])) / bitan_deno
                    }
                    
                }else{
                    if(a11==0 && a12*a21!=0){
                        //sAB=a12*B
                        //sAC=a21*T+a22*B
                        for(let j=0;j<3;j++){
                            bitangent[j]=sAB[j]/a12;
                            tangent[j]=(sAC[j]-a22*bitangent[j])/a21;
                        }
                    }else if(a12==0 && a11*a22!=0){
                        //sAB=a11*T
                        //sAC=a21*T+a22*B
                        for(let j=0;j<3;j++){
                            tangent[j]=sAB[j]/a11
                            bitangent[j]=(sAC[j]-a21*tangent[j])/a22;
                        }
                    }
                    else{
                        alert("warning"+a11+","+a12+","+a21+","+a22)
                        console.log(a11,a12,a21,a22)
                        console.log(uva,uvb,uvc)
                    }
                }

                if(Math.abs(tangent[0])<0.1 && Math.abs(tangent[1])<0.1 && Math.abs(tangent[2])<0.1){
                    alert("INVALID TANGENT")
                }
                
                for(let j=0;j<3;j++){
                    tangents = tangents.concat(tangent)
                    bitangents = bitangents.concat(bitangent)
                }
            }

            const bibuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ARRAY_BUFFER,bibuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(bitangents),gl.STATIC_DRAW)
            p.getBuffer().set("bitan", bibuffer)

            const tanBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ARRAY_BUFFER,tanBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(tangents),gl.STATIC_DRAW)
            p.getBuffer().set("tan", tanBuffer)

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