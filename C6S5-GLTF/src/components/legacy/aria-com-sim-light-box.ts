import { AriaBufferMap } from "../../core/aria-buffer-map";
import { AriaCamera } from "../../core/aria-camera";
import { AriaShader } from "../../core/aria-shader";
import { AriaTextureMap } from "../../core/aria-texture-map";

export class AriaComSimLightBox{
    static drawLight(gl:WebGL2RenderingContext,progInfo:AriaShader,
        buffer:AriaBufferMap,camera:AriaCamera){
        
        //Projection 
        const modelview = camera.getLookAt()
        const projectionMatrix = camera.getPerspective()
    
        //Attrib
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("pos"));
        gl.vertexAttribPointer(progInfo.getAttr("aVert"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aVert"))
    
        //Use Shader
        progInfo.use()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.get("ele"))
    
        //Uniforms
        gl.uniformMatrix4fv(progInfo.getAttr("uModel"),false,modelview)
        gl.uniformMatrix4fv(progInfo.getAttr("uProj"),false,projectionMatrix)
    
        gl.drawElements(gl.TRIANGLES,36,gl.UNSIGNED_SHORT,0)
    }
    static initBuffer(gl:WebGL2RenderingContext):AriaBufferMap{
        const posBuffer = <WebGLBuffer>gl.createBuffer();
        let vertices = [
            -1.0, -1.0,  1.0,
             1.0, -1.0,  1.0,
             1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,
            -1.0, -1.0, -1.0,
            -1.0,  1.0, -1.0,
             1.0,  1.0, -1.0,
             1.0, -1.0, -1.0,
            -1.0,  1.0, -1.0,
            -1.0,  1.0,  1.0,
             1.0,  1.0,  1.0,
             1.0,  1.0, -1.0,
            -1.0, -1.0, -1.0,
             1.0, -1.0, -1.0,
             1.0, -1.0,  1.0,
            -1.0, -1.0,  1.0,
             1.0, -1.0, -1.0,
             1.0,  1.0, -1.0,
             1.0,  1.0,  1.0,
             1.0, -1.0,  1.0,
            -1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            -1.0,  1.0,  1.0,
            -1.0,  1.0, -1.0
        ];
        for(let i=0;i<vertices.length;i+=3){
            vertices[i] = vertices[i] * 0.2 + 1.5
            vertices[i+1] = vertices[i+1] * 0.2 + 0.5
            vertices[i+2] = vertices[i+2] * 0.2 + 2.0
        }
        gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)
    
        const eleBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
        const eleIndices = [
            0,  1,  2,      0,  2,  3,    // front
            4,  5,  6,      4,  6,  7,    // back
            8,  9,  10,     8,  10, 11,   // top
            12, 13, 14,     12, 14, 15,   // bottom
            16, 17, 18,     16, 18, 19,   // right
            20, 21, 22,     20, 22, 23,   // left
        ];
        
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
    
        const r = new AriaBufferMap()
        r.set("pos",posBuffer)
        r.set("ele",eleBuffer)
    
        return r
    }
    
    
}