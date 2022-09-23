import { AriaComponent } from "../base/aria-component";

export class AriaComTexture extends AriaComponent{
    tex:WebGLTexture
    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.tex = <WebGLTexture>gl.createTexture()
    }
    setTex(o:WebGLTexture){
        this.tex = o
        return this
    }
    getTex(){
        return this.tex
    }
    protected override register(): void {
        
    }
}