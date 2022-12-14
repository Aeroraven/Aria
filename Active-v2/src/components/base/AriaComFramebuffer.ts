import { AriaComponent } from "../../core/AriaComponent"
import { AriaEnv } from "../../core/graphics/AriaEnv"
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps"
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps"
import { IAriaFramebuffer } from "../../core/interface/IAriaFramebuffer"
import { IAriaGLBuffer } from "../../core/interface/IAriaGLBuffer"
import { AriaComTexture } from "./AriaComTexture"
import { IAriaTextureAttached } from "./interface/IAriaTextureAttached"

export class AriaFramebufferOption{
    enableHdr:boolean
    depthMap:boolean
    scaler:number
    regularRect:boolean
    mipmap:boolean
    cubic:boolean
    constructor(){
        this.mipmap = false
        this.regularRect = true
        this.enableHdr = true
        this.depthMap = false
        this.scaler = 1
        this.cubic = false
    }
    static create(){
        return new AriaFramebufferOption()
    }
    setRectangleText(x:boolean){
        this.regularRect = x
        return this
    }
    setHdr(x:boolean){
        this.enableHdr = x
        return this
    }
    setMipMap(x:boolean){
        this.mipmap = x
        return this
    }
    setDepthMap(x:boolean){
        this.enableHdr = x
        return this
    }
    setScaler(x:number){
        this.scaler = x
        return this
    }
    setCubic(x:boolean){
        if(x){
            this.regularRect = true
        }
        this.cubic = x
        return this
    }
}

export class AriaComFramebuffer extends AriaComponent implements IAriaFramebuffer{
    protected gl:WebGL2RenderingContext
    protected fb:WebGLFramebuffer
    protected tex:WebGLTexture
    protected rbos:WebGLRenderbuffer[]
    protected options:AriaFramebufferOption
    protected texWrapper: AriaComTexture = new AriaComTexture()
    
    constructor(options: AriaFramebufferOption | null){
        super("AriaCom/Framebuffer")
        this.options = options ? options : new AriaFramebufferOption();
        this.gl = AriaEnv.env
        const gl = this.gl
        
        const wid = (this.options.regularRect)?2048*this.options.scaler:AriaEnv.width*this.options.scaler
        const height = (this.options.regularRect)?2048*this.options.scaler:AriaEnv.height*this.options.scaler

        this.fb = gl.createFramebuffer()!

        gl.bindFramebuffer(gl.FRAMEBUFFER,this.fb)
        
        if(!this.options.cubic){
            this.tex = AriaRenderOps.createEmptyTexture(wid,height,this.options.mipmap,this.options.enableHdr)
            gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,this.tex,0)
        }else{
            this.tex = AriaRenderOps.createEmptyCubicTexture(wid,height,this.options.mipmap,this.options.enableHdr)
            gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_CUBE_MAP_POSITIVE_X,this.tex,0)
        }

        this.rbos = []
        if(this.options.cubic){
            for(let i=0;i<6;i++){
                this.rbos.push(AriaRenderOps.createEmptyRBO(wid,height))
                gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,this.rbos[i])
            }
            gl.bindFramebuffer(gl.FRAMEBUFFER,null)
        }else{
            this.rbos.push(AriaRenderOps.createEmptyRBO(wid,height))
            gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,this.rbos[0])
            gl.bindFramebuffer(gl.FRAMEBUFFER,null)
        }
        

        this.texWrapper.setTex(this.tex)
        
    }
    onClear(){
        
        const gl = this.gl
        if(this.options.cubic){
            for(let i=0;i<6;i++){
                gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_CUBE_MAP_POSITIVE_X+i, this.tex, 0);
                gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,this.rbos[i])
                AriaRenderOps.clearScreen()
            }
        }else{
            AriaRenderOps.clearScreen()
        }
    }
    
    onRender(renderCall: () => any): void {
        if(this.options.cubic){
            const gl = this.gl
            for(let i=0;i<6;i++){
                AriaShaderOps.defineUniform("uModel",AriaShaderUniformTp.ASU_MAT4,AriaRenderOps.getCubicLookat(i)) 
                gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_CUBE_MAP_POSITIVE_X+i, this.tex, 0);
                gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,this.rbos[i])
                renderCall()
            }
            
        }else{
            renderCall()
        }
    }

    getTex(): AriaComTexture {
        return this.texWrapper
    }
    public getGLObject() {
        return this.fb
    }
    public bind(){
        if(!this.options.depthMap){
            if(this.options.regularRect){
                this.gl.viewport(0,0,2048*this.options.scaler,2048*this.options.scaler)
            }else{
                this.gl.viewport(0,0,window.innerWidth*this.options.scaler,window.innerHeight*this.options.scaler)
            }

        }else{
            this.gl.viewport(0,0,1024,1024)
        }
        this.gl.bindFramebuffer(this.gl.FRAMEBUFFER,this.fb)
        AriaRenderOps.activateFramebuffer(this)
    }
    public unbind(){
        if(this.options.mipmap){
            this.gl.bindTexture(this.gl.TEXTURE_2D,this.tex)
            this.gl.generateMipmap(this.gl.TEXTURE_2D)
            this.gl.bindTexture(this.gl.TEXTURE_2D,null)
        }
        this.gl.viewport(0,0,window.innerWidth,window.innerHeight)
        this.gl.bindFramebuffer(this.gl.FRAMEBUFFER,null)
        AriaRenderOps.removeFramebuffer()
    }
}