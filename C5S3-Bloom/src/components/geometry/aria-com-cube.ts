import { mat4, vec3 } from "../../../node_modules/gl-matrix-ts/dist/index";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { IAriaComGeometry } from "./aria-com-geometry";

export class AriaComCube extends AriaComponent implements IAriaComGeometry {
    cx: number
    cy: number
    cz: number
    sc: number
    rot: Float32Array
    rotinv: Float32Array
    
    protected constructor(gl: WebGL2RenderingContext) {
        super(gl);
        this.cx = 0
        this.cy = 0
        this.cz = 0
        this.sc = 0
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
        this.sc = sc
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
    protected override register(): void {
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
            const posBuffer = <WebGLBuffer>gl.createBuffer();
            let vertices = [
                -1.0, -1.0,  1.0,
                1.0, -1.0,  1.0,
                1.0,  1.0,  1.0,
                -1.0,  1.0,  1.0,
            
                // Back face
                -1.0, -1.0, -1.0,
                -1.0,  1.0, -1.0,
                1.0,  1.0, -1.0,
                1.0, -1.0, -1.0,
            
                // Top face
                -1.0,  1.0, -1.0,
                -1.0,  1.0,  1.0,
                1.0,  1.0,  1.0,
                1.0,  1.0, -1.0,
            
                // Bottom face
                -1.0, -1.0, -1.0,
                1.0, -1.0, -1.0,
                1.0, -1.0,  1.0,
                -1.0, -1.0,  1.0,
            
                // Right face
                1.0, -1.0, -1.0,
                1.0,  1.0, -1.0,
                1.0,  1.0,  1.0,
                1.0, -1.0,  1.0,
            
                // Left face
                -1.0, -1.0, -1.0,
                -1.0, -1.0,  1.0,
                -1.0,  1.0,  1.0,
                -1.0,  1.0, -1.0
            ];
            for (let i = 0; i < vertices.length; i += 3) {
                let g = vec3.create()
                vec3.set(g, vertices[i],vertices[i+1],vertices[i+2])
                vec3.transformMat4(g,g,this.rot)
                vertices[i] = g[0] * this.sc + this.cx
                vertices[i + 1] = g[1] * this.sc + this.cy
                vertices[i + 2] = g[2] * this.sc + this.cz
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, posBuffer)
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW)
            p.getBuffer().set("pos", posBuffer)


            const texBuffer = <WebGLBuffer>gl.createBuffer();
            let preTex = [0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0]
            let texList:number[] = []
            for(let i=0;i<6;i++){
                texList = texList.concat(preTex)
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, texBuffer)
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texList), gl.STATIC_DRAW)
            p.getBuffer().set("tex", texBuffer)

            const normBuffer = <WebGLBuffer>gl.createBuffer()
            let preNorms = [
                [0,0,1],
                [0,0,-1],
                [0,1,0],
                [0,-1,0],
                [1,0,0],
                [-1,0,0]
            ]

            let normList:number[] = []
            for(let i=0;i<6;i++){
                for(let j=0;j<4;j++){
                    let g = vec3.create()
                    vec3.set(g, preNorms[i][0],preNorms[i][1],preNorms[i][2])   
                    vec3.transformMat4(g,g,this.rotinv)
                    normList = normList.concat([g[0],g[1],g[2]])
                }
            }
            gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(normList),gl.STATIC_DRAW)
            p.getBuffer().set("norm", normBuffer)


            const eleBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            const eleIndices = [
                0,  1,  2,      0,  2,  3,    // front
                4,  5,  6,      4,  6,  7,    // back
                8,  9,  10,     8,  10, 11,   // top
                12, 13, 14,     12, 14, 15,   // bottom
                16, 17, 18,     16, 18, 19,   // right
                20, 21, 22,     20, 22, 23,   // left
            ];
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(eleIndices), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            p.getBuffer().setNumVertices(36)

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}