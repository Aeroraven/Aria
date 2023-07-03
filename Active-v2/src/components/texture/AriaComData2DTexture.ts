import { AriaComTexture } from "../base/AriaComTexture";

export class AriaComData2DTexture extends AriaComTexture{
    length:number
    width:number
    height:number
    data:number[]
    constructor(length:number,width:number){
        super()
        this._rename("AriaCom/Data2DTexture")
        this.length=length
        this.width=width
        this.height=1
        this.data=new Array<number>(length*width)

        this.texLoadProcedure = (renderer)=>{
            const dataArray = new Uint8Array(this.data)
            const txw = renderer.createTextureData2D(dataArray,this.width,this.length)
            this.setTex(txw)
        }
    }
    setItem(x:number,y:number,v:number){
        this.texLoaded = false
        this.data[x*this.width+y] = v
    }
}