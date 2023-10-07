import { AriaComTexture } from "../base/AriaComTexture";

export class AriaComData3DTexture extends AriaComTexture{
    length:number
    width:number
    height:number
    data:number[]
    constructor(length:number,width:number,height:number){
        super()
        this._rename("AriaCom/Data3DTexture")
        this.length=length
        this.width=width
        this.height=height
        this.data=new Array<number>(height*length*width)

        this.texLoadProcedure = (renderer)=>{
            const dataArray = new Uint8Array(this.data)
            const txw = renderer.createTexture3D(dataArray,this.width,this.height,this.length)
            this.setTex(txw)
        }
    }
    setItem(x:number,y:number,z:number,v:number){
        this.texLoaded = false
        this.data[x*this.width*this.height+y*this.height+z] = v
    }
}