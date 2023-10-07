import axios from "../../node_modules/axios/index";

export function loadShader(gl:WebGL2RenderingContext,type:number,source:string):WebGLShader|null{
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

export function initShaderProgram(gl:WebGL2RenderingContext,vsrc:string,fsrc:string):WebGLProgram|null{
    const vs = loadShader(gl,gl.VERTEX_SHADER,vsrc)
    const fs = loadShader(gl,gl.FRAGMENT_SHADER,fsrc)
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

export async function loadImage(path:string,downloadCb:(p:number)=>void = (p:number)=>{}):Promise<HTMLImageElement>{
    const img = new Image();

    const resp = await axios({ url: path, responseType: 'arraybuffer', method: 'get',onDownloadProgress: (x:any)=>{
        let percent = x.loaded / x.total * 100
        downloadCb(percent)
    },headers: {
        'Cache-Control': 'no-cache'
    }});
    
    const response = window.URL.createObjectURL(new Blob([resp.data]))

    await new Promise((resolve,reject)=>{
        img.onload = ()=>{
            resolve(true)
        }
        img.src = response
    })
    return img
}

export async function loadFile(path:string,downloadCb:(p:number)=>void = (p:number)=>{} ):Promise<string>{
    const resp = await axios.get(path,{
        onDownloadProgress(progressEvent) {
            let percent = progressEvent.loaded / progressEvent.total * 100
            downloadCb(percent)
        },
    });
    return <string>resp.data;
}

export function createTexture(gl:WebGL2RenderingContext,img:HTMLImageElement):WebGLTexture{
    const tex = gl.createTexture()
    gl.bindTexture(gl.TEXTURE_2D,tex);
    gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE,img);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.bindTexture(gl.TEXTURE_2D,null)
    return <WebGLTexture>tex;
}