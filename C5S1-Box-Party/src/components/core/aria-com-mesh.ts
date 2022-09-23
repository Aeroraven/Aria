import { AriaCamera } from "../../core/aria-camera";
import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { AriaComBuffers } from "./aria-com-buffers";
import { AriaComTexture } from "./aria-com-texture";



export interface IAriaRenderable{
    render():void
}

export enum AriaComMeshTextureType{
    acmtDiffuse = "texDiffuse"
}

interface IAriaComMeshTextureTypeMapping{
    k:AriaComMeshTextureType
    u:string
}

export class AriaComMesh extends AriaComponent implements IAriaRenderable{
    camera:AriaCamera
    shader:AriaShader
    texUniformMaps:IAriaComMeshTextureTypeMapping[]
    texUniformOrds:number[]

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.camera = new AriaCamera()
        this.shader = new AriaShader(gl,"","")
        this.texUniformMaps = [
            {k:AriaComMeshTextureType.acmtDiffuse,u:"uDiffuse"}
        ]
        this.texUniformOrds = [
            gl.TEXTURE0
        ]
    }

    setCamera(o:AriaCamera){
        this.camera = o
        return this
    }

    setShader(s:AriaShader){
        this.shader = s
        return this
    }

    setBuffer(b:AriaComponent){
        this.addComponent("buffer",b)
        return this
    }

    setTexture(tp:AriaComMeshTextureType,o:AriaComponent){
        this.addComponent(tp,o)
        return this
    }

    render(){
        const gl = this.gl
        const modelview = this.camera.getLookAt()
        const projectionMatrix = this.camera.getPerspective()
        const buf : AriaComBuffers = <AriaComBuffers>this.getChild("buffer")
        
        //Shader
        this.shader.use()

        //Attrib
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("pos"));
        gl.vertexAttribPointer(this.shader.getAttr("aVert"),3,gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(this.shader.getAttr("aVert"))

        if(buf.getBuffer().exists("tex")){
            gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("tex"));
            gl.vertexAttribPointer(this.shader.getAttr("aTex"),2,gl.FLOAT,false,0,0)
            gl.enableVertexAttribArray(this.shader.getAttr("aTex"))
        }

        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buf.getBuffer().get("ele"))

        gl.uniformMatrix4fv(this.shader.getUniform("uModel"),false,modelview)
        gl.uniformMatrix4fv(this.shader.getUniform("uProj"),false,projectionMatrix)
    
        for(let i=0;i<this.texUniformMaps.length;i++){
            if(this.childExist(this.texUniformMaps[i].k)){
                gl.activeTexture(this.texUniformOrds[i])
                gl.bindTexture(gl.TEXTURE_2D, (<AriaComTexture>this.getChild(this.texUniformMaps[i].k)).getTex())
                gl.uniform1i(this.shader.getUniform(this.texUniformMaps[i].u),i);
            }
        }

        gl.drawElements(gl.TRIANGLES,buf.getBuffer().numVertices,gl.UNSIGNED_SHORT,0)
    
    }
}
