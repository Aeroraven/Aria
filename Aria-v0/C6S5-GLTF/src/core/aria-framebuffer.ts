export class AriaFramebufferOption{
    enableHdr:boolean
    depthMap:boolean
    scaler:number
    constructor(){
        this.enableHdr = false
        this.depthMap = false
        this.scaler = 1
    }
    static create(){
        return new AriaFramebufferOption()
    }
    setHdr(x:boolean){
        this.enableHdr = x
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
}

export class AriaFramebuffer{
    gl:WebGL2RenderingContext
    fb:WebGLFramebuffer
    tex:WebGLTexture
    rbo:WebGLRenderbuffer
    options:AriaFramebufferOption
    
    constructor(gl:WebGL2RenderingContext, options: AriaFramebufferOption | null){
        this.options = options ? options : new AriaFramebufferOption();
        this.gl = gl
        this.fb = <WebGLFramebuffer>gl.createFramebuffer()
        this.tex = <WebGLTexture>gl.createTexture()
        gl.bindFramebuffer(gl.FRAMEBUFFER,this.fb)
        gl.bindTexture(gl.TEXTURE_2D,this.tex)
        if(this.options.enableHdr){
            gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA16F,window.innerWidth*this.options.scaler,window.innerHeight*this.options.scaler,0,gl.RGBA,gl.FLOAT,null)
        }
        else if(this.options.depthMap){
            gl.texImage2D(gl.TEXTURE_2D,0,gl.DEPTH_COMPONENT,1024,1024,0,gl.DEPTH_COMPONENT,gl.FLOAT,null)
        }
        else{
            gl.texImage2D(gl.TEXTURE_2D,0,gl.RGB,window.innerWidth*this.options.scaler,window.innerHeight*this.options.scaler,0,gl.RGB,gl.UNSIGNED_BYTE,null)
        }
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR );
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.bindTexture(gl.TEXTURE_2D,null);
        if(this.options.depthMap){
            gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.DEPTH_ATTACHMENT,gl.TEXTURE_2D,this.tex,0)
        }else{
            gl.framebufferTexture2D(gl.FRAMEBUFFER,gl.COLOR_ATTACHMENT0,gl.TEXTURE_2D,this.tex,0)
        }
        

        this.rbo = <WebGLRenderbuffer>gl.createRenderbuffer()
        if(this.options.depthMap){
            gl.readBuffer(gl.NONE)
        }else{
            gl.bindRenderbuffer(gl.RENDERBUFFER,this.rbo)
            gl.renderbufferStorage(gl.RENDERBUFFER,gl.DEPTH24_STENCIL8,window.innerWidth*this.options.scaler,window.innerHeight*this.options.scaler)
            gl.bindRenderbuffer(gl.RENDERBUFFER,null)
    
            gl.framebufferRenderbuffer(gl.FRAMEBUFFER,gl.DEPTH_STENCIL_ATTACHMENT,gl.RENDERBUFFER,this.rbo)
            
        }
        gl.bindFramebuffer(gl.FRAMEBUFFER,null)
        
    }
    bind(){
        if(!this.options.depthMap){
            this.gl.viewport(0,0,window.innerWidth*this.options.scaler,window.innerHeight*this.options.scaler)
        }else{
            this.gl.viewport(0,0,1024,1024)
        }
        this.gl.bindFramebuffer(this.gl.FRAMEBUFFER,this.fb)
    }
    unbind(){
        this.gl.viewport(0,0,window.innerWidth,window.innerHeight)
        this.gl.bindFramebuffer(this.gl.FRAMEBUFFER,null)
    }
}