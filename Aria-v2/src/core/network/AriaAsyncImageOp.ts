import axios from "axios";

export class AriaAsyncImageOp{
    public static async loadImage(path:string,downloadCb:(p:number)=>void = (p:number)=>{}):Promise<HTMLImageElement>{
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
}