import { AriaComponent } from "../../core/AriaComponent";
import { AriaEnv } from "../../core/AriaEnv";
import { AriaRenderOps } from "../../core/AriaRenderOps";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaChildContainer } from "../base/interface/IAriaChildContainer";
import { IAriaComponentContainer } from "../base/interface/IAriaComponentContainer";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";

export class AriaComScene extends AriaComponent implements 
IAriaComponentContainer<IAriaShaderEmitter,AriaComScene>, IAriaRenderable, IAriaChildContainer<IAriaRenderable, AriaComScene>{
    
    private _renderObjects:IAriaRenderable[] = []
    private _components: IAriaShaderEmitter[] = []
    private _componentTriggers: (()=>any )[] = []

    constructor(){
        super("AriaCom/Scene")
    }

    public addChild(c: IAriaRenderable<void>): AriaComScene {
        this._renderObjects.push(c)
        return this
    }

    public addComponent(c: IAriaShaderEmitter): AriaComScene {
        this._components.push(c)
        this._componentTriggers.push(()=>{
            c.exportToShader()
        })
        return this
    }

    public render(preTriggers?:(()=>any)[], postTriggers?:(()=>any)[]): void {
        AriaRenderOps.clearScreen()
        this._renderObjects.forEach((el)=>{
            el.render(this._componentTriggers)
        })
    }
    
}