import { AriaComponent } from "../../core/AriaComponent";
import { AriaRenderOps } from "../../core/graphics/AriaRenderOps";
import { AriaShaderOps, AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps";
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter";
import { AriaFramebufferOption } from "../base/AriaComFramebuffer";
import { IAriaCanavs } from "../base/interface/IAriaCanvas";
import { IAriaRenderable } from "../base/interface/IAriaRenderable";
import { AriaComCamera } from "../camera/AriaComCamera";
import { AriaComGeometry } from "../geometry/base/AriaComGeometry";
import { AriaComRectangle } from "../geometry/primary/AriaComRectangle";
import { AriaComCube } from "../geometry/primary/AriaComCube";
import { AriaComMaterial } from "../material/AriaComMaterial";
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore";

export class AriaComPostPass extends AriaComponent implements IAriaRenderable{
    private _material: AriaComMaterial|null = null
    private _geometry: AriaComGeometry
    private _components:  IAriaShaderEmitter[] = []

    protected _inputCanvas: IAriaCanavs[] = []
    protected _inputCanvasName: string[] = []
    protected _inputCanvasType: Map<string,AriaShaderUniformTp> = new Map<string,AriaShaderUniformTp>()
    protected _inputCanvasMap:Map<string,IAriaCanavs> = new Map<string,IAriaCanavs>()
    private _canvasConfig: AriaFramebufferOption = new AriaFramebufferOption()

    constructor(){
        super("AriaCom/PostPass")
        this._geometry = new AriaComRectangle(); 
        const cam = new AriaComCamera()
        //cam.initInteraction()
        this._components.push(cam)
    }
    public setMaterial(m:AriaComMaterial){
        this._material = m
        return this
    }
    public addInput(m:IAriaCanavs, w:string="uSourceFrame", tp:AriaShaderUniformTp = AriaShaderUniformTp.ASU_TEX2D){
        this._inputCanvas.push(m)
        this._inputCanvasName.push(w)
        this._inputCanvasMap.set(w,m)
        this._inputCanvasType.set(w,tp)
        this._logInfo("Added input :"+w)
        return this
    }
    render(renderer:IAriaRendererCore,preTriggers?: ((_:IAriaRendererCore) => any)[] | undefined, postTriggers?: ((_:IAriaRendererCore) => any)[] | undefined): void {
        renderer.clearScreen()
        this._material!.use(renderer)
        if(this._inputCanvas.length==0){
            this._logError("Input canvas cannot be empty")
        }
        this._inputCanvasMap.forEach((value,key)=>{
            renderer.defineUniform(key,AriaShaderUniformTp.ASU_TEX2D,value.getTex())
        })
        
        this._geometry.exportToShader(renderer)
        this._components.forEach((el)=>{
            el.exportToShader(renderer)
        })
        if(preTriggers){
            preTriggers.forEach((el)=>{
                el(renderer)
            })
        }
        renderer.renderInstancedEntry(this._geometry.getVertexNumber())
    }
}