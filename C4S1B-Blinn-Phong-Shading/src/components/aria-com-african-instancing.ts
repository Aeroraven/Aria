import { AriaBufferMap } from "../core/aria-buffer-map";
import { AriaCamera } from "../core/aria-camera";
import { AriaShader } from "../core/aria-shader";
import { AriaTextureMap } from "../core/aria-texture-map";
import { mat4,mat3 } from "../../node_modules/gl-matrix-ts/dist/index";
import { AriaLoadedMesh } from "../core/aria-loaded-mesh";

export class AriaComAfricanInstancing{
    static initBuffer(gl:WebGL2RenderingContext,meshSrc:string):AriaBufferMap{
        const mesh = new AriaLoadedMesh()
        mesh.loadFromWavefront(meshSrc)
        const meshdata = mesh.exportData()
    
        const posBuffer = <WebGLBuffer>gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.v),gl.STATIC_DRAW)
    
        const colBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ARRAY_BUFFER,colBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.vc),gl.STATIC_DRAW)
    
        const normBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.vn),gl.STATIC_DRAW)
    
        const texBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(meshdata.vt),gl.STATIC_DRAW)
    
        const eleBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(meshdata.f),gl.STATIC_DRAW)
    
        
        //
        const r = new AriaBufferMap()
        r.setNumVertices(meshdata.v.length/3)
        r.set("pos",posBuffer)
        r.set("col",colBuffer)
        r.set("norm",normBuffer)
        r.set("tex",texBuffer)
        r.set("ele",eleBuffer)
    
        return r
    }

    static draw(gl:WebGL2RenderingContext,progInfo:AriaShader,buffer:AriaBufferMap,tx:AriaTextureMap,camera:AriaCamera,sb:boolean=false){
        
        gl.enable(gl.DEPTH_TEST)
        gl.stencilOp(gl.KEEP,gl.KEEP,gl.REPLACE)
        gl.stencilFunc(gl.ALWAYS,1,0xff)
        gl.stencilMask(0xff)
        
    
        //Projection
        const modelview = camera.getLookAt()
        const projectionMatrix = camera.getPerspective()
        const modelIT2 = mat4.create()
        const modelIT = mat4.create()
        mat4.invert(modelIT2,modelview)
        mat4.transpose(modelIT,modelIT2)
    
        //Attrib
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("pos"));
        gl.vertexAttribPointer(progInfo.getAttr("aVert"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aVert"))
    
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("col"));
        gl.vertexAttribPointer(progInfo.getAttr("aColor"),4,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aColor"))
    
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("tex"));
        gl.vertexAttribPointer(progInfo.getAttr("aTex"),2,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aTex"))
    
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer.get("norm"));
        gl.vertexAttribPointer(progInfo.getAttr("aNorm"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(progInfo.getAttr("aNorm"))
    
        //Use Shader
        progInfo.use()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buffer.get("ele"))
        
        //Offset
        let ofs = [-1.0,0.0,0.0,0.0, 1.0,1.0,0.0,0.0]

        //Uniforms
        gl.uniform3fv(progInfo.getUniform("uCamPos"),camera.getCamPosArray())
        gl.uniform3fv(progInfo.getUniform("uCamFront"),camera.getCamFrontArray())
        gl.uniform4fv(progInfo.getUniform("uOffset"),ofs)
        gl.uniformMatrix4fv(progInfo.getUniform("uModel"),false,modelview)
        gl.uniformMatrix4fv(progInfo.getUniform("uProj"),false,projectionMatrix)
        gl.uniformMatrix4fv(progInfo.getUniform("uModelInvTrans"),false,modelIT)
    
        gl.activeTexture(gl.TEXTURE0)
        gl.bindTexture(gl.TEXTURE_2D, tx.get("tex1"))
        gl.uniform1i(progInfo.getUniform("uSampler"),0);
    
        gl.activeTexture(gl.TEXTURE1)
        gl.bindTexture(gl.TEXTURE_2D, tx.get("tex2"))
        gl.uniform1i(progInfo.getUniform("uSpecSampler"),1);

        if(sb){
            gl.activeTexture(gl.TEXTURE2)
            gl.bindTexture(gl.TEXTURE_2D, tx.get("texSky"))
            gl.uniform1i(progInfo.getUniform("uSybox"),2);
        }
        
    
        gl.drawElementsInstanced(gl.TRIANGLES,buffer.getNumVertices(),gl.UNSIGNED_SHORT,0,2)
    }
}