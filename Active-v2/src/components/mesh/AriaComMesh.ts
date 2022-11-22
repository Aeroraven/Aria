import { AriaComponent } from "../../core/AriaComponent";
import { AriaRenderOps } from "../../core/AriaRenderOps";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { IAriaComponentContainer } from "../base/interface/IAriaComponentContainer";
import { AriaComGeometry } from "../geometry/AriaComGeometry";
import { AriaComMaterial } from "../material/AriaComMaterial";
import { IAriaGeometry } from "../base/interface/IAriaGeometry";

export class AriaComMesh extends AriaComponent implements IAriaRenderable, IAriaComponentContainer<IAriaShaderEmitter>{
    private _material: AriaComMaterial|null
    private _geometry: IAriaGeometry|null
    private _components: IAriaShaderEmitter[] = []

    constructor(m:AriaComMaterial|null = null, g:IAriaGeometry|null = null){
        super("AriaCom/Mesh")
        this._material = m
        this._geometry = g
    }
    public addComponent(c: IAriaShaderEmitter):void{ 
        this._components.push(c)
    }

    public setMaterial(m:AriaComMaterial){
        this._material = m
        return this
    }

    public setGeometry(g:IAriaGeometry){
        this._geometry = g
        return this
    }

    public render(preTriggers?:(()=>any)[], postTriggers?:(()=>any)[]): void {
        if(this._material == null){
            this._logError("Material should not be empty")
            return 
        }
        if(this._geometry == null){
            this._logError("Geometry should not be empty")
            return
        }
        this._material.use()
        this._geometry.exportToShader()
        this._components.forEach((el)=>{
            el.exportToShader()
        })
        if(preTriggers){
            preTriggers.forEach((el)=>{
                el()
            })
        }
        AriaRenderOps.renderInstanced(this._geometry.getVertexNumber())
    }
}