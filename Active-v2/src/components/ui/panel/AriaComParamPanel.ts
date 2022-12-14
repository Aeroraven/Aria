import { AriaComponent } from "../../../core/AriaComponent";
import { AriaEnv } from "../../../core/graphics/AriaEnv";
import { IAriaPair } from "../../../core/interface/IAriaPair";
import { IAriaInteractive } from "../../base/interface/IAriaInteractive";

export class AriaComParamPanel extends AriaComponent implements IAriaInteractive{
    private el = AriaEnv.doc.createElement("div")
    private _id = 0
    private _fpsTimeStamp = Date.now()
    private _fps = 0
    private _fpsCounter = 0

    constructor(){
        super("Aria/ParamPanel")
    }

    disableInteraction(): void {
        this._logError("ParamPanel: interaction trigger cannot be detached")
    }

    initInteraction(): void {
        this.el.style.position = "absolute" 
        this.el.style.left = "0px"
        this.el.style.top = "0px"
        this.el.style.minWidth = "300px"
        this.el.style.paddingRight = "40px"
        this.el.style.color = "#efefef"
        this.el.style.paddingLeft = "20px"
        this.el.style.paddingTop = "10px"
        this.el.style.paddingBottom = "10px"
        this.el.style.backgroundColor = "#2f2f2f"
        AriaEnv.docBody.appendChild(this.el)
    }

    addSelector(name:string, options:IAriaPair<string,string>[], def:string="", handler:(respId:string)=>any){
        const selParent = AriaEnv.doc.createElement("select")
        const selParentId = "parampanel-aria-identifier-"+this._id
        selParent.id = selParentId
        selParent.addEventListener("change",(e)=>{
            const value = (<HTMLSelectElement>document.getElementById(selParentId)).value
            handler(value)
        })
        options.forEach((x)=>{
            const childEl = AriaEnv.doc.createElement("option")
            childEl.value = x.key
            childEl.text = x.value
            if(x.key==def){
                childEl.selected = true
            }
            selParent.appendChild(childEl)
        })
        const label = AriaEnv.doc.createElement("span")
        label.innerHTML = name
        label.style.width = "150px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"
        const block = AriaEnv.doc.createElement("div")
        block.appendChild(label)
        block.appendChild(selParent)
        this.el.appendChild(block)
        this._id++
    }

    addTitle(name:string){
        const label = AriaEnv.doc.createElement("span")
        label.innerHTML = name
        label.style.marginRight = "20px"
        label.style.display = "inline-block"
        label.style.fontFamily = "sans-serif"
        label.style.fontWeight = "bold"
        label.style.fontSize = "20px"
        label.style.marginBottom = "10px"
        const block = AriaEnv.doc.createElement("div")
        block.appendChild(label)
        this.el.appendChild(block)
    }

    addFPSMeter(name:string){
        const label = AriaEnv.doc.createElement("span")
        label.innerHTML = name
        label.style.width = "150px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"

        const labelx = AriaEnv.doc.createElement("span")
        labelx.innerHTML = "-"
        labelx.style.width = "150px"
        labelx.style.display = "inline-block"
        labelx.style.fontFamily = "serif"
        labelx.id = "parampanel-aria-identifier-fps-"+this._id

        setInterval(()=>{
            labelx.innerHTML = this._fps + ""
        },200)

        const block = AriaEnv.doc.createElement("div")
        block.appendChild(label)
        block.appendChild(labelx)
        this.el.appendChild(block)
    }

    reqAniFrame(callback:FrameRequestCallback){
        this._fpsCounter++
        if(this._fpsCounter==10){
            this._fpsCounter=0
            this._fps = Math.floor(1000/(Date.now()-this._fpsTimeStamp)*10*100)/100
            this._fpsTimeStamp = Date.now()
        }
        requestAnimationFrame(callback)
    }


}