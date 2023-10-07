import { AriaComponent } from "../../core/AriaComponent"
import { IAriaCoreBuffer, IAriaCoreTexture } from "../../core/base/AriaRendererCompatDef"
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps"
import { IAriaFramebuffer } from "../../core/interface/IAriaFramebuffer"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore"
import { AriaComTexture } from "./AriaComTexture"

export class AriaFramebufferOption{
    enableHdr:boolean
    depthMap:boolean
    scaler:number
    regularRect:boolean
    mipmap:boolean
    cubic:boolean
    useGivenWH:boolean
    wid:number = -1
    height:number = -1
    baseW:number = window.innerWidth
    baseH:number = window.innerHeight
    constructor(){
        this.mipmap = false
        this.regularRect = false
        this.enableHdr = true
        this.depthMap = false
        this.scaler = 1
        this.cubic = false
        this.useGivenWH = false
    }
    static create(){
        return new AriaFramebufferOption()
    }
    setRectangleText(x:boolean){
        this.regularRect = x
        return this
    }
    setWH(wid:number,height:number){
        this.wid = wid
        this.height = height
        this.useGivenWH = true
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
    protected fb:IAriaCoreBuffer
    protected tex:IAriaCoreTexture
    protected rbos:IAriaCoreBuffer[]
    protected options:AriaFramebufferOption
    protected texWrapper: AriaComTexture
    protected renderer:IAriaRendererCore
    
    constructor(renderer:IAriaRendererCore,options: AriaFramebufferOption | null){
        super("AriaCom/Framebuffer")
        this.renderer = renderer
        this.options = options ? options : new AriaFramebufferOption();
        this.texWrapper  = new AriaComTexture()
   
        let wid = (this.options.regularRect)?2048*this.options.scaler:this.options.baseW*this.options.scaler
        let height = (this.options.regularRect)?2048*this.options.scaler:this.options.baseH*this.options.scaler

        if(this.options.useGivenWH){
            wid = this.options.wid
            height = this.options.height
        }

        this.fb = renderer.createFramebuffer()
        this.renderer.activateFramebuffer(this)
        
        if(!this.options.cubic){
            this.tex = renderer.createEmptyTexture(wid,height,this.options.mipmap,this.options.enableHdr)
            renderer.setFramebufferTexture(this.tex)
        }else{
            this.tex = renderer.createEmptyCubicTexture(wid,height,this.options.mipmap,this.options.enableHdr)
            renderer.setFramebufferTextureCubic(this.tex)
        }

        this.rbos = []
        if(this.options.cubic){
            for(let i=0;i<6;i++){
                this.rbos.push(renderer.createEmptyRBO(wid,height))
                this.renderer.setFramebufferRenderBuffer(this.rbos[i])
            }
        }else{
            this.rbos.push(renderer.createEmptyRBO(wid,height))
            this.renderer.setFramebufferRenderBuffer(this.rbos[0])
        }
        this.renderer.removeFramebuffer()
        this.texWrapper.setTex(this.tex)
    }
    onClear(renderer:IAriaRendererCore){
        if(this.options.cubic){
            for(let i=0;i<6;i++){
                renderer.switchFramebufferAttachmentsForCubemap(this.rbos[i],this.tex,i)
                renderer.clearScreen()
            }
        }else{
            renderer.clearScreen()
        }
    }
    
    onRender(renderer:IAriaRendererCore,renderCall: () => any): void {
        if(this.options.cubic){
            for(let i=0;i<6;i++){
                renderer.defineUniform("uModel",AriaShaderUniformTp.ASU_MAT4,renderer.getCubicLookat(i)) 
                renderer.switchFramebufferAttachmentsForCubemap(this.rbos[i],this.tex,i)
                renderCall()
            }
        }else{
            renderCall()
        }
    }

    getTex(): AriaComTexture {
        return this.texWrapper
    }
    public getGeneralBufferObject() {
        return this.fb
    }
    public bind(){
        if(!this.options.depthMap){
            if(this.options.regularRect){
                this.renderer.setViewport(2048*this.options.scaler,2048*this.options.scaler)
            }else{
                if(this.options.useGivenWH){
                    this.renderer.setViewport(this.options.wid,this.options.height)
                }else{
                    this.renderer.setViewport(window.innerWidth*this.options.scaler,window.innerHeight*this.options.scaler)
                }
            }

        }else{
            this.renderer.setViewport(1024,1024)
        }
        this.renderer.activateFramebuffer(this)
    }
    public unbind(){
        if(this.options.mipmap){
            this.renderer.generateMipmap2D(this.tex)
        }
        this.renderer.setViewport(window.innerWidth,window.innerHeight)
        this.renderer.removeFramebuffer()
    }
}