import { AriaBufferMap } from "../../core/aria-buffer-map";
import { AriaCamera } from "../../core/aria-camera";
import { AriaShader } from "../../core/aria-shader";
import { AriaTextureMap } from "../../core/aria-texture-map";

export class AriaComAfricanOutline{
    static draw(gl:WebGL2RenderingContext,progInfo:AriaShader,
        buffer:AriaBufferMap,camera:AriaCamera){
        
        progInfo.use()
        gl.stencilOp(gl.KEEP,gl.KEEP,gl.KEEP)
        gl.stencilFunc(gl.NOTEQUAL,0x01,0xff)
        gl.stencilMask(0x00)
        gl.disable(gl.DEPTH_TEST)
    
        //Projection 
        const modelview = camera.getLookAt()
        const projectionMatrix = camera.getPerspective()
    
        //Attrib
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("pos"));
        gl.vertexAttribPointer(progInfo.getAttr("aVert"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aVert"))
    
        //Use Shader
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.get("ele"))
    
        //Uniforms
        gl.uniformMatrix4fv(progInfo.getUniform("modelMat"),false,modelview)
        gl.uniformMatrix4fv(progInfo.getUniform("projMat"),false,projectionMatrix)
    
        gl.drawElements(gl.TRIANGLES,buffer.getNumVertices(),gl.UNSIGNED_SHORT,0)
    
        gl.stencilMask(0xff)
        gl.enable(gl.DEPTH_TEST)
    }
    
}