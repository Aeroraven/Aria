import { AriaComponent } from "../../../core/AriaComponent";
import { IAriaPair } from "../../../core/interface/IAriaPair";
import { IAriaInteractive } from "../../base/interface/IAriaInteractive";

export class AriaComParamPanel extends AriaComponent implements IAriaInteractive{
    private _id = 0
    private _fpsTimeStamp = Date.now()
    private _fps = 0
    private _fpsCounter = 0
    private _status = ""
    private _docBody = document
    private el = this._docBody.createElement("div")

    private _colWid2 = 150
    private _colWid1 = 150
    

    constructor(colWid1:number = 150,colWid2:number = 150){
        super("Aria/ParamPanel")
        this._colWid2 = colWid2
        this._colWid1 = colWid1
    }
    allocateId(){
        this._id++
        return this._id
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
        this.el.style.backgroundColor = "#2f2f2faf"
        this._docBody.body.appendChild(this.el)
    }
    addSlidebar(name:string,min_value:number,max_value:number,handler:(value:number)=>any,initValue:number=0){
        const element = this._docBody.createElement("input")
        element.id = "parampanel-aria-identifier-"+this.allocateId()
        const attrType = this._docBody.createAttribute("type")
        attrType.value = "range"
        const attrMin = this._docBody.createAttribute("min")
        attrMin.value = ""+min_value
        const attrMax = this._docBody.createAttribute("max")
        attrMax.value = ""+max_value
        const attrValue = this._docBody.createAttribute("value")
        attrValue.value = ""+initValue
        
        const idv = this._docBody.createElement("span")
        idv.innerHTML = ""+initValue
        idv.style.paddingLeft = "10px"

        element.setAttributeNode(attrType)
        element.setAttributeNode(attrMin)
        element.setAttributeNode(attrMax)
        element.setAttributeNode(attrValue)
        element.addEventListener("change",(e)=>{
            const value = parseInt((<HTMLInputElement>document.getElementById(element.id)).value)
            handler(value)
            idv.innerHTML = ""+value
        })
        const label = this._docBody.createElement("span")
        label.innerHTML = name
        label.style.width = this._colWid1+"px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"
        const block = this._docBody.createElement("div")
        block.appendChild(label)
        block.appendChild(element)
        block.appendChild(idv)
        this.el.appendChild(block)
        this._id++
    }

    addSelector(name:string, options:IAriaPair<string,string>[], def:string="", handler:(respId:string)=>any){
        const selParent = this._docBody.createElement("select")
        const selParentId = "parampanel-aria-identifier-"+this.allocateId()
        selParent.id = selParentId
        selParent.addEventListener("change",(e)=>{
            const value = (<HTMLSelectElement>document.getElementById(selParentId)).value
            handler(value)
        })
        options.forEach((x)=>{
            const childEl = this._docBody.createElement("option")
            childEl.value = x.key
            childEl.text = x.value
            if(x.key==def){
                childEl.selected = true
            }
            selParent.appendChild(childEl)
        })
        const label = this._docBody.createElement("span")
        label.innerHTML = name
        label.style.width = this._colWid1+"px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"
        const block = this._docBody.createElement("div")
        block.appendChild(label)
        block.appendChild(selParent)
        this.el.appendChild(block)
        this._id++
    }

    addTitle(name:string){
        const label = this._docBody.createElement("span")
        label.innerHTML = name
        label.style.marginRight = "20px"
        label.style.display = "inline-block"
        label.style.fontFamily = "sans-serif"
        label.style.fontWeight = "bold"
        label.style.fontSize = "20px"
        label.style.marginBottom = "10px"
        const block = this._docBody.createElement("div")
        block.appendChild(label)
        this.el.appendChild(block)
    }

    addFPSMeter(name:string){
        const label = this._docBody.createElement("span")
        label.innerHTML = name
        label.style.width = this._colWid1+"px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"

        const labelx = this._docBody.createElement("span")
        labelx.innerHTML = "-"
        labelx.style.width = this._colWid2+"px"
        labelx.style.display = "inline-block"
        labelx.style.fontFamily = "serif"
        labelx.id = "parampanel-aria-identifier-fps-"+this.allocateId()

        setInterval(()=>{
            labelx.innerHTML = this._fps + ""
        },200)

        const block = this._docBody.createElement("div")
        block.appendChild(label)
        block.appendChild(labelx)
        this.el.appendChild(block)
    }

    addDynamicText(name:string,call:()=>string,bold:boolean=false){
        const label = this._docBody.createElement("span")
        label.innerHTML = name
        label.style.width = this._colWid1+"px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"
        if(bold){
            label.style.fontWeight = "bold"
        }

        const labelx = this._docBody.createElement("span")
        labelx.innerHTML = "-"
        labelx.style.width = this._colWid2+"px"
        labelx.style.display = "inline-block"
        labelx.style.fontFamily = "serif"
        labelx.id = "parampanel-aria-identifier-dyntext-"+this.allocateId()

        setInterval(()=>{
            labelx.innerHTML = call()
        },200)

        const block = this._docBody.createElement("div")
        block.appendChild(label)
        block.appendChild(labelx)
        this.el.appendChild(block)
    }

    setStatus(x:string){
        this._status = x
    }
    addStatusBar(name:string){
        const label = this._docBody.createElement("span")
        label.innerHTML = name
        label.style.width = this._colWid1+"px"
        label.style.display = "inline-block"
        label.style.fontFamily = "serif"

        const labelx = this._docBody.createElement("span")
        labelx.innerHTML = "-"
        labelx.style.width = this._colWid2+"px"
        labelx.style.display = "inline-block"
        labelx.style.fontFamily = "serif"
        labelx.id = "parampanel-aria-identifier-loading-bar-"+this.allocateId()

        setInterval(()=>{
            labelx.innerHTML = this._status
        },200)

        const block = this._docBody.createElement("div")
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