import { AriaComponent } from "../../core/AriaComponent";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaChildContainer } from "../base/interface/IAriaChildContainer";
import { IAriaComponentContainer } from "../base/interface/IAriaComponentContainer";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";

export class AriaComScene extends AriaComponent implements 
IAriaComponentContainer<IAriaShaderEmitter,AriaComScene>, IAriaRenderable, IAriaChildContainer<IAriaRenderable, AriaComScene>{
    
    private _renderObjects:IAriaRenderable[] = []
    private _components: IAriaShaderEmitter[] = []
    private _componentTriggers: ((renderer:IAriaRendererCore)=>any )[] = []

    constructor(){
        super("AriaCom/Scene")
    }

    public addChild(c: IAriaRenderable<void>): AriaComScene {
        this._renderObjects.push(c)
        return this
    }

    public addComponent(c: IAriaShaderEmitter): AriaComScene {
        this._components.push(c)
        this._componentTriggers.push((renderer:IAriaRendererCore)=>{
            c.exportToShader(renderer)
        })
        return this
    }

    public render(renderer:IAriaRendererCore, preTriggers?:((_:IAriaRendererCore)=>any)[], postTriggers?:((_:IAriaRendererCore)=>any)[]): void {
        renderer.clearScreenRequest()
        const epTriggers = new Array<((_:IAriaRendererCore)=>any)>().concat(preTriggers?preTriggers:[]).concat(this._componentTriggers)
        this._renderObjects.forEach((el)=>{
            el.render(renderer,epTriggers)
        })
    }
    
}