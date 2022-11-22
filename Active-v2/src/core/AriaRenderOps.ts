import { AriaEnv } from "./AriaEnv";
import { AriaObject } from "./base/AriaObject";

export class AriaRenderOps extends AriaObject{
    constructor(){
        super("AriaRenderOps")
    }

    public static clearScreen(color:number[] = [0,0,0,0]){
        const gl = AriaEnv.env
        gl.clearColor(color[0],color[1],color[2],color[3]);
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LEQUAL);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
    }

    public static loadShader(type:number,source:string):WebGLShader|null{
        const gl = AriaEnv.env
        const shader = <WebGLShader>gl.createShader(type);
        gl.shaderSource(shader,source);
        gl.compileShader(shader);
        if(!gl.getShaderParameter(shader,gl.COMPILE_STATUS)){
            alert("Shader error:"+gl.getShaderInfoLog(shader));
            console.log("Cannot Compile Shader")
            console.log(source)
            gl.deleteShader(shader)
            return null;
        }
        return shader;
    }
    

    public static initShaderProgram(vsrc:string,fsrc:string):WebGLProgram|null{
        const gl = AriaEnv.env
        const vs = this.loadShader(gl.VERTEX_SHADER,vsrc)
        const fs = this.loadShader(gl.FRAGMENT_SHADER,fsrc)
        if(vs==null||fs==null){
            return null
        }else{
            const vsr = <WebGLShader>vs;
            const fsr = <WebGLShader>fs;
            const shaderProg = <WebGLProgram>gl.createProgram()
            gl.attachShader(shaderProg,vsr);
            gl.attachShader(shaderProg,fsr);
            gl.linkProgram(shaderProg)
            if(!gl.getProgramParameter(shaderProg,gl.LINK_STATUS)){
                alert("ShaderProg error:"+gl.getProgramInfoLog(shaderProg))
                return null;
            }
            return shaderProg
        }
    }

    public static renderInstanced(num:number, instances:number = 1){
        const gl = AriaEnv.env
        gl.drawElementsInstanced(gl.TRIANGLES, num, gl.UNSIGNED_SHORT, 0, instances)
    }

    public static fboUnbind(){
        const gl = AriaEnv.env
        gl.viewport(0,0,window.innerWidth,window.innerHeight)
        gl.bindBuffer(gl.FRAMEBUFFER, null)
    }

    public static createTexture(img:HTMLImageElement):WebGLTexture{
        const gl = AriaEnv.env
        const tex = gl.createTexture()
        gl.bindTexture(gl.TEXTURE_2D,tex);
        gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE,img);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.bindTexture(gl.TEXTURE_2D,null)
        return <WebGLTexture>tex;
    }
}