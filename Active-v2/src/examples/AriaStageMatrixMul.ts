import { AriaComCanvasEx } from "../components/canvas/AriaComCanvasEx"
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer"
import { AriaComData2DTexture } from "../components/texture/AriaComData2DTexture"
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel"
import { AriaStage } from "./AriaStage"

import vertex from "../presets/shaders/postprocess/matmul/vertex.glsl"
import fragment from "../presets/shaders/postprocess/matmul/fragment.glsl"
import { AriaComPostPass } from "../components/postproc/AriaComPostPass"
import { AriaComMaterial } from "../components/material/AriaComMaterial"
import { AriaComShaderMaterial } from "../components/material/AriaComShaderMaterial"
import { AriaPostFxIdentity } from "../presets/postprocess/AriaPostFxIdentity"


export class AriaStageMatrixMul extends AriaStage{
    constructor(){
        super("AriaStage/MatrixMul")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")

        const sz = 4
        const tex_1 = new AriaComData2DTexture(sz,sz)
        const tex_2 = new AriaComData2DTexture(sz,sz)
        const canvas = new AriaComCanvasEx(sz,sz)

        tex_1.setItem(0,0,255)


        const material = new AriaComShaderMaterial({
            vertex:vertex,
            fragment:fragment
        })

        const postproc = new AriaComPostPass()
        postproc.unsafeAllowEmptyInput()
        postproc.setMaterial(material)
        postproc.addInputTexture(tex_1,"uTex1")

        const afterproc = new AriaPostFxIdentity()
        afterproc.addInput(canvas,"uSourceFrame")

        const panel = new AriaComParamPanel()
        panel.initInteraction()

        const drawCall = ()=>{
            renderer.renderComposite(canvas,()=>{
                renderer.renderSimple(postproc)
            })
            renderer.renderSimple(afterproc)
            panel.reqAniFrame(drawCall)
        }
        drawCall()
    }
}