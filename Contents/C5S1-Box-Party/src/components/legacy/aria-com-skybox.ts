import { loadImage } from "../../core/aria-base"
import { AriaBufferMap } from "../../core/aria-buffer-map"
import { AriaCamera } from "../../core/aria-camera"
import { AriaShader } from "../../core/aria-shader"

export class AriaComSkybox{
    gl:WebGL2RenderingContext
    texList:WebGLTexture[]
    cubeTex:WebGLTexture
    buf:AriaBufferMap
    vao:WebGLVertexArrayObject

    constructor(gl:WebGL2RenderingContext){
        this.gl = gl
        this.texList = []
        this.cubeTex = <WebGLTexture>gl.createTexture()
        this.buf = new AriaBufferMap()
        this.vao = <WebGLVertexArrayObject>gl.createVertexArray()
    }
    initBuffer(){
        const gl = this.gl
        const posBuffer = <WebGLBuffer>gl.createBuffer();
        let vertices = [
            -1.0,  1.0, -1.0,
            -1.0, -1.0, -1.0,
            1.0, -1.0, -1.0,
            1.0, -1.0, -1.0,
            1.0,  1.0, -1.0,
            -1.0,  1.0, -1.0,

            -1.0, -1.0,  1.0,
            -1.0, -1.0, -1.0,
            -1.0,  1.0, -1.0,
            -1.0,  1.0, -1.0,
            -1.0,  1.0,  1.0,
            -1.0, -1.0,  1.0,

            1.0, -1.0, -1.0,
            1.0, -1.0,  1.0,
            1.0,  1.0,  1.0,
            1.0,  1.0,  1.0,
            1.0,  1.0, -1.0,
            1.0, -1.0, -1.0,

            -1.0, -1.0,  1.0,
            -1.0,  1.0,  1.0,
            1.0,  1.0,  1.0,
            1.0,  1.0,  1.0,
            1.0, -1.0,  1.0,
            -1.0, -1.0,  1.0,

            -1.0,  1.0, -1.0,
            1.0,  1.0, -1.0,
            1.0,  1.0,  1.0,
            1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,
            -1.0,  1.0, -1.0,

            -1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            1.0, -1.0, -1.0,
            1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            1.0, -1.0,  1.0
        ];
        gl.bindVertexArray(this.vao)
        gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)
        gl.bindVertexArray(null)

        this.buf.set("pos",posBuffer)
    }

    async loadTexFromFolder(folder:string){
        const faces = ["right","left","top","bottom","front","back"]
        const gl = this.gl

        gl.bindTexture(gl.TEXTURE_CUBE_MAP,this.cubeTex)

        for(let i=0;i<faces.length;i++){
            const img = await loadImage(folder+"/"+faces[i]+".jpg")
            gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X + i,0,gl.RGB,img.width,img.height,0,gl.RGB,gl.UNSIGNED_BYTE,img)
        }
        gl.texParameteri(gl.TEXTURE_CUBE_MAP,gl.TEXTURE_MIN_FILTER,gl.LINEAR)
        gl.texParameteri(gl.TEXTURE_CUBE_MAP,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
        gl.texParameteri(gl.TEXTURE_CUBE_MAP,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE)
        gl.texParameteri(gl.TEXTURE_CUBE_MAP,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE)
        gl.texParameteri(gl.TEXTURE_CUBE_MAP,gl.TEXTURE_WRAP_R,gl.CLAMP_TO_EDGE)
    }

    draw(shader:AriaShader,camera:AriaCamera){
        
        const gl = this.gl
        gl.disable(gl.STENCIL_TEST)
        gl.depthMask(false)
        const proj = camera.getPerspective()
        const view = camera.getLookAt3()

        gl.bindVertexArray(this.vao)

        gl.bindBuffer(gl.ARRAY_BUFFER, this.buf.get("pos"));
        gl.vertexAttribPointer(shader.getAttr("aPos"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(shader.getAttr("aPos"))

        shader.use()

        gl.activeTexture(gl.TEXTURE0)
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, this.cubeTex)
        gl.uniform1i(shader.getUniform("uSkybox"),0);

        gl.uniformMatrix4fv(shader.getUniform("uProj"),false,proj)
        gl.uniformMatrix4fv(shader.getUniform("uView"),false,view)

        gl.drawArrays(gl.TRIANGLES,0,36)
        gl.bindVertexArray(null)
        gl.depthMask(true)
        gl.enable(gl.STENCIL_TEST)
    }
}