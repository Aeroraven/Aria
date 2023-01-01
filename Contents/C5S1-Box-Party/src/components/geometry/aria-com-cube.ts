import { mat4, vec3 } from "../../../node_modules/gl-matrix-ts/dist/index";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";

export class AriaComCube extends AriaComponent {
    cx: number
    cy: number
    cz: number
    sc: number
    rot: Float32Array
    
    protected constructor(gl: WebGL2RenderingContext) {
        super(gl);
        this.cx = 0
        this.cy = 0
        this.cz = 0
        this.sc = 0
        this.rot = mat4.create()
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
    public rotateX(r:number){
        mat4.rotate(this.rot,this.rot,r,new Float32Array([1,0,0]))
        return this
    }

    public rotateY(r:number){
        mat4.rotate(this.rot,this.rot,r,new Float32Array([0,1,0]))
        return this
    }

    public rotateZ(r:number){
        mat4.rotate(this.rot,this.rot,r,new Float32Array([0,0,1]))
        return this
    }
    protected override register(): void {
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
            const posBuffer = <WebGLBuffer>gl.createBuffer();
            let vertices = [
                -0.5, -0.5, -0.5,
                0.5, -0.5, -0.5,
                0.5, 0.5, -0.5,
                0.5, 0.5, -0.5,
                -0.5, 0.5, -0.5,
                -0.5, -0.5, -0.5,

                -0.5, -0.5, 0.5,
                0.5, -0.5, 0.5,
                0.5, 0.5, 0.5,
                0.5, 0.5, 0.5,
                -0.5, 0.5, 0.5,
                -0.5, -0.5, 0.5,

                -0.5, 0.5, 0.5,
                -0.5, 0.5, -0.5,
                -0.5, -0.5, -0.5,
                -0.5, -0.5, -0.5,
                -0.5, -0.5, 0.5,
                -0.5, 0.5, 0.5,

                0.5, 0.5, 0.5,
                0.5, 0.5, -0.5,
                0.5, -0.5, -0.5,
                0.5, -0.5, -0.5,
                0.5, -0.5, 0.5,
                0.5, 0.5, 0.5,

                -0.5, -0.5, -0.5,
                0.5, -0.5, -0.5,
                0.5, -0.5, 0.5,
                0.5, -0.5, 0.5,
                -0.5, -0.5, 0.5,
                -0.5, -0.5, -0.5,

                -0.5, 0.5, -0.5,
                0.5, 0.5, -0.5,
                0.5, 0.5, 0.5,
                0.5, 0.5, 0.5,
                -0.5, 0.5, 0.5,
                -0.5, 0.5, -0.5,
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
            let texList = [
                0.0, 0.0,
                1.0, 0.0,
                1.0, 1.0,
                1.0, 1.0,
                0.0, 1.0,
                0.0, 0.0,

                0.0, 0.0,
                1.0, 0.0,
                1.0, 1.0,
                1.0, 1.0,
                0.0, 1.0,
                0.0, 0.0,

                1.0, 0.0,
                1.0, 1.0,
                0.0, 1.0,
                0.0, 1.0,
                0.0, 0.0,
                1.0, 0.0,

                1.0, 0.0,
                1.0, 1.0,
                0.0, 1.0,
                0.0, 1.0,
                0.0, 0.0,
                1.0, 0.0,

                0.0, 1.0,
                1.0, 1.0,
                1.0, 0.0,
                1.0, 0.0,
                0.0, 0.0,
                0.0, 1.0,

                0.0, 1.0,
                1.0, 1.0,
                1.0, 0.0,
                1.0, 0.0,
                0.0, 0.0,
                0.0, 1.0
            ];
            gl.bindBuffer(gl.ARRAY_BUFFER, texBuffer)
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texList), gl.STATIC_DRAW)
            p.getBuffer().set("tex", texBuffer)

            const eleBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            const eleIndices:number[] = [];
            for(let i=0;i<36;i++){
                eleIndices.push(i)
            }
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(eleIndices), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            p.getBuffer().setNumVertices(36)

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}