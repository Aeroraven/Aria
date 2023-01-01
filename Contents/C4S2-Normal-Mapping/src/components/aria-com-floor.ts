import { AriaBufferMap } from "../core/aria-buffer-map";
import { AriaCamera } from "../core/aria-camera";
import { AriaShader } from "../core/aria-shader";
import { AriaTextureMap } from "../core/aria-texture-map";
import { mat4,mat3 } from "../../node_modules/gl-matrix-ts/dist/index";

export class AriaComFloor{
    static draw(gl:WebGL2RenderingContext,progInfo:AriaShader,
        buffer:AriaBufferMap,tx:AriaTextureMap,camera:AriaCamera){
        
        //Projection 
        const projectionMatrix = camera.getPerspective()
        const modelview = camera.getLookAt()
        const modelIT2 = mat4.create()
        const modelIT = mat4.create()
        mat4.invert(modelIT2,modelview)
        mat4.transpose(modelIT,modelIT2)
    
        //Attrib
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("pos"));
        gl.vertexAttribPointer(progInfo.getAttr("aPos"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aPos"))

        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("norm"));
        gl.vertexAttribPointer(progInfo.getAttr("aNorm"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aNorm"))
    
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("tex"));
        gl.vertexAttribPointer(progInfo.getAttr("aTex"),2,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aTex"))
    
        //Use Shader
        progInfo.use()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.get("ele"))
        gl.activeTexture(gl.TEXTURE0)
        gl.bindTexture(gl.TEXTURE_2D, tx.get("texFloor"))
        gl.uniform1i(progInfo.getUniform("uSampler"),0);
    
    
        //Uniforms
        gl.uniformMatrix4fv(progInfo.getUniform("uProj"),false,projectionMatrix)
        gl.uniformMatrix4fv(progInfo.getUniform("uModel"),false,modelview)
        gl.uniformMatrix4fv(progInfo.getUniform("uModelInvTrans"),false,modelIT)
        gl.drawElements(gl.TRIANGLES,6,gl.UNSIGNED_SHORT,0)
    }
    static initBuffer(gl:WebGL2RenderingContext):AriaBufferMap{
        const posBuffer = <WebGLBuffer>gl.createBuffer();
        let vertices = [
            -1.0, -1.0,  1.0,
             1.0, -1.0,  1.0,
             1.0,  1.0,  -1.0,
            -1.0,  1.0,  -1.0,
        ];
        gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)
    
        const texBuffer = <WebGLBuffer>gl.createBuffer();
        let texLst = [0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0]
        gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(texLst),gl.STATIC_DRAW)

        const normBuffer = <WebGLBuffer>gl.createBuffer();
        let normList = [
            0.0, 1.0,  1.0,
            0.0, 1.0,  1.0,
            0.0, 1.0,  1.0,
            0.0, 1.0,  1.0,
        ];
        gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(normList),gl.STATIC_DRAW)
    
        const eleBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
        const eleIndices = [
            0,  1,  2,      0,  2,  3,   
        ];
        
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
    
        //Return
        const mp = new AriaBufferMap()
        mp.set("pos",posBuffer)
        mp.set("tex",texBuffer)
        mp.set("ele",eleBuffer)
        mp.set("norm",normBuffer)
        return mp
    }
    
}
