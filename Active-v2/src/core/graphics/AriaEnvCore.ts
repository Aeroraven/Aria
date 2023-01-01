import { AriaObject } from "../base/AriaObject";

export class AriaEnvCore extends AriaObject{
    public static inst:AriaEnvCore|null = null
    //Env
    private defaultContainer:string = "webgl_displayer"
    private env:WebGL2RenderingContext|null = null
    private canvas:HTMLCanvasElement|null = null
    public static getInstance(){
        if(this.inst==null){
            this.inst = new AriaEnvCore()
        }
        return <AriaEnvCore>this.inst
    }

    constructor(){
        super("AriaEnvCore")
        const canvas = <HTMLCanvasElement>(document.getElementById(this.defaultContainer));
        canvas.width = window.innerWidth
        canvas.height = window.innerHeight

        this.env = canvas.getContext("webgl2", { stencil: true })
        || (()=>{
            this._logError("WebGL2 is not supported. Update your browser to view the content")
            return null
        })();
        if(this.env!=null){
            this._logInfo("Initialized WebGL2 context")
        }

        this.canvas = canvas
        const gl = this.env!
        gl.viewport(0,0,window.innerWidth,window.innerHeight)
        gl.disable(gl.CULL_FACE)

        if(gl.getExtension('EXT_color_buffer_float')==null){
            this._logError("Cannot enable extension `EXT_color_buffer_float`. Please upgrade your browser!")
        }else{
            this._logInfo("Enabled extension `EXT_color_buffer_float`")
        }
    }

    public getEnv(): WebGL2RenderingContext{
        if(this.env==null){
            this._logError("WebGL2 context is not valid")
        }
        return <WebGL2RenderingContext>this.env
    }

    public getCanvas(): HTMLCanvasElement{
        if(this.canvas==undefined||this.canvas==null){
            this._logError("Canvas is not defined")
        }
        return <HTMLCanvasElement>this.canvas
    }
}