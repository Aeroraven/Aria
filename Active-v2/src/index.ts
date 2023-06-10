import { AriaStageAntialiasing } from "./examples/AriaStageAntialiasing";
import { AriaStageBloom } from "./examples/AriaStageBloom";
import { AriaStageBlur } from "./examples/AriaStageBlur";
import { AriaStageDrop } from "./examples/AriaStageDrop";
import { AriaStagePointLight } from "./examples/AriaStagePointLight";
import { AriaStageSkyBox } from "./examples/AriaStageSkybox";
import { AriaStageSurface } from "./examples/AriaStageSurface";
import { AriaStageSpringMassCloth } from "./examples/AriaStageSpringMassCloth";
import { AriaVoxel } from "./components/voxel/AriaVoxel";
import { AriaStageVoxel } from "./examples/AriaStageVoxel";
import { AriaStageVoxelKlee } from "./examples/AriaStageVoxelKlee";

function addEntry(name:string, description:string, redirect:string){
    const w = document.createElement("div")
    const title = document.createElement("h3")
    title.innerHTML = name
    title.style.cursor="pointer"
    title.onclick = (ev)=>{
        window.location.href=(window.location.href += "?stage="+redirect)
    }
    const desc = document.createElement("description")
    desc.innerHTML = description
    w.appendChild(title)
    w.appendChild(desc)
    document.body.appendChild(w)
}

function addTitle(){
    const title = document.createElement("h1")
    title.innerHTML = "List of Available Demo <small style='font-size:14px'>Aria v2</small>"
    title.style.borderBottom = "1px solid black"
    document.body.appendChild(title)
}

function helperPageReady(){
    document.body.style.backgroundColor = "#ffffff"
    document.body.style.marginLeft = "30px"
    document.body.style.marginRight = "30px"
    document.body.style.marginTop = "30px"
    document.getElementById("webgl_displayer")!.style.display = "none"
    addTitle()
}

async function main(){
    const px = [
        {name:"Antialiasing",redir:"antialiasing",cl:AriaStageAntialiasing,desc:"Implementation of postprocessing antialiasing algorithms including FXAA and MLAA"},
        {name:"Blurring",redir:"blur",cl:AriaStageBlur,desc:"Implementation of some blurring algorithms like gaussian blurs"},
        {name:"Bloom Effect",redir:"bloom",cl:AriaStageBloom,desc:"Implementation of bloom effect based on postprocess blurring"},
        {name:"Skybox",redir:"skybox",cl:AriaStageSkyBox,desc:"Implementation of sky box using cube map"},
        {name:"Point Shadow",redir:"pointshadow",cl:AriaStagePointLight,desc:"Implementation of omnidirectional shadow"},
        {name:"Mass Spring Cloth",redir:"smcloth",cl:AriaStageSpringMassCloth,desc:"Cloth simulation based on mass spring model"},
        {name:"Voxelization",redir:"voxklee",cl:AriaStageVoxelKlee,desc:"Turning a mesh to voxel grids"}
    ]
    const stageParam = (new URLSearchParams(window.location.href.replace(/(.|\s)*\?/i,""))).get("stage") 
    let goRedirect = false
    for(let value of px){
        if(value.redir == stageParam){
            goRedirect = true;
            (new value.cl).entry()
            break;
        }
    }
    if(!goRedirect){
        helperPageReady()
        for(let value of px){
            addEntry(value.name,value.desc,value.redir)
        }
    }
}

main()

