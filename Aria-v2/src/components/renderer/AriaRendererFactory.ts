import { AriaComponent } from "../../core/AriaComponent";
import { AriaWGL2Renderer } from "./AriaWGL2Renderer";

export enum AriaRendererFactoryTypes{
    ARFT_WEBGL = 1,
    ARFT_WEBGPU = 2,
    ARFT_VULKAN = 3
}

export class AriaRendererFactory extends AriaComponent{
    constructor(){
        super("AriaRendererFactory")
    }
    public async create(tp:AriaRendererFactoryTypes,context:string){
        if(tp==AriaRendererFactoryTypes.ARFT_WEBGL){
            return new AriaWGL2Renderer(context)
        }else if(tp==AriaRendererFactoryTypes.ARFT_WEBGPU){
            let adapter = await navigator.gpu?.requestAdapter()
            const device = await adapter?.requestDevice();
            if (!device) {
                this._logError("WebGPU is not supported now!")
                return;
            }
            this._logError("Not supported")
        }
        throw Error("Not supported")
    }
}