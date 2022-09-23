import { mat4, vec3 } from "../../../node_modules/gl-matrix-ts/dist/index";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "../core/aria-com-buffers";
import { IAriaComGeometry } from "./aria-com-geometry";

export class AriaComRect extends AriaComponent implements IAriaComGeometry {
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

    protected override register(): void {
        if (this.parent instanceof AriaComBuffers) {
            const p = <AriaComBuffers>this.parent
            const gl = this.gl
            const posBuffer = <WebGLBuffer>gl.createBuffer();

            let vertices = [
                -1.0, -1.0,  -1.0,
                1.0, -1.0,  -1.0,
                1.0,  1.0,  -1.0,
                -1.0,  1.0,  -1.0,
            ];
            gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)
        
            const texBuffer = <WebGLBuffer>gl.createBuffer();
            let texLst = [0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0]
            gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
            gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(texLst),gl.STATIC_DRAW)
        
            const eleBuffer = <WebGLBuffer>gl.createBuffer()
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
            const eleIndices = [
                0,  1,  2,      0,  2,  3,   
            ];
            
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
            p.getBuffer().set("pos", posBuffer)


            gl.bindBuffer(gl.ARRAY_BUFFER, texBuffer)
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texLst), gl.STATIC_DRAW)
            p.getBuffer().set("tex", texBuffer)

            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eleBuffer)
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(eleIndices), gl.STATIC_DRAW)
            p.getBuffer().set("ele", eleBuffer)

            p.getBuffer().setNumVertices(6)

        } else {
            throw new Error("Parent should be AriaComBuffers")
        }
    }
}