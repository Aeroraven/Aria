import { mat4, vec3, vec4 } from "../../../node_modules/gl-matrix-ts/dist/index";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { IAriaComGeometry } from "./aria-com-geometry";

export class AriaComCube extends AriaComponent implements IAriaComGeometry {
    cx: number
    cy: number
    cz: number
    scx: number
    scy: number
    scz: number
    rot: Float32Array
    rotinv: Float32Array
    topOnly:boolean
    
    protected constructor(gl: WebGL2RenderingContext) {
        super(gl);
        this.topOnly = false
        this.cx = 0
        this.cy = 0
        this.cz = 0
        this.scx = 0
        this.scy = 0
        this.scz = 0
        this.rot = mat4.create()
        mat4.identity(this.rot)
        this.rotinv = mat4.create()
        mat4.identity(this.rotinv)
    }

    public translate(dx: number, dy: number, dz: number) {
        this.cx = dx
        this.cy = dy
        this.cz = dz
        return this
    }
    public scale(sc: number) {
        this.scx = sc
        this.scz = sc
        this.scy = sc
        return this
    }
    public scalex(scx: number,scy:number, scz:number) {
        this.scx = scx
        this.scy = scy
        this.scz = scz
        return this
    }
    public rotateX(r:number){
        mat4.rotate(this.rot,this.rot,r,new Float32Array([1,0,0]))
        mat4.invert(this.rotinv,this.rot)
        return this
    }

    public rotateY(r:number){
        mat4.rotate(this.rot,this.rot,r,new Float32Array([0,1,0]))
        mat4.invert(this.rotinv,this.rot)
        return this
    }

    public rotateZ(r:number){
        mat4.rotate(this.rot,this.rot,r,new Float32Array([0,0,1]))
        mat4.invert(this.rotinv,this.rot)
        return this
    }
    public setTopOnly(b:boolean){
        this.topOnly = b
        return this
    }
    protected override register(): void {
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
            const posBuffer = <WebGLBuffer>gl.createBuffer();
            let vertices = [
                //Back
                -0.5, -0.5, -0.5,   
                0.5, -0.5, -0.5,   
                0.5,  0.5, -0.5,   
                0.5,  0.5, -0.5,   
               -0.5,  0.5, -0.5,   
               -0.5, -0.5, -0.5,   
                
               //Front
               -0.5, -0.5,  0.5,   
                0.5, -0.5,  0.5,   
                0.5,  0.5,  0.5,   
                0.5,  0.5,  0.5,   
               -0.5,  0.5,  0.5,   
               -0.5, -0.5,  0.5,   
                
               //Left
               -0.5,  0.5,  0.5,   
               -0.5,  0.5, -0.5,   
               -0.5, -0.5, -0.5,   
               -0.5, -0.5, -0.5,   
               -0.5, -0.5,  0.5,   
               -0.5,  0.5,  0.5,   
                

               //Right
                0.5,  0.5,  0.5,   
                0.5,  0.5, -0.5,   
                0.5, -0.5, -0.5,   
                0.5, -0.5, -0.5,   
                0.5, -0.5,  0.5,   
                0.5,  0.5,  0.5,   
                
                //Bottom
               -0.5, -0.5, -0.5,   
                0.5, -0.5, -0.5,   
                0.5, -0.5,  0.5,   
                0.5, -0.5,  0.5,   
               -0.5, -0.5,  0.5,   
               -0.5, -0.5, -0.5,   
                
               //Top
               -0.5,  0.5, -0.5,   
                0.5,  0.5, -0.5,   
                0.5,  0.5,  0.5,   
                0.5,  0.5,  0.5,   
               -0.5,  0.5,  0.5,   
               -0.5,  0.5, -0.5,   
            ];

            if(this.topOnly){
                vertices = [
                    -0.5,  0, -0.5,   
                    0.5,  0, -0.5,   
                    0.5,  0,  0.5,   
                    0.5,  0,  0.5,   
                    -0.5,  0,  0.5,   
                    -0.5,  0, -0.5,   
                ]
            }
            for (let i = 0; i < vertices.length; i += 3) {
                let g = vec3.create()
                vec3.set(g, vertices[i],vertices[i+1],vertices[i+2])
                vec3.transformMat4(g,g,this.rot)
                vertices[i] = g[0] * this.scx + this.cx
                vertices[i + 1] = g[1] * this.scy + this.cy
                vertices[i + 2] = g[2] * this.scz + this.cz
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, posBuffer)
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW)
            p.getBuffer().set("pos", posBuffer)


            const texBuffer = <WebGLBuffer>gl.createBuffer();
            let preTex = [
                0.0, 0.0,
                1.0, 0.0,
                1.0, 1.0,
                1.0, 1.0,
                0.0, 1.0,
                0.0, 0.0,
            ]
            let texList:number[] = []
            for(let i=0;i<((this.topOnly)?1:6);i++){
                texList = texList.concat(preTex)
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, texBuffer)
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texList), gl.STATIC_DRAW)
            p.getBuffer().set("tex", texBuffer)

            const normBuffer = <WebGLBuffer>gl.createBuffer()
            let preNorms = [
                [0,0,-1],
                [0,0,1],
                [-1,0,0],
                [1,0,0],
                [0,-1,0],
                [0,1,0],
            ]
            if(this.topOnly){
                preNorms = [[0,1,0]]
            }
            let normList:number[] = []
            for(let i=0;i<((this.topOnly)?1:6);i++){
                for(let j=0;j<6;j++){
                    let g = vec3.create()
                    let w = vec3.create()
                    vec3.set(w, preNorms[i][0],preNorms[i][1],preNorms[i][2])   
                    vec3.transformMat4(g,w,this.rot)
                    normList = normList.concat([g[0],g[1],g[2]])
                }
            }
            gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(normList),gl.STATIC_DRAW)
            p.getBuffer().set("norm", normBuffer)


            const eleBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            const eleIndices:number[] = []
            for(let i=0;i<((this.topOnly)?6:36);i++){
                eleIndices.push(i)
            }
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(eleIndices), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            //Tangent & Bitangent
            const tanBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ARRAY_BUFFER,tanBuffer)
            let tangents:number[] = []
            let bitangents:number[] = []

            for(let ik = 0;ik<((this.topOnly)?2:12);ik++){
                let va = ik*3
                let vb = ik*3+1
                let vc = ik*3+2
                let pa = [vertices[3*va],vertices[3*va+1],vertices[3*va+2]]
                let pb = [vertices[3*vb],vertices[3*vb+1],vertices[3*vb+2]]
                let pc = [vertices[3*vc],vertices[3*vc+1],vertices[3*vc+2]]

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
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(tangents),gl.STATIC_DRAW)
            p.getBuffer().set("tan", tanBuffer)

            const bibuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ARRAY_BUFFER,bibuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(bitangents),gl.STATIC_DRAW)
            p.getBuffer().set("bitan", bibuffer)

            p.getBuffer().setNumVertices(((this.topOnly)?6:36))

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}