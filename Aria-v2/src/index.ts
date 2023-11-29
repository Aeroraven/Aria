import { AriaStageAntialiasing } from "./examples/AriaStageAntialiasing";
import { AriaStageBloom } from "./examples/AriaStageBloom";
import { AriaStageBlur } from "./examples/AriaStageBlur";
import { AriaStagePointLight } from "./examples/AriaStagePointLight";
import { AriaStageSkyBox } from "./examples/AriaStageSkybox";
import { AriaStageSpringMassCloth as AriaStageMassSpringCloth } from "./examples/AriaStageSpringMassCloth";
import { AriaStageVolumetricCloud } from "./examples/AriaStageVolCloud";
import { AriaStagePathTracing } from "./examples/AriaStagePathTracing";
import { AriaStageWasm } from "./examples/AriaStageWasm";
import { AriaStageRectangleReproj } from "./examples/AriaStageRectangleReproj";
import { AriaStageIAP } from "./examples/AriaStageInternalAngleReproj";

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
    const title = document.createElement("h2")
    title.innerHTML = "List of Available Scenes <small style='font-size:12px'>Aria v2</small>"
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
        {name:"Testing",redir:"wasm",cl:AriaStageWasm,desc:"Pending"}, 
        {name:"Mass Spring Cloth",redir:"smcloth",cl:AriaStageMassSpringCloth,desc:"Cloth simulation based on mass spring model"},
        {name:"Antialiasing",redir:"antialiasing",cl:AriaStageAntialiasing,desc:"Implementation of postprocessing antialiasing algorithms"},
        {name:"Blurring",redir:"blur",cl:AriaStageBlur,desc:"Implementation of some blurring algorithms like gaussian blurs"},
        {name:"Bloom Effect II",redir:"bloom",cl:AriaStageBloom,desc:"Implementation of bloom effect based on postprocess blurring"},
        {name:"Skybox",redir:"skybox",cl:AriaStageSkyBox,desc:"Implementation of sky box using cube map"},
        {name:"Point Shadow",redir:"pointshadow",cl:AriaStagePointLight,desc:"Implementation of omnidirectional shadow"},
        //{name:"Voxelization",redir:"voxklee",cl:AriaStageVoxelKlee,desc:"Turning a mesh to voxel grids"},
        {name:"Ray Marching II",redir:"volcloud",cl:AriaStageVolumetricCloud,desc:"Single-pass volumetric rendering of 3D perlin noise"},
        {name:"Rectified Path Tracer",redir:"pathtracing",cl:AriaStagePathTracing,desc:"Some thing done as my course project seems to be wrong. Here I made it right."}, 
        //{name:"Pending",redir:"flsim",cl:AriaStageFLSim,desc:"flsim"},
        //{name:"Pending",redir:"matmul",cl:AriaStageMatrixMul,desc:"matmul"},
        {name:"",redir:"rectreproj",cl:AriaStageRectangleReproj,desc:"",hidden:true}, 
        {name:"",redir:"ianglereproj",cl:AriaStageIAP,desc:"",hidden:true}, 
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
            if(value.hidden && value.hidden===true){
                continue;
            }
            addEntry(value.name,value.desc,value.redir)
        }
    }
}

main()

