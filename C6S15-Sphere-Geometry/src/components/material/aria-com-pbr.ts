import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { IAriaComMaterial } from "../core/interfaces/aria-com-material";

export class AriaComPBR extends AriaComponent implements IAriaComMaterial{
    albedo:WebGLTexture
    roughness:WebGLTexture
    metallic:WebGLTexture
    ao:WebGLTexture
    normal:WebGLTexture
    gl:WebGL2RenderingContext

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.gl = gl
        this.albedo = <WebGLTexture>gl.createTexture()
        this.roughness = <WebGLTexture>gl.createTexture()
        this.metallic =  <WebGLTexture>gl.createTexture()
        this.normal =  <WebGLTexture>gl.createTexture()
        this.ao = <WebGLTexture>gl.createTexture()
    }
    protected override register(): void {
        
    }
    setAlbedo(tex:WebGLTexture){
        this.albedo = tex
        return this
    }
    setNormal(tex:WebGLTexture){
        this.normal = tex
        return this
    }
    setRoughness(dv:WebGLTexture){
        this.roughness = dv
        return this
    }

    setMetallic(dv:WebGLTexture){
        this.metallic = dv
        return this
    }

    setAO(dv:WebGLTexture){
        this.ao = dv
        return this;
    }

    emitUniforms(shader:AriaShader): void {
        const gl = this.gl
        const lst = ['uT_Albedo','uT_Roughness','uT_Metallic','uT_AO','uT_Normal']
        const vlst = [this.albedo,this.roughness,this.metallic,this.ao,this.normal]
        for(let i=0;i<5;i++){
            let tid = shader.getTexId()
            gl.activeTexture(gl.TEXTURE0 + tid)
            gl.bindTexture(gl.TEXTURE_2D, vlst[i])
            gl.uniform1i(shader.getUniform(lst[i]),tid);
        }
    }
    
}