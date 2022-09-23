import { AriaPageIndicator } from "./core/aria-page-indicator";
import { AriaStage } from "./stage/aria-stage-base";
import { AriaStageFog } from "./stage/aria-stage-fog";
import { AriaStageFXAA } from "./stage/aria-stage-fxaa";
import { AriaStageGLTF } from "./stage/aria-stage-gltf";
import { AriaStageToonOutline } from "./stage/aria-stage-outline";
import { AriaStagePerlinNoise } from "./stage/aria-stage-perlin-noise";
import { AriaStageSSAO } from "./stage/aria-stage-ssao";
import { AriaStageSSR } from "./stage/aria-stage-ssr";
import { AriaStageValueNoise } from "./stage/aria-stage-value-noise";
import { AriaStageVolume } from "./stage/aria-stage-volume";
import { AriaStageVolumetricFog } from "./stage/aria-stage-volumetric-fog";
import { AriaStageWave } from "./stage/aria-stage-wave";


async function main(){
    AriaPageIndicator.getInstance().updateLoadingTip("Initializing")
    const canvas = <HTMLCanvasElement>(document.getElementById("webgl_displayer"));
    canvas.width = window.innerWidth
    canvas.height = window.innerHeight
    
    const gl_p = canvas.getContext("webgl2", { stencil: true })
        || (()=>{
            window.alert("WebGL2 is not supported. Update your browser to view the content")
            return null
        })();
    if(!gl_p) return;
    const gl = <WebGL2RenderingContext>gl_p;

    //Params 
    const chosenStage = (()=>{
        const defaultString = "ts_outline"
        const titlePrefix = "Aeroraven Demo"
        const stageParam = (new URLSearchParams(window.location.href.replace(/(.|\s)*\?/i,""))).get("stage") 
            || (window.location.href += "?stage="+defaultString)
        const stagePair = [
            {key:null, stage:AriaStage, name:"Default"},
            {key:"fog", stage:AriaStageFog, name:"Fog"},
            {key:"value_noise", stage:AriaStageValueNoise, name:"Value Noise"},
            {key:"perlin_noise", stage:AriaStagePerlinNoise, name:"Perlin Noise"},
            {key:"gerstner_wave", stage:AriaStageWave, name:"Gerstner Wave"},
            {key:"volume_render", stage:AriaStageVolume, name:"Volume Render"},
            {key:"fxaa", stage:AriaStageFXAA, name:"Fast Approximate Antialiasing"},
            {key:"gltf", stage:AriaStageGLTF, name:"GLTF Model"},
            {key:"vol_light", stage:AriaStageVolumetricFog, name:"Volumetric Lighting"},
            {key:"ssao", stage:AriaStageSSAO, name:"Screen Space Ambient Occlusion"},
            {key:"ssr", stage:AriaStageSSR, name:"Screen Space Reflection"},
            {key:"ts_outline", stage:AriaStageToonOutline, name:"Toon Shading Outline"},
        ]
        let defaultType = AriaStage
        for(let value of stagePair){
            if(value.key == stageParam){
                AriaPageIndicator.getInstance().setSceneName(value.name)
                document.getElementsByTagName("title")[0].innerHTML = titlePrefix + " [" + value.name + "]"
                return value.stage
            }
        }
        return defaultType
    })()

    //Stage
    const stage:AriaStage = new chosenStage();
    stage.prepare(gl)

    //Render
    function render(){
        stage.render(gl)
        requestAnimationFrame(render)
    }
    requestAnimationFrame(render)
}

main()